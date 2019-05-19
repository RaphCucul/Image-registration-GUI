#include "multivideolicovani.h"
#include "ui_multivideolicovani.h"
#include "util/prace_s_vektory.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "fancy_staff/sharedvariables.h"
#include "fancy_staff/errors.h"

#include <QDebug>
#include <QMimeType>
#include <QMimeData>
#include <QMimeDatabase>
#include <QUrl>
#include <QDropEvent>
#include <QFileDialog>
#include <QTableWidget>
#include <QJsonObject>
#include <QThread>
#include <QIcon>
#include <QHash>

MultiVideoLicovani::MultiVideoLicovani(QWidget *parent) :
    LicovaniParent(parent),
    ui(new Ui::MultiVideoLicovani)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    /*QIcon leftArrow(":/images/leftArrow.png");
    QIcon rightArrow(":/images/rightArrow.png");
    ui->leftArrowPB->setIcon(leftArrow);
    ui->rightArrowPB->setIcon(rightArrow);*/

    //ui->videoParameters->setColumnCount(4);

   // ui->videoParameters->setHorizontalHeaderLabels(columnHeaders);
    ui->listOfVideos->setRowCount(1);
    ui->listOfVideos->setColumnCount(2);
    QStringList columnHeadersList = {"Status","Video"};
    ui->listOfVideos->setHorizontalHeaderLabels(columnHeadersList);

    ui->chooseMultiVPB->setText(tr("Choose few videos"));
    ui->chooseFolderPB->setText(tr("Choose whole folder"));
    ui->deleteChosenPB->setText(tr("Delete selected"));
    ui->registratePB->setText(tr("Registrate"));
    ui->saveResultsPB->setText(tr("Save computed results"));

    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));

    localErrorDialogHandling[ui->listOfVideos] = new ErrorDialog(ui->listOfVideos);
    localErrorDialogHandling[ui->registratePB] = new ErrorDialog(ui->registratePB);
}

bool MultiVideoLicovani::checkPath(QString filenameToAnalyse){
    QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("adresarTXT_nacteni")+"/"+filenameToAnalyse+".dat");
    if (!videoParametersFile.exists()){
        localErrorDialogHandling[ui->listOfVideos]->evaluate("center","info",0);
        localErrorDialogHandling[ui->listOfVideos]->show();
        return false;
    }
    return true;
}

void MultiVideoLicovani::displayStatus(QString status){
    if (status == "startingCalculations"){
        QIcon icon(":/images/currentlyProcessed.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(icon);
        ui->listOfVideos->setItem(videoCounter, 0, icon_item);
    }
    else if (status == "noProgress"){
        QIcon icon(":/images/noProgress.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(icon);
        ui->listOfVideos->setItem(videoCounter, 0, icon_item);
    }
    else if (status == "allDone"){
        QIcon icon(":/images/everythingOK.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(icon);
        ui->listOfVideos->setItem(videoCounter, 0, icon_item);
    }
    else if (status == "error"){
        QIcon icon(":/images/everythingBad.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(icon);
        ui->listOfVideos->setItem(videoCounter, 0, icon_item);
    }
}

void MultiVideoLicovani::populateProperties(QStringList chosenVideos)
{    
    for (int videoIndex = 0; videoIndex < chosenVideos.count(); videoIndex++) {
        QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("adresarTXT_nacteni")+"/"+chosenVideos.at(videoIndex)+".dat");
        QJsonObject videoParametersJson = readJson(videoParametersFile);
        videoPropertiesDouble[chosenVideos.at(videoIndex)] = videoParametersDouble;
        videoPropertiesInt[chosenVideos.at(videoIndex)] = videoParametersInt;
        videoPropertiesAnomaly[chosenVideos.at(videoIndex)] = videoAnomalies;
        processVideoParameters(videoParametersJson,
                               videoPropertiesDouble[chosenVideos.at(videoIndex)],
                               videoPropertiesInt[chosenVideos.at(videoIndex)],
                               videoPropertiesAnomaly[chosenVideos.at(videoIndex)]);
    }
}

void MultiVideoLicovani::processVideoParameters(QJsonObject& videoData,
                                                QMap<QString, QVector<double> > &inputMMdouble,
                                                QMap<QString, QVector<int> > &inputMMint,
                                                QMap<QString, QVector<int>> &inputMManomaly)
{
    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 6){
            QJsonArray arrayDouble = videoData[videoParameters.at(parameter)].toArray();
            QVector<double> pomDouble = arrayDouble2vector(arrayDouble);
            inputMMdouble[videoParameters.at(parameter)] = pomDouble;
        }
        if (parameter == 6){
            QJsonArray arrayInt = videoData[videoParameters.at(parameter)].toArray();
            QVector<int> pomInt = arrayInt2vector(arrayInt);
            inputMMint[videoParameters.at(parameter)] = pomInt;
        }
        if (parameter > 6){
            int anomaly = videoData[videoParameters.at(parameter)].toInt();
            inputMManomaly[videoParameters.at(parameter)].push_back(anomaly);
        }
    }
}

void MultiVideoLicovani::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData->hasUrls()) {
        return;
    }
    QStringList videosToAdd;
    QList<QUrl> urls = mimeData->urls();
    foreach (QUrl url,urls){
        QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
        if (mime.inherits("video/x-msvideo")) {
            QString path = url.toLocalFile();
            if (videoListFull.contains(path))
                continue;
            else{
                QString folder,filename,suffix;
                processFilePath(path,folder,filename,suffix);
                if (checkPath(filename)){
                    videoListFull.append(path);
                    videoListNames.append(filename);
                    videosToAdd.append(filename);
                }
            }
        }
    }
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
        populateProperties(videosToAdd);
        qDebug()<<"Aktualizace seznamu videi: "<<videoListNames;
    }
}

void MultiVideoLicovani::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
      event->acceptProposedAction();
}

MultiVideoLicovani::~MultiVideoLicovani()
{
    delete ui;
}

void MultiVideoLicovani::on_chooseMultiVPB_clicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),
                            SharedVariables::getSharedVariables()->getPath("cestaKvideim"),
                            tr("Video files (*.avi);;;") );
    if( !filenames.isEmpty() )
    {
        QStringList videosToAdd;
        ui->listOfVideos->setRowCount(filenames.count());
        for (int i =0;i<filenames.count();i++)
        {
            if (videoListFull.contains(filenames.at(i)))
                continue;
            else{
                QString path = filenames.at(i);
                QString folder,filename,suffix;
                processFilePath(path,folder,filename,suffix);

                if (checkPath(filename)){
                    videoListFull.append(filenames.at(i));
                    videoListNames.append(filename);
                    videosToAdd.append(filename);
                    QTableWidgetItem* newVideo = new QTableWidgetItem(filename);
                    ui->listOfVideos->setItem(i,1,newVideo);
                    QIcon icon(":/images/noProgress.png");
                    QTableWidgetItem *icon_item = new QTableWidgetItem;
                    icon_item->setIcon(icon);
                    ui->listOfVideos->setItem(i, 0, icon_item);
                    populateProperties(videosToAdd);
                }
            }
        }
    }
}

void MultiVideoLicovani::on_chooseFolderPB_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                  SharedVariables::getSharedVariables()->getPath("cestaKvideim"),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir chosenDirectory(dir);
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
    //qDebug()<<"contains "<<videosInDirectory.length();
    if (!videosInDirectory.isEmpty())
    {
        QStringList videosToAdd;
        ui->listOfVideos->setRowCount(videosInDirectory.count());
        for (int a = 0; a < videosInDirectory.count();a++)
        {
            if (videoListFull.contains(videosInDirectory.at(a)))
                continue;
            else{
                QString path = videosInDirectory.at(a);
                QString folder,filename,suffix;
                processFilePath(path,folder,filename,suffix);
                if (checkPath(filename)){
                    videoListFull.append(videosInDirectory.at(a));
                    videoListNames.append(filename);
                    videosToAdd.append(filename);
                    QTableWidgetItem* newVideo = new QTableWidgetItem(filename);
                    ui->listOfVideos->setItem(a,1,newVideo);
                    QIcon icon(":/images/noProgress.png");
                    QTableWidgetItem *icon_item = new QTableWidgetItem;
                    icon_item->setIcon(icon);
                    ui->listOfVideos->setItem(a, 0, icon_item);
                    populateProperties(videosToAdd);
                }
            }
        }
    }
    qDebug()<<"sezVid contains "<<videoListFull.count()<<" videos.";
}

void MultiVideoLicovani::on_deleteChosenPB_clicked()
{
    QList<QTableWidgetItem*> selectedVideos = ui->listOfVideos->selectedItems();
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
        qDebug()<<videoPropertiesDouble;
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
        ui->listOfVideos->setRowCount(1);
}

void MultiVideoLicovani::startCalculations(cv::VideoCapture &capture){
    actualFrameCount = capture.get(CV_CAP_PROP_FRAME_COUNT);
    actualVideoName = videoListNames.at(videoCounter);
    videoPool[videoListNames.at(videoCounter)] = new QTableWidget(int(actualFrameCount),4);
    videoPool[videoListNames.at(videoCounter)]->setHorizontalHeaderLabels(columnHeaders);
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

bool MultiVideoLicovani::checkVideo(cv::VideoCapture& capture){
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
            localErrorDialogHandling[ui->listOfVideos]->show();
            return false;
        }
        else if(goodVideoFound){
            capture = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
            startCalculations(capture);
            return true;
        }
    }
    else{
        startCalculations(capture);
        return true;
    }
}

void MultiVideoLicovani::on_registratePB_clicked()
{
    if (runStatus){
        cv::VideoCapture cap = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
        if (!checkVideo(cap))
            emit calculationStopped();
        else{
            ui->registratePB->setText(tr("Cancel"));
            runStatus = false;
        }
    }
    else{
        runStatus = true;
        ui->registratePB->setText(tr("Registrate"));
        canProceed = false;
        ui->progressBar->setValue(0);
    }
}


void MultiVideoLicovani::createAndRunThreads(int indexThread, cv::VideoCapture &cap,
                                             int lowerLimit, int upperLimit)
{
    int referencialFrameNo = findReferenceFrame(videoPropertiesInt[videoListNames.at(videoCounter)]["Ohodnoceni"]);
    cv::Mat referencialFrame;
    qDebug()<<"Referencial image for "<<videoListNames.at(videoCounter)<<": "<<referencialFrameNo;
    cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrameNo);
    if (!cap.read(referencialFrame))
        qWarning()<<"Frame "+QString::number(referencialFrameNo)+" cannot be opened.";
    QString _tempVideoName = videoListNames.at(videoCounter);
    QString _tempVideoPath = videoListFull.at(videoCounter);
    QVector<double> _tempFrangi = SharedVariables::getSharedVariables()->getFrangiParameters();
    QVector<int> _evalFrames = videoPropertiesInt[videoListNames.at(videoCounter)]["Ohodnoceni"];
    int _iter = -1;
    double _arMax = 10.0;
    double _angle = 0.1;
    int _vertAnom = videoPropertiesAnomaly[videoListNames.at(videoCounter)]["VerticalAnomaly"][0];
    int _horizAnom = videoPropertiesAnomaly[videoListNames.at(videoCounter)]["HorizontalAnomaly"][0];
    threadPool[indexThread] = new RegistrationThread(indexThread,_tempVideoPath,_tempVideoName,_tempFrangi,
                                                     _evalFrames,
                                                     referencialFrame,lowerLimit,upperLimit,_iter,_arMax,_angle,
                                                     _vertAnom,_horizAnom,false,
                                                     SharedVariables::getSharedVariables()->getFrangiMargins(),
                                                     SharedVariables::getSharedVariables()->getFrangiRatios());

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

void MultiVideoLicovani::addItem(int row, int column, QString parameter)
{
    videoPool[videoListNames.at(videoCounter)]->setItem(row,column,new QTableWidgetItem(parameter));
}

void MultiVideoLicovani::addStatus(int row, int column, QString status){
    if (status == "error"){
        QIcon iconError(":/images/everythingBad.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(iconError);
        videoPool[videoListNames.at(videoCounter)]->setItem(row,column,icon_item);
    }
    else if (status == "done"){
        QIcon iconError(":/images/everythingOK.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(iconError);
        videoPool[videoListNames.at(videoCounter)]->setItem(row,column,icon_item);
    }
    ui->progressBar->setValue(qRound((double(internalCounter)/actualFrameCount)*100.0));
    internalCounter+=1;
}
void MultiVideoLicovani::errorHandler(int indexOfThread, QString errorMessage){
    Q_UNUSED(indexOfThread);
    localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
    localErrorDialogHandling[ui->registratePB]->show();

    int threadPoolSize = threadPool.count();
    for (int threadIndex = 1; threadIndex <= threadPoolSize; threadIndex++){
        qDebug()<<"Terminating "<<threadIndex<<". thread because of error.";
        threadPool[threadIndex]->terminate();
        delete threadPool.take(threadIndex);
    }
    videoCounter++;
}

void MultiVideoLicovani::processAnother(int indexOfThread){
    threadProcessed++;
    processResuluts(indexOfThread);
    qDebug()<<"Thread "<<threadProcessed<<" processed.";
    if (threadProcessed == numberOfThreads){
        ui->actualVideoLB->setText(tr("Writing properly translated frames to the new video."));
        if(writeToVideo()){
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
                calculationStopped();
        }
    }
}

void MultiVideoLicovani::processResuluts(int analysedThread){
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

void MultiVideoLicovani::saveTheResults(QMap<QString, QVector<double> > input, int from, int to)
{
    for (int index = from; index <= to; index++){
        videoPropertiesDouble[actualVideoName]["FrangiX"][index] = input["FrangiX"][index];
        videoPropertiesDouble[actualVideoName]["FrangiY"][index] = input["FrangiY"][index];
        videoPropertiesDouble[actualVideoName]["FrangiEuklid"][index] = input["FrangiEuklid"][index];
        videoPropertiesDouble[actualVideoName]["POCX"][index] = input["FinalPOCx"][index];
        videoPropertiesDouble[actualVideoName]["POCY"][index] = input["FinalPOCy"][index];
        videoPropertiesDouble[actualVideoName]["Uhel"][index] = input["Angles"][index];
    }
}

int MultiVideoLicovani::writeToVideo()
{
    QString fullPath = videoListFull.at(videoCounter);
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
    QString whereToWrite = SharedVariables::getSharedVariables()->getPath("ulozeniVidea")+"/"+videoListNames.at(videoCounter)+"_GUI.avi";
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
            fullyRegistrated = frameRotation(fullyRegistrated,videoPropertiesDouble[videoListNames.at(videoCounter)]["Uhel"][indexImage]);
            writer.write(fullyRegistrated);
            shiftedOrig.release();
            fullyRegistrated.release();
        }
    }
    return true;
}

void MultiVideoLicovani::on_saveResultsPB_clicked()
{

}

void MultiVideoLicovani::on_listOfVideos_cellClicked(int row, int column)
{
    qDebug()<<"Video "<<row<<" clicked.";
    Q_UNUSED(column);
    if (row <= ui->registrationResults->count()){
        ui->registrationResults->setCurrentIndex(row);
    }
}

void MultiVideoLicovani::disableWidgets(){
    ui->chooseFolderPB->setEnabled(false);
    ui->chooseMultiVPB->setEnabled(false);
    ui->deleteChosenPB->setEnabled(false);
    ui->saveResultsPB->setEnabled(false);
}

void MultiVideoLicovani::enableWidgets(){
    ui->chooseFolderPB->setEnabled(true);
    ui->chooseMultiVPB->setEnabled(true);
    ui->deleteChosenPB->setEnabled(true);
    ui->saveResultsPB->setEnabled(true);
}
