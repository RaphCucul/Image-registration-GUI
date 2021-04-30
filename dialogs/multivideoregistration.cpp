#include "dialogs/multivideoregistration.h"
#include "ui_multivideoregistration.h"
#include "util/vector_operations.h"
#include "util/files_folders_operations.h"
#include "util/playbutton.h"
#include "image_analysis/frangi_utilization.h"
#include "shared_staff/sharedvariables.h"
#include "shared_staff/errors.h"

#include <QDebug>
#include <QMimeType>
#include <QMimeData>
#include <QMimeDatabase>
#include <QUrl>
#include <QDropEvent>
#include <QFileDialog>
#include <QTableWidget>
#include <QJsonDocument>
#include <QThread>
#include <QIcon>
#include <QHash>

MultiVideoRegistration::MultiVideoRegistration(QWidget *parent) :
    RegistrationParent(parent),
    ui(new Ui::MultiVideoRegistration)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    initMaps();

    ui->listOfVideos->setRowCount(1);
    ui->listOfVideos->setColumnCount(3);
    QStringList columnHeadersList = {tr("Video"),tr("Status"),tr("Result")};
    ui->listOfVideos->setHorizontalHeaderLabels(columnHeadersList);

    ui->chooseMultiVPB->setText(tr("Choose few videos"));
    ui->chooseFolderPB->setText(tr("Choose whole folder"));
    ui->registratePB->setText(tr("Registrate"));
    ui->registratePB->setEnabled(false);
    ui->saveResultsPB->setText(tr("Save computed results"));

    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));

    localErrorDialogHandling[ui->listOfVideos] = new ErrorDialog(ui->listOfVideos);
    localErrorDialogHandling[ui->registratePB] = new ErrorDialog(ui->registratePB);
}

MultiVideoRegistration::~MultiVideoRegistration()
{
    delete ui;
}

void MultiVideoRegistration::displayStatus(QString status){
    ui->listOfVideos->removeCellWidget(videoCounter,1);
    if (status == "startingCalculations"){
        ui->listOfVideos->setCellWidget(videoCounter, 1, createIconTableItem(true,"currentlyProcessed.png"));
    }
    else if (status == "noProgress"){
        ui->listOfVideos->setCellWidget(videoCounter, 1, createIconTableItem(true,"noProgress.png"));
    }
    else if (status == "allDone"){
        ui->listOfVideos->setCellWidget(videoCounter, 1, createIconTableItem(true,"everythingOK.png"));
        ui->listOfVideos->removeCellWidget(videoCounter,2);
        ui->listOfVideos->setCellWidget(videoCounter,2,createIconTableItem(false,videoListFull.at(videoCounter)));
    }
    else if (status == "error"){
        ui->listOfVideos->setCellWidget(videoCounter, 1, createIconTableItem(true,"everythingBad.png"));
    }
}

void MultiVideoRegistration::populateProperties(QStringList chosenVideos)
{    
    for (int videoIndex = 0; videoIndex < chosenVideos.count(); videoIndex++) {
        QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+chosenVideos.at(videoIndex)+".dat");
        QJsonObject videoParametersJson = readJson(videoParametersFile);
        videoPropertiesDouble[chosenVideos.at(videoIndex)] = videoParametersDouble;
        videoPropertiesInt[chosenVideos.at(videoIndex)] = videoParametersInt;
        videoPropertiesAnomaly[chosenVideos.at(videoIndex)] = videoAnomalies;
        processVideoParameters(videoParametersJson,
                               videoPropertiesDouble[chosenVideos.at(videoIndex)],
                               videoPropertiesInt[chosenVideos.at(videoIndex)],
                               videoPropertiesAnomaly[chosenVideos.at(videoIndex)]);
        readyToProcess.insert(chosenVideos.at(videoIndex),false);
    }
}

void MultiVideoRegistration::processVideoParameters(QJsonObject& videoData,
                                                QMap<QString, QVector<double> > &inputMMdouble,
                                                QMap<QString, QVector<int> > &inputMMint,
                                                QMap<QString, QVector<int>> &inputMManomaly)
{
    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 6){
            QJsonArray arrayDouble = videoData[videoParameters.at(parameter)].toArray();
            QVector<double> pomDouble = array2vector<double>(arrayDouble);
            inputMMdouble[videoParameters.at(parameter)] = pomDouble;
        }
        if (parameter == 6){
            QJsonArray arrayInt = videoData[videoParameters.at(parameter)].toArray();
            QVector<int> pomInt = array2vector<int>(arrayInt);
            inputMMint[videoParameters.at(parameter)] = pomInt;
        }
        if (parameter > 6){
            QJsonArray anomaly = videoData[videoParameters.at(parameter)].toArray();
            QVector<int> pomInt = array2vector<int>(anomaly);
            inputMManomaly[videoParameters.at(parameter)] = pomInt;
        }
    }
}

void MultiVideoRegistration::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData->hasUrls()) {
        return;
    }
    QStringList _list;
    QList<QUrl> urls = mimeData->urls();
    foreach (QUrl url,urls){
        QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
        if (mime.inherits("video/x-msvideo")) {
            QString path = url.toLocalFile();
            _list.append(path);
        }
    }
    if (!_list.isEmpty()) {
        foreach(QString video,_list) {
            videoListFull.append(video);
        }
        videoListFull.removeDuplicates();
        fillTable(true);
    }
    //populateProperties(videosToAdd);
    qDebug()<<"Video list update: "<<videoListNames;
}

void MultiVideoRegistration::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
      event->acceptProposedAction();
}

void MultiVideoRegistration::on_chooseMultiVPB_clicked()
{
    QStringList _list = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),
                            SharedVariables::getSharedVariables()->getPath("videosPath"),
                            tr("Video files (*.avi);;;") );
    if (!_list.isEmpty()) {
        foreach(QString video,_list) {
            videoListFull.append(video);
        }
        videoListFull.removeDuplicates();
        fillTable(true);
    }
}

void MultiVideoRegistration::on_chooseFolderPB_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                  SharedVariables::getSharedVariables()->getPath("videosPath"),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir chosenDirectory(dir);
    QStringList _list = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
    if (!_list.isEmpty()) {
        foreach(QString video,_list) {
            videoListFull.append(video);
        }
        videoListFull.removeDuplicates();
        fillTable(true);
    }
}

bool MultiVideoRegistration::checkVideo(QString i_video){
    bool _returnResult = false;
    cv::VideoCapture cap = cv::VideoCapture(i_video.toLocal8Bit().constData());
    if (cap.isOpened()){
        _returnResult = true;
    }
    return _returnResult;
}

bool MultiVideoRegistration::checkPath(QString filenameToAnalyse){
    QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+filenameToAnalyse+".dat");
    if (!videoParametersFile.exists()){
        localErrorDialogHandling[ui->registratePB]->evaluate("center","info",0);
        localErrorDialogHandling[ui->registratePB]->show(true);
        return false;
    }
    return true;
}

QWidget* MultiVideoRegistration::createIconTableItem(bool icon, QString information) {
    if (icon) {
        QLabel *lbl_item = new QLabel();
        lbl_item->setPixmap(QPixmap(":/images/"+information));
        lbl_item->setAlignment(Qt::AlignCenter);
        //lbl_item->setAlignment(Qt::AlignVCenter);
        return lbl_item;
    }
    else {
        PlayButton* playB = new PlayButton(information,this);
        return playB;
    }
}

void MultiVideoRegistration::fillTable(bool fillInternalVariables) {
    bool videoWithoutDat = false;
    bool cannotOpen = false;
    QVector<int> problematic = {};
    if (!videoListFull.isEmpty()) {
        for (int indexList = 0; indexList < videoListFull.count();indexList++)
        {
            QString folder,filename,suffix;
            processFilePath(videoListFull.at(indexList),folder,filename,suffix);
            if (checkVideo(videoListFull.at(indexList))) { // video loadable
                if (checkPath(filename)) { // .dat file exists
                    readyToProcess[videoListFull.at(indexList)] = true;
                    if (fillInternalVariables) {
                        if (!videoListNames.contains(filename))
                            videoListNames.append(filename);                       
                    }
                }
                else {
                    videoWithoutDat = true;
                    problematic.push_back(indexList);
                }
            }
            else {
                cannotOpen = true;
                problematic.push_back(indexList);
            }
        }

        for (int removeIndex=0; removeIndex < problematic.length(); removeIndex++)
            videoListFull.removeAt(removeIndex);

        ui->listOfVideos->setRowCount(videoListNames.count());
        for (int indexList = 0; indexList < videoListNames.count(); indexList++) {
             ui->listOfVideos->setItem(indexList,0,new QTableWidgetItem(videoListNames.at(indexList)));
             ui->listOfVideos->setCellWidget(indexList,1,createIconTableItem(true,"noProgress.png"));
             ui->listOfVideos->setCellWidget(indexList,2,createIconTableItem(true,"play_inactive.png"));
        }
        if (cannotOpen)
            localErrorDialogHandling[ui->listOfVideos]->evaluate("left","hardError",6);

        if (fillInternalVariables) {
            ui->listOfVideos->setColumnWidth(0,(ui->listOfVideos->width()/5)*3);
            ui->listOfVideos->setColumnWidth(1,(ui->listOfVideos->width()/5));
            ui->listOfVideos->setColumnWidth(2,(ui->listOfVideos->width()/5));
            populateProperties(videoListNames);
        }
    }

    if (videoListNames.isEmpty() && !videoWithoutDat) {
        localErrorDialogHandling[ui->registratePB]->evaluate("center","softError",3);
        localErrorDialogHandling[ui->registratePB]->show(false);
    }
    else if (!videoListNames.isEmpty())
        ui->registratePB->setEnabled(true);
}

void MultiVideoRegistration::startCalculations(cv::VideoCapture &capture){
    actualFrameCount = capture.get(CV_CAP_PROP_FRAME_COUNT);
    actualVideoName = videoListNames.at(videoCounter);
    videoPool[videoListNames.at(videoCounter)] = new QTableWidget(int(actualFrameCount),4);
    videoPool[videoListNames.at(videoCounter)]->setHorizontalHeaderLabels(columnHeaders);
    videoPool[videoListNames.at(videoCounter)]->setColumnWidth(0,ui->registrationResults->width()/4);
    videoPool[videoListNames.at(videoCounter)]->setColumnWidth(1,ui->registrationResults->width()/4);
    videoPool[videoListNames.at(videoCounter)]->setColumnWidth(2,ui->registrationResults->width()/4);
    videoPool[videoListNames.at(videoCounter)]->setColumnWidth(3,ui->registrationResults->width()/4);

    ui->registrationResults->addWidget(videoPool[videoListNames.at(videoCounter)]);
    displayStatus("startingCalculations");
    QVector<QVector<int>> threadRange = divideIntoPeaces(int(actualFrameCount),numberOfThreads);
    int threadIndex = 1;
    canProceed = true;
    for (int indexThreshold = 0; indexThreshold < threadRange[0].length() && canProceed; indexThreshold++){
        createAndRunThreads(threadIndex,capture,threadRange[0][indexThreshold],threadRange[1][indexThreshold]);
        threadIndex++;
    }
    emit calculationStarted();
}

bool MultiVideoRegistration::checkVideo(cv::VideoCapture& capture){
    bool result = false;
    if (!capture.isOpened()){
        bool goodVideoFound = false;
        videoCounter++;
        while(!goodVideoFound && videoCounter < videoListFull.count()){
            capture = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
            if (!capture.isOpened()){
                displayStatus("error");
                videoCounter++;
            }
            else{
                goodVideoFound = true;
            }
        }
        if (!goodVideoFound){
            localErrorDialogHandling[ui->listOfVideos]->evaluate("left","hardError","No video could be opened.");
            localErrorDialogHandling[ui->listOfVideos]->show(true);
            result = false;
        }
        else if(goodVideoFound){
            capture = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
            startCalculations(capture);
            result = true;
        }
    }
    else{
        startCalculations(capture);
        result = true;
    }
    return result;
}

void MultiVideoRegistration::on_registratePB_clicked()
{
    if (runStatus){
        cv::VideoCapture cap = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
        if (!checkVideo(cap))
            emit calculationStopped();
        else{
            ui->registratePB->setText(tr("Cancel"));
            runStatus = false;
            ui->registratePB->setEnabled(false);
        }
    }
    else{
        cancelAllCalculations();
        runStatus = true;
        ui->registratePB->setText(tr("Registrate"));
        canProceed = false;
        ui->progressBar->setValue(0);
        for (int i=0;i<ui->registrationResults->count();i++) {
            QWidget* widget = ui->registrationResults->widget(i);
            ui->registrationResults->removeWidget(widget);
            widget->deleteLater();
        }
        for (int i=0;i<ui->listOfVideos->rowCount();i++) {
            ui->listOfVideos->removeCellWidget(i,1);
            ui->listOfVideos->removeCellWidget(i,2);
            ui->listOfVideos->setCellWidget(i,1,createIconTableItem(true,"noProgress.png"));
            ui->listOfVideos->setCellWidget(i,2,createIconTableItem(true,"play_inactive.png"));
        }
    }
}


void MultiVideoRegistration::createAndRunThreads(int indexThread, cv::VideoCapture &cap,
                                             int lowerLimit, int upperLimit)
{
    int referencialFrameNo = findReferenceFrame(videoPropertiesInt[videoListNames.at(videoCounter)]["evaluation"]);
    cv::Mat referencialFrame;
    qDebug()<<"Referencial image for "<<videoListNames.at(videoCounter)<<": "<<referencialFrameNo;
    cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrameNo);
    if (!cap.read(referencialFrame))
        qWarning()<<"Frame "+QString::number(referencialFrameNo)+" cannot be opened.";
    QString _tempVideoName = videoListNames.at(videoCounter);
    QString _tempVideoPath = videoListFull.at(videoCounter);
    QVector<int> _evalFrames = videoPropertiesInt[videoListNames.at(videoCounter)]["evaluation"];
    int _iter = -1;
    double _arMax = 10.0;
    double _angle = 0.1;
    cv::Rect _extra = convertVector2Rect(videoPropertiesAnomaly[videoListNames.at(videoCounter)]["extra"]);
    cv::Rect _standard = convertVector2Rect(videoPropertiesAnomaly[videoListNames.at(videoCounter)]["standard"]);
    threadPool[indexThread] = new RegistrationThread(indexThread,_tempVideoPath,_tempVideoName,
                                                     SharedVariables::getSharedVariables()->getFrangiParameterWrapper(frangiType::VIDEO_SPECIFIC,videoListNames.at(videoCounter)),
                                                     _evalFrames,
                                                     referencialFrame,lowerLimit,upperLimit,_iter,_arMax,_angle,
                                                     _extra,_standard,false,
                                                     SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(frangiType::VIDEO_SPECIFIC,videoListNames.at(videoCounter)),
                                                     SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(frangiType::VIDEO_SPECIFIC,videoListNames.at(videoCounter)));

    QObject::connect(threadPool[indexThread],SIGNAL(x_coordInfo(int,int,QString)),  this,SLOT(addItem(int,int,QString)));
    QObject::connect(threadPool[indexThread],SIGNAL(y_coordInfo(int,int,QString)),  this,SLOT(addItem(int,int,QString)));
    QObject::connect(threadPool[indexThread],SIGNAL(angleInfo(int,int,QString)),    this,SLOT(addItem(int,int,QString)));
    QObject::connect(threadPool[indexThread],SIGNAL(statusInfo(int,int,QString)),   this,SLOT(addStatus(int,int,QString)));
    QObject::connect(threadPool[indexThread],SIGNAL(allWorkDone(int)),              this,SLOT(processAnother(int)));
    QObject::connect(threadPool[indexThread],SIGNAL(errorDetected(int,QString)),    this,SLOT(errorHandler(int,QString)));
    QObject::connect(threadPool[indexThread],SIGNAL(readyForFinish(int)),           this,SLOT(onFinishThread(int)));

    threadPool[indexThread]->start();
    ui->actualVideoLB->setText(tr("Processing video ")+videoListNames.at(videoCounter));
}

void MultiVideoRegistration::addItem(int row, int column, QString parameter)
{
    videoPool[videoListNames.at(videoCounter)]->setItem(row,column,new QTableWidgetItem(parameter));
}

void MultiVideoRegistration::addStatus(int row, int column, QString status){
    if (status == "error"){
        videoPool[videoListNames.at(videoCounter)]->setCellWidget(row,column,createIconTableItem(true,"everythingBad.png"));
    }
    else if (status == "done"){
        videoPool[videoListNames.at(videoCounter)]->setCellWidget(row,column,createIconTableItem(true,"everythingOK.png"));
    }
    ui->progressBar->setValue(qRound((double(internalCounter)/actualFrameCount)*100.0));
    internalCounter+=1;
    if (internalCounter > 1 && !ui->registratePB->isEnabled())
        ui->registratePB->setEnabled(true);
}
void MultiVideoRegistration::errorHandler(int indexOfThread, QString errorMessage){
    Q_UNUSED(indexOfThread)
    localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
    localErrorDialogHandling[ui->registratePB]->show(true);

    int threadPoolSize = threadPool.count();
    for (int threadIndex = 1; threadIndex <= threadPoolSize; threadIndex++){
        qDebug()<<"Terminating "<<threadIndex<<". thread because of error.";
        threadPool[threadIndex]->terminate();
        delete threadPool.take(threadIndex);
    }
    videoCounter++;
}

void MultiVideoRegistration::processAnother(int indexOfThread){
    threadProcessed++;
    processResuluts(indexOfThread);
    qDebug()<<"Thread "<<threadProcessed<<" processed.";
    if (threadProcessed == numberOfThreads){
        ui->actualVideoLB->setText(tr("Writing properly translated frames to the new video."));
        writeToVideo();
    }
}

void MultiVideoRegistration::continueAlgorithm(){
    if (readyToContinue){
        displayStatus("allDone");
        ui->actualVideoLB->setText(tr("Video written properly."));
        videoCounter++;
        threadProcessed = 0;
        internalCounter = 0;
        ui->progressBar->setValue(0);
        if (videoCounter < videoListNames.count() && videoListNames.count() != 0){
            cv::VideoCapture cap = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
            if (!checkVideo(cap))
                emit calculationStopped();
        }
        else
            emit calculationStopped();
    }
}

void MultiVideoRegistration::processResuluts(int analysedThread){
    QVector<int> range = threadPool[analysedThread]->threadFrameRange();
    QMap<QString,QVector<double>> measuredData = threadPool[analysedThread]->provideResults();
    for (int parameterIndex = 0; parameterIndex < 6; parameterIndex++){
        qDebug()<<"Processing "<<videoParameters.at(parameterIndex);
        for (int position = range[0]; position <= range[1]; position++){
            if (videoPropertiesDouble[videoListNames.at(videoCounter)][videoParameters.at(parameterIndex)][position] == 0.0)
                videoPropertiesDouble[videoListNames.at(videoCounter)][videoParameters.at(parameterIndex)][position] = measuredData[videoParameters.at(parameterIndex)][position];
        }
    }
    threadPool[analysedThread]->dataObtained();
}

void MultiVideoRegistration::processWriterError(int errorNumber){
    localErrorDialogHandling[ui->registratePB]->evaluate("center","hardError",errorNumber);
    localErrorDialogHandling[ui->registratePB]->show(true);
    readyToContinue = false;
    emit terminateWriter();
}

void MultiVideoRegistration::processWriterError(QString errorMessage){
    localErrorDialogHandling[ui->registratePB]->evaluate("center","hardError",errorMessage);
    localErrorDialogHandling[ui->registratePB]->show(true);
    readyToContinue = false;
    emit terminateWriter();
}

void MultiVideoRegistration::processSuccess(){
    readyToContinue = true;
    emit terminateWriter();
}

void MultiVideoRegistration::writeToVideo()
{
    QString whereToWrite = SharedVariables::getSharedVariables()->getPath("saveVideosPath")+"/"+videoListNames.at(videoCounter)+"_GUI.avi";
    VideoWriter* videoWriter = new VideoWriter(videoListFull.at(videoCounter),
                                               videoPropertiesDouble[videoListNames.at(videoCounter)],
                                               videoPropertiesInt[videoListNames.at(videoCounter)]["evaluation"],
                                               whereToWrite,ui->onlyBestFrames->isChecked());

    QThread* thread = new QThread;
    videoWriter->moveToThread(thread);

    QObject::connect(thread,SIGNAL(started()),videoWriter,SLOT(writeVideo()));
    QObject::connect(videoWriter, SIGNAL(finishedSuccessfully()), this, SLOT(processSuccess()));
    QObject::connect(videoWriter, SIGNAL(errorOccured(int)), this, SLOT(processWriterError(int)));
    QObject::connect(videoWriter, SIGNAL(errorOccured(QString)), this, SLOT(processWriterError(QString)));

    QObject::connect(this, SIGNAL(terminateWriter()), thread, SLOT(quit()));
    QObject::connect(this, SIGNAL(terminateWriter()), videoWriter, SLOT(deleteLater()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(thread, SIGNAL(finished()), this, SLOT(continueAlgorithm()));

    thread->start();
    /*QString fullPath = videoListFull.at(videoCounter);
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",6);
        localErrorDialogHandling[ui->registratePB]->show();
        return false;
    }
    double width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Size frameSize = cv::Size(int(width),int(height));
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    QString whereToWrite = SharedVariables::getSharedVariables()->getPath("saveVideosPath")+"/"+videoListNames.at(videoCounter)+"_GUI.avi";
    cv::VideoWriter writer;
    writer.open(whereToWrite.toLocal8Bit().constData(),static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)),cap.get(CV_CAP_PROP_FPS),frameSize,true);
    if (!writer.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",12);
        localErrorDialogHandling[ui->registratePB]->show();
        return false;
    }
    qDebug()<<"Writting video "<<videoListNames.at(videoCounter)+"_GUI.avi";
    for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
        cv::Mat shiftedOrig;
        cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
        if (cap.read(shiftedOrig)!=1)
        {
            QString errorMessage = QString("Frame %1 could not be loaded from the video for registration. Process interrupted").arg(indexImage);
            localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
            localErrorDialogHandling[ui->registratePB]->show();
            return false;
        }
        else if (videoPropertiesDouble[videoListNames.at(videoCounter)]["POCX"][indexImage] == 999.0){
            writer.write(shiftedOrig);
            shiftedOrig.release();
        }
        else{
            cv::Point3d finalTranslation(0.0,0.0,0.0);
            cv::Mat fullyRegistrated = cv::Mat::zeros(cv::Size(shiftedOrig.cols,shiftedOrig.rows), CV_32FC3);
            transformMatTypeTo8C3(shiftedOrig);
            finalTranslation.x = videoPropertiesDouble[videoListNames.at(videoCounter)]["POCX"][indexImage];
            finalTranslation.y = videoPropertiesDouble[videoListNames.at(videoCounter)]["POCY"][indexImage];
            finalTranslation.z = 0.0;
            fullyRegistrated = frameTranslation(shiftedOrig,finalTranslation,shiftedOrig.rows,shiftedOrig.cols);
            fullyRegistrated = frameRotation(fullyRegistrated,videoPropertiesDouble[videoListNames.at(videoCounter)]["angle"][indexImage]);
            writer.write(fullyRegistrated);
            shiftedOrig.release();
            fullyRegistrated.release();
        }
    }*/
}

void MultiVideoRegistration::on_saveResultsPB_clicked()
{
    for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++){
        QJsonDocument document;
        QString actualName = videoListNames.at(videoIndex);
        QString datPath = SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+actualName+".dat";
        QFile file(datPath);
        QJsonObject object = readJson(file);
        for (int parameter = 0; parameter < videoParameters.count(); parameter++){
            if (parameter < 8){
                QVector<double> pomDouble = videoPropertiesDouble[actualName][videoParameters.at(parameter)];
                QJsonArray pomArray = vector2array(pomDouble);
                object[videoParameters.at(parameter)] = pomArray;
            }
            else if (parameter >= 8 && parameter <= 12){
                QVector<int> pomInt = videoPropertiesInt[actualName][videoParameters.at(parameter)];
                QJsonArray pomArray = vector2array(pomInt);
                object[videoParameters.at(parameter)] = pomArray;
            }
            else{
                /*if (videoParameters.at(parameter) == "VerticalAnomaly"){
                    double _pom = double(SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().y);
                    if (_pom <= 0)
                        object[videoParameters.at(parameter)] = 0;
                    else
                        object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().y);
                }
                else{
                    double _pom = double(SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().x);
                    if (_pom <= 0)
                        object[videoParameters.at(parameter)] = 0;
                    else
                        object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().x);
                }*/
            }
        }
        document.setObject(object);
        QString documentString = document.toJson();
        QFile writer;
        writer.setFileName(datPath);
        writer.open(QIODevice::WriteOnly);
        writer.write(documentString.toLocal8Bit());
        writer.close();
    }

}

void MultiVideoRegistration::on_listOfVideos_cellClicked(int row, int column)
{
    qDebug()<<"Video "<<row<<" clicked.";
    Q_UNUSED(column)
    if (row <= ui->registrationResults->count()){
        ui->registrationResults->setCurrentIndex(row);
    }
}

void MultiVideoRegistration::disableWidgets(){
    ui->chooseFolderPB->setEnabled(false);
    ui->chooseMultiVPB->setEnabled(false);
    ui->saveResultsPB->setEnabled(false);
    ui->onlyBestFrames->setEnabled(false);
}

void MultiVideoRegistration::enableWidgets(){
    ui->chooseFolderPB->setEnabled(true);
    ui->chooseMultiVPB->setEnabled(true);
    ui->saveResultsPB->setEnabled(true);
    ui->onlyBestFrames->setEnabled(true);
}

void MultiVideoRegistration::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
        case Qt::Key_Delete:
            deleteSelectedFiles();
            break;
         default:
            QWidget::keyPressEvent(event);
    }
}

void MultiVideoRegistration::deleteSelectedFiles(){
    QItemSelectionModel *selection = ui->listOfVideos->selectionModel();
    QModelIndexList _list = selection->selectedRows();
    if (selection->hasSelection()) {
        foreach (QModelIndex index, _list) {
            const QAbstractItemModel* _model = index.model();
            QString selectedVideo = _model->data(index,0).toString();
            int indexOfVideo = videoListNames.indexOf(selectedVideo);
            videoListNames.removeAt(indexOfVideo);
            QStringList selectedVideoFull = videoListFull.filter(selectedVideo);
            indexOfVideo = videoListFull.indexOf(selectedVideoFull.at(0));
            videoListFull.removeAt(indexOfVideo);
            readyToProcess.remove(selectedVideo);
        }
        ui->listOfVideos->clearContents();
        fillTable(false);
    }
    qDebug()<<videoListFull;
    qDebug()<<videoListNames;
    /*QList<QTableWidgetItem*> selectedVideos = ui->listOfVideos->selectedItems();
    //qDebug()<<"Selected videos will be deleted: "<<selectedVideos;
    QVector<int> indexOfDeletion;
    foreach (QTableWidgetItem* item,selectedVideos)
    {
        int index = ui->listOfVideos->row(item);
        QString actuallyDeleted = item->text();
        if (actuallyDeleted == "")
            continue;
        indexOfDeletion.push_back(index);
        qDebug()<<"video "<<item->text()<<" will be deleted";
        //delete ui->listOfVideos->takeItem(ui->listOfVideos->row(item),1);
        QString folder,filename,suffix;
        processFilePath(actuallyDeleted,folder,filename,suffix);
        auto foundItemDouble = videoPropertiesDouble.find(filename);
        auto foundItemInt = videoPropertiesInt.find(filename);
        videoPropertiesDouble.erase(foundItemDouble);
        videoPropertiesInt.erase(foundItemInt);
        //qDebug()<<videoPropertiesDouble;
    }
    for (int deletion = 0; deletion < indexOfDeletion.length(); deletion++){
        if (deletion == 0){
            videoListFull.removeAt(indexOfDeletion[deletion]);
            videoListNames.removeAt(indexOfDeletion[deletion]);
        }
        else{
            videoListFull.removeAt(indexOfDeletion[deletion] - deletion);
            videoListNames.removeAt(indexOfDeletion[deletion] - deletion);
        }
    }
    ui->listOfVideos->clearContents();
    if (!videoListFull.isEmpty()){
        ui->listOfVideos->setRowCount(videoListFull.count());
        for (int videoIndex = 0; videoIndex < videoListFull.count(); videoIndex++){
            QTableWidgetItem* newVideo = new QTableWidgetItem(videoListNames.at(videoIndex));
            ui->listOfVideos->setItem(videoIndex,1,newVideo);
            QIcon icon(":/images/noProgress.png");
            QTableWidgetItem *icon_item = new QTableWidgetItem;
            icon_item->setIcon(icon);
            ui->listOfVideos->setItem(videoIndex, 0, icon_item);
        }
    }
    else
        ui->listOfVideos->setRowCount(1);*/
}
