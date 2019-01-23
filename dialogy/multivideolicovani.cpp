#include "multivideolicovani.h"
#include "ui_multivideolicovani.h"
//#include "hlavni_program/t_b_ho.h"
#include "util/prace_s_vektory.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/pouzij_frangiho.h"

#include <QDebug>
#include <QMimeType>
#include <QMimeData>
#include <QMimeDatabase>
#include <QUrl>
#include <QDropEvent>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QJsonObject>
#include <QThread>
#include <QMovie>
#include <QIcon>

extern QString videaKanalyzeAktual;
extern QString ulozeniVideiAktual;
extern QString TXTnacteniAktual;
extern QString TXTulozeniAktual;
extern QString paramFrangi;

MultiVideoLicovani::MultiVideoLicovani(QWidget *parent) :
    LicovaniParent(parent),
    ui(new Ui::MultiVideoLicovani)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    QIcon leftArrow(":/images/leftArrow.png");
    QIcon rightArrow(":/images/rightArrow.png");
    ui->leftArrowPB->setIcon(leftArrow);
    ui->rightArrowPB->setIcon(rightArrow);

    ui->videoParameters->setColumnCount(4);
    QStringList columnHeaders = {"X","Y",tr("Angle"),"Status"};
    ui->videoParameters->setHorizontalHeaderLabels(columnHeaders);
    ui->listOfVideos->setRowCount(1);
    ui->listOfVideos->setColumnCount(2);
    QStringList columnHeadersList = {"Status","Video"};
    ui->listOfVideos->setHorizontalHeaderLabels(columnHeadersList);

    ui->chooseMultiVPB->setText(tr("Choose few videos"));
    ui->chooseFolderPB->setText(tr("Choose whole folder"));
    ui->deleteChosenPB->setText(tr("Delete selected"));
    ui->registratePB->setText(tr("Registrate"));
    ui->saveResultsPB->setText(tr("Save computed results"));

    velikost_frangi_opt(6,parametryFrangi);
    if (paramFrangi != ""){
        QFile soubor;
        soubor.setFileName(paramFrangi+"/frangiParameters.json");
        parametryFrangiJS = readJson(soubor);
        QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
        for (int a = 0; a < 6; a++)
        {
            inicializace_frangi_opt(parametryFrangiJS,parametry.at(a),parametryFrangi,a);
        }
    }
}

void MultiVideoLicovani::checkPaths(){
    if (paramFrangi != ""){
        QFile soubor;
        soubor.setFileName(paramFrangi+"/frangiParameters.json");
        parametryFrangiJS = readJson(soubor);
        QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
        for (int a = 0; a < 6; a++)
        {
            inicializace_frangi_opt(parametryFrangiJS,parametry.at(a),parametryFrangi,a);
        }
    }
}

void MultiVideoLicovani::populateMM(QHash<QString, QMap<QString, QVector<double> > >  &inputMM)
{
    if (inputMM.isEmpty() || videoListNames.isEmpty()){
        for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++) {
            QMap<QString,QVector<double>> pomMap;
            QVector<double> pomVec;
            for (int fillDoubles = 0; fillDoubles < 6; fillDoubles++)
                pomMap[videoParameters.at(fillDoubles)] = pomVec;
            inputMM.insert(videoListNames.at(videoIndex),pomMap);
        }
    }
    else{
        for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++) {
            if (inputMM.contains(videoListNames.at(videoIndex)))
                continue;
            else{
                QMap<QString,QVector<double>> pomMap;
                QVector<double> pomVec;
                for (int fillInts = 6; fillInts <= 8; fillInts++)
                    pomMap[videoParameters.at(fillInts)] = pomVec;
                inputMM.insert(videoListNames.at(videoIndex),pomMap);
            }
        }
    }
    qDebug()<<inputMM;
}

void MultiVideoLicovani::populateMM(QHash<QString, QMap<QString, QVector<int> > > &inputMM)
{
    if (inputMM.isEmpty() || videoListNames.isEmpty()){
        for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++) {
            QMap<QString,QVector<int>> pomMap;
            QVector<int> pomVec;
            for (int fillInts = 6; fillInts <= 8; fillInts++)
                pomMap[videoParameters.at(fillInts)] = pomVec;
            inputMM.insert(videoListNames.at(videoIndex),pomMap);
        }
    }
    else{
        for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++){
            if (inputMM.contains(videoListNames.at(videoIndex)))
                continue;
            else{
                QMap<QString,QVector<int>> pomMap;
                QVector<int> pomVec;
                for (int fillInts = 6; fillInts <= 8; fillInts++)
                    pomMap[videoParameters.at(fillInts)] = pomVec;
                inputMM.insert(videoListNames.at(videoIndex),pomMap);
            }
        }
    }
    qDebug()<<inputMM;
}

void MultiVideoLicovani::processVideoParameters(QJsonObject& videoData,
                                                QHash<QString,QMap<QString,QVector<double>>>& inputMMdouble,
                                                QHash<QString,QMap<QString,QVector<int>>>& inputMMint)
{
    for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++) {
        for (int parameter = 0; parameter < videoParameters.count(); parameter++){
            if (parameter < 6){
                QJsonArray arrayDouble = videoData[videoParameters.at(parameter)].toArray();
                QVector<double> pomDouble = arrayDouble2vector(arrayDouble);
                inputMMdouble[videoListNames.at(videoIndex)][videoParameters.at(parameter)].append(pomDouble);
            }
            if (parameter == 6){
                QJsonArray arrayInt = videoData[videoParameters.at(parameter)].toArray();
                QVector<int> pomInt = arrayInt2vector(arrayInt);
                inputMMint[videoListNames.at(videoIndex)][videoParameters.at(parameter)].append(pomInt);
                //qDebug()<<"processing video parameters"<<videoParametersInt;
            }
            if (parameter > 6){
                int anomaly = videoData[videoParameters.at(parameter)].toInt();
                inputMMint[videoListNames.at(videoIndex)][videoParameters.at(parameter)].push_back(anomaly);
            }
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
            QString addedVideo = url.toLocalFile();
            if (videoListFull.contains(addedVideo))
                continue;
            else{
                 videoListFull.append(addedVideo);
                 QString path = addedVideo;
                 QString slozka,jmeno,koncovka;
                 zpracujJmeno(path,slozka,jmeno,koncovka);
                 videoListNames.append(jmeno);
            }
        }
    }
    ui->listOfVideos->clearContents();
    ui->listOfVideos->setRowCount(videoListFull.count());
    for (int videoIndex = 0; videoIndex < videoListFull.count(); videoIndex++){
        QTableWidgetItem* newVideo = new QTableWidgetItem(videoListNames.at(videoIndex));
        ui->listOfVideos->setItem(videoIndex,1,newVideo);
        QIcon icon(":/images/noProgress.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(icon);
        //icon_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->listOfVideos->setItem(videoIndex, 0, icon_item);
    }
    /*foreach (QUrl url,urls){
        QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
        if (mime.inherits("video/x-msvideo")) {
            QString addedVideo = url.toLocalFile();            
            if (videoListFull.contains(addedVideo))
                continue;
            else{
                ui->listOfVideos->insertRow(1);
                videoListFull.append(url.toLocalFile());
                QString path = url.toLocalFile();
                QString slozka,jmeno,koncovka;
                zpracujJmeno(path,slozka,jmeno,koncovka);
                videoListNames.append(jmeno);
                QTableWidgetItem* newVideo = new QTableWidgetItem(jmeno);
                ui->listOfVideos->setItem(TableRowCounter,1,newVideo);
                TableRowCounter+=1;
                ui->listOfVideos->setRowCount(TableRowCounter+2);
            }
        }
    }*/
    //sezVid = seznamVidei;
    qDebug()<<"Aktualizace seznamu videi: "<<videoListNames;
    populateMM(videoPropertiesDouble);
    populateMM(videoPropertiesInt);
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
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),videaKanalyzeAktual,tr("Video files (*.avi);;;") );
    if( !filenames.isEmpty() )
    {
        ui->listOfVideos->setRowCount(filenames.count());
        for (int i =0;i<filenames.count();i++)
        {
            if (videoListFull.contains(filenames.at(i)))
                continue;
            else{
                videoListFull.append(filenames.at(i));
                QString path = filenames.at(i);
                QString slozka,jmeno,koncovka;
                zpracujJmeno(path,slozka,jmeno,koncovka);
                videoListNames.append(jmeno);
                QTableWidgetItem* newVideo = new QTableWidgetItem(jmeno);
                ui->listOfVideos->setItem(i,1,newVideo);
                QIcon icon(":/images/noProgress.png");
                QTableWidgetItem *icon_item = new QTableWidgetItem;
                icon_item->setIcon(icon);
                //icon_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                ui->listOfVideos->setItem(i, 0, icon_item);
            }
        }
    }
    populateMM(videoPropertiesDouble);
    populateMM(videoPropertiesInt);
}

void MultiVideoLicovani::on_chooseFolderPB_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),videaKanalyzeAktual,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir chosenDirectory(dir);
    //qDebug()<<"Chosen directory:"<<dir;
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
    //qDebug()<<"contains "<<videosInDirectory.length();
    if (!videosInDirectory.isEmpty())
    {
        ui->listOfVideos->setRowCount(videosInDirectory.count());
        for (int a = 0; a < videosInDirectory.count();a++)
        {
            if (videoListFull.contains(videosInDirectory.at(a)))
                continue;
            else{
                videoListFull.append(videosInDirectory.at(a));
                QString path = videosInDirectory.at(a);
                QString slozka,jmeno,koncovka;
                zpracujJmeno(path,slozka,jmeno,koncovka);
                videoListNames.append(jmeno);
                QTableWidgetItem* newVideo = new QTableWidgetItem(jmeno);
                ui->listOfVideos->setItem(a,1,newVideo);
                QIcon icon(":/images/noProgress.png");
                QTableWidgetItem *icon_item = new QTableWidgetItem;
                icon_item->setIcon(icon);
                //icon_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                ui->listOfVideos->setItem(a, 0, icon_item);
            }
        }
    }
    qDebug()<<"sezVid contains "<<videoListFull.count()<<" videos.";
    populateMM(videoPropertiesDouble);
    populateMM(videoPropertiesInt);
}

void MultiVideoLicovani::on_deleteChosenPB_clicked()
{
    QList<QTableWidgetItem*> selectedVideos = ui->listOfVideos->selectedItems();
    //qDebug()<<"Selected videos will be deleted: "<<selectedVideos;
    QVector<int> indexOfDeletion;
    foreach (QTableWidgetItem* item,selectedVideos)
    {
        int index = ui->listOfVideos->row(item);        
        QString slozka,jmeno,koncovka;
        QString actuallyDeleted = item->text();
        if (actuallyDeleted == "")
            continue;
        indexOfDeletion.push_back(index);
        zpracujJmeno(actuallyDeleted,slozka,jmeno,koncovka);
        qDebug()<<"video "<<item->text()<<" will be deleted";
        //delete ui->listOfVideos->takeItem(ui->listOfVideos->row(item),1);

        auto foundItemDouble = videoPropertiesDouble.find(jmeno);
        auto foundItemInt = videoPropertiesInt.find(jmeno);
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

void MultiVideoLicovani::on_registratePB_clicked()
{
    for (int videoIndex = 0; videoIndex < videoListNames.count(); videoIndex++) {
        QFile videoParametersFile(TXTnacteniAktual+"/"+videoListNames.at(videoIndex)+".dat");
        QJsonObject videoParametersJson = readJson(videoParametersFile);
        processVideoParameters(videoParametersJson,videoPropertiesDouble,videoPropertiesInt);
    }
    cv::VideoCapture cap = cv::VideoCapture(videoListFull.at(0).toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        qWarning()<<"Unable to open "+videoListNames.at(0);
    }

    int numberOfThreads = 2;//QThread::idealThreadCount();
    actualFrameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    actualVideoName = videoListNames.at(0);
    ui->videoParameters->setRowCount(int(actualFrameCount));
    ui->videoParameters->setColumnCount(1);
    threadRange = divideIntoPeaces(int(actualFrameCount),numberOfThreads);
    QVector<int> pomThreadsVec(threadRange[0].length());
    std::generate(pomThreadsVec.begin(),pomThreadsVec.end(),[n = 1] () mutable { return n++; });
    totalSum = vectorSum(pomThreadsVec);    
    for (int indexThreshold = 0; indexThreshold < threadRange[0].length(); indexThreshold++){
        createAndRunThreads(0,pomThreadsVec[indexThreshold],cap,threadRange[0][indexThreshold],threadRange[1][indexThreshold]);
        //qDebug()<<threadRange[0][indexThreshold];
        //qDebug()<<threadRange[1][indexThreshold];
    }
}


void MultiVideoLicovani::createAndRunThreads(int indexProcVid, int indexThread, cv::VideoCapture &cap,
                                              int lowerLimit, int upperLimit)
{
    QVector<int> ohodnoceniActualVideo = videoPropertiesInt[videoListNames.at(indexProcVid)]["Ohodnoceni"];
    qDebug()<<ohodnoceniActualVideo;
    int cisloReference = findReferenceFrame(ohodnoceniActualVideo);
    cv::Mat referencniSnimek;
    cap.set(CV_CAP_PROP_POS_FRAMES,cisloReference);
    if (!cap.read(referencniSnimek))
        qWarning()<<"Frame "+QString::number(cisloReference)+" cannot be opened.";
    regThread = new RegistrationThread(cap,
                                       indexThread,
                                       videoListNames.at(indexProcVid),
                                       parametryFrangi,
                                       ohodnoceniActualVideo,
                                       referencniSnimek,
                                       lowerLimit,upperLimit,
                                       -1,10.0,0.1,
                                       videoPropertiesInt[videoListNames.at(indexProcVid)]["VerticalAnomaly"][indexProcVid],
                                       videoPropertiesInt[videoListNames.at(indexProcVid)]["HorizontalAnomaly"][indexProcVid],
                                       false);
    existingThreads[indexThread] = regThread;
    //QObject::connect(regThread,SIGNAL(frameCompleted(int)),this,SLOT(totalFramesCompleted(int)));
    //QObject::connect(regThread,SIGNAL(frameCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
    QObject::connect(regThread,SIGNAL(numberOfFrame(int,int,QTableWidgetItem*)),this,SLOT(addItem(int,int,QTableWidgetItem*)));
    QObject::connect(regThread,SIGNAL(allWorkDone(int)),this,SLOT(registrateVideoFrames(int)));
    regThread->start();
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

void MultiVideoLicovani::registrateVideoFrames(int number)
{
    qDebug()<<"Thread "<<number<<" finished.";
    controlSum += number;
    if (controlSum == totalSum){
        for (int threadRangeIndex = 0; threadRangeIndex < threadRange[0].length(); threadRangeIndex++){
            int start = threadRange[0][threadRangeIndex];
            int stop = threadRange[1][threadRangeIndex];
            RegistrationThread *pomRegTh = existingThreads[threadRangeIndex+1];
            QMap<QString,QVector<double>> threadResults = pomRegTh->provideResults();
            saveTheResults(threadResults,start,stop);
            pomRegTh->quit();
        }
        qDebug()<<videoPropertiesDouble[actualVideoName]["FrangiX"];
        qDebug()<<videoPropertiesDouble[actualVideoName]["FrangiY"];
        qDebug()<<videoPropertiesDouble[actualVideoName]["FrangiEuklid"];
        qDebug()<<videoPropertiesDouble[actualVideoName]["POCX"];
        qDebug()<<videoPropertiesDouble[actualVideoName]["POCY"];
        qDebug()<<videoPropertiesDouble[actualVideoName]["Uhel"];
    }

}

void MultiVideoLicovani::addItem(int row,int column, QTableWidgetItem* item)
{
    ui->videoParameters->setItem(row,column,item);
    //ui->prubehVypoctu->setValue(qRound(double(internalCounter)/frameCount)*100);
    ui->progressBar->setValue(qRound((double(internalCounter)/actualFrameCount)*100));
    internalCounter+=1;
}

int MultiVideoLicovani::writeToVideo()
{
    for (int indexVideo = 0; indexVideo < videoListFull.count(); indexVideo++){
        QString kompletni_cesta = videoListFull.at(indexVideo);
        cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
        double sirka_framu = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        double vyska_framu = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        double FPSvidea = cap.get(CV_CAP_PROP_FPS);
        cv::Size velikostSnimku = cv::Size(int(sirka_framu),int(vyska_framu));
        double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
        QString cestaZapis = ulozeniVideiAktual+"/"+videoListNames.at(indexVideo)+"_GUI.avi";
        cv::VideoWriter writer = cv::VideoWriter(cestaZapis.toLocal8Bit().constData(),
                CV_FOURCC('F','F','V','1'),FPSvidea,velikostSnimku,true);
        if (!writer.isOpened())
        {
            qWarning()<<"Video nelze zapsat. Cesta neexistuje.";
            return 0;
        }
        for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
            cv::Mat posunutyOrig,posunutyLicovani,posunutyLicovaniFinal;
            cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
            if (cap.read(posunutyOrig)!=1)
            {
                qWarning()<< "Image "<<indexImage<<" cannot be registrated.";
                continue;
            }
            else{
                cv::Point3d finalTranslation;
                finalTranslation.x = videoParametersDouble["POCX"][0][indexImage];
                finalTranslation.y = videoParametersDouble["POCY"][0][indexImage];
                finalTranslation.z = 0.0;
                posunutyLicovani = translace_snimku(posunutyOrig,finalTranslation,posunutyOrig.rows,posunutyOrig.cols);
                posunutyLicovaniFinal = rotace_snimku(posunutyLicovani,videoParametersDouble["Uhel"][0][indexImage]);
                writer.write(posunutyLicovaniFinal);
                posunutyOrig.release();
                posunutyLicovani.release();
                posunutyLicovaniFinal.release();
            }
        }
    }
}

void MultiVideoLicovani::on_saveResultsPB_clicked()
{

}

void MultiVideoLicovani::on_listOfVideos_cellDoubleClicked(int row, int column)
{

}
