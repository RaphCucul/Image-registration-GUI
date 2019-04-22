#include "dialogy/singlevideolicovani.h"
#include "licovani/registrationthread.h"
#include "licovani/fazova_korelace_funkce.h"
#include "ui_singlevideolicovani.h"
#include "util/licovaniparent.h"
#include "util/souborove_operace.h"
#include "util/prace_s_vektory.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/korelacni_koeficient.h"

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iterator>

#include <QVector>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QGridLayout>
#include <QIcon>

using cv::Mat;
using cv::Point3d;
using cv::Rect;

SingleVideoLicovani::SingleVideoLicovani(QWidget *parent) :
    LicovaniParent(parent),
    ui(new Ui::SingleVideoLicovani)
{
    ui->setupUi(this);
    ui->chooseVideoPB->setText(tr("Choose video"));
    ui->registratePB->setEnabled(false);
    ui->registratePB->setText(tr("Registrate videoframes"));
    ui->savePB->setEnabled(false);
    ui->savePB->setText(tr("Save computed parameters"));
    ui->showResultsPB->setEnabled(false);
    ui->showResultsPB->setText(tr("Show registration result"));

    ui->vysledkyLicovaniTW->setColumnCount(4);
    QStringList columnHeaders = {"X","Y",tr("Angle"),"Status"};
    ui->vysledkyLicovaniTW->setHorizontalHeaderLabels(columnHeaders);

    initMaps();

    QObject::connect(ui->registratePB,SIGNAL(clicked()),this,SLOT(registrateVideoframes()));
    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));

    localErrorDialogHandling[ui->registratePB] = new ErrorDialog(ui->registratePB);
}

SingleVideoLicovani::~SingleVideoLicovani()
{
    delete ui;
}

void SingleVideoLicovani::checkPaths(){
    if (SharedVariables::getSharedVariables()->getPath("cestaKvideim") == ""){
        ui->chooseVideoLE->setPlaceholderText(tr("Chosen video"));
        ui->chooseVideoLE->setReadOnly(true);
    }
    else
    {
        analyseAndSaveFirst(SharedVariables::getSharedVariables()->getPath("cestaKvideim"),chosenVideo);
        if (chosenVideo[1] != ""){
            QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+chosenVideo[1]+".dat");
            if (videoParametersFile.exists()){
                videoParametersJson = readJson(videoParametersFile);
                processVideoParameters(videoParametersJson);
                ui->chooseVideoLE->setText(chosenVideo[1]);
                ui->registratePB->setEnabled(true);
                ui->chooseVideoLE->setReadOnly(false);
                videoListFull.append(chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2]);
                videoListNames.append(chosenVideo[1]);
                ui->chooseVideoLE->setStyleSheet("color: #33aa00");
            }
        }
        else{
            ui->chooseVideoLE->setPlaceholderText(tr("Chosen video"));
            ui->chooseVideoLE->setReadOnly(true);
        }
    }
}

void SingleVideoLicovani::on_chooseVideoLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenVideo[0]+"/"+arg1+"."+chosenVideo[2];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chooseVideoLE->setStyleSheet("color: #FF0000");
    }
    else
    {
        QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+arg1+".dat");
        if (videoParametersFile.exists()){
            videoParametersJson = readJson(videoParametersFile);
            processVideoParameters(videoParametersJson);
            ui->registratePB->setEnabled(true);
            ui->chooseVideoLE->setReadOnly(false);
            chosenVideo[1] = arg1;
            ui->chooseVideoLE->setText(chosenVideo[1]);
            if (videoListFull.count() == 0){
                videoListFull.append(chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2]);
                videoListNames.append(chosenVideo[1]);
            }
            else{
                videoListFull.insert(0,(chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2]));
                videoListNames.insert(0,chosenVideo[1]);
            }
            ui->chooseVideoLE->setStyleSheet("color: #33aa00");
        }
        else
            ui->chooseVideoLE->setStyleSheet("color: #FF0000");
    }
}

void SingleVideoLicovani::on_chooseVideoPB_clicked()
{
    fullVideoPath = QFileDialog::getOpenFileName(this,
         tr("Choose video"), SharedVariables::getSharedVariables()->getPath("cestaKvideim"),tr("*.avi;;;"));
    QString folder,filename,suffix;
    processFilePath(fullVideoPath,folder,filename,suffix);
    cv::VideoCapture cap = cv::VideoCapture(fullVideoPath.toLocal8Bit().constData());
    ui->chooseVideoLE->setReadOnly(false);
    if (!cap.isOpened())
    {
        ui->chooseVideoLE->setText(filename);
        ui->chooseVideoLE->setStyleSheet("color: #FF0000");
    }
    else
    {
        videoListFull.append(fullVideoPath);
        videoListNames.append(filename);
        if (chosenVideo.length() == 0)
        {
            chosenVideo.push_back(folder);
            chosenVideo.push_back(filename);
            chosenVideo.push_back(suffix);
        }
        else
        {
            chosenVideo.clear();
            chosenVideo.push_back(folder);
            chosenVideo.push_back(filename);
            chosenVideo.push_back(suffix);
        }
        ui->chooseVideoLE->setText(chosenVideo[1]);
        ui->chooseVideoLE->setStyleSheet("color: #33aa00");
        QString dir = SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni");
        QDir chosenDirectory(dir);
        QStringList JsonInDirectory = chosenDirectory.entryList(QStringList() << "*.dat" << "*.DAT",QDir::Files);
        for (int a = 0; a < JsonInDirectory.count(); a++)
        {
            if (JsonInDirectory.at(a) == (chosenVideo[1]+".dat"))
            {
                if (chosenJson.length() == 0)
                {
                    chosenJson.push_back(SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni"));
                    chosenJson.push_back(chosenVideo[1]);
                    chosenJson.push_back("dat");
                }
                else
                {
                    chosenJson.clear();
                    chosenJson.push_back(SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni"));
                    chosenJson.push_back(chosenVideo[1]);
                    chosenJson.push_back("dat");
                }
            }
        }
        QFile videoParametersFile(chosenJson[0]+"/"+chosenJson[1]+"."+chosenJson[2]);
        videoParametersJson = readJson(videoParametersFile);
        processVideoParameters(videoParametersJson);
        ui->registratePB->setEnabled(true);
    }
}

void SingleVideoLicovani::registrateVideoframes()
{
    if (runStatus){
        cv::VideoCapture cap = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
        if (!cap.isOpened())
        {
            localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",6);
            localErrorDialogHandling[ui->registratePB]->show();
            return;
        }
        else{
            initMaps();
            emit calculationStarted();
            ui->registratePB->setText(tr("Cancel"));
            runStatus = false;
            canProceed = true;

            actualFrameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
            ui->vysledkyLicovaniTW->setRowCount(int(actualFrameCount));
            QVector<QVector<int>> threadRange = divideIntoPeaces(int(actualFrameCount),numberOfThreads);
            int threadIndex = 1;
            for (int indexThreshold = 0; indexThreshold < threadRange[0].length() && canProceed; indexThreshold++){
                createAndRunThreads(threadIndex,cap,threadRange[0][indexThreshold],threadRange[1][indexThreshold]);
                threadIndex++;
            }
        }
    }
    else{
        cancelAllCalculations();
        ui->registratePB->setText(tr("Registrate videoframes"));
        runStatus = true;
        ui->progress->setValue(0);
    }
}

void SingleVideoLicovani::createAndRunThreads(int indexThread, cv::VideoCapture &cap,
                                              int lowerLimit, int upperLimit)
{

    int cisloReference = findReferenceFrame(videoParametersInt["Ohodnoceni"]);
    cv::Mat referencniSnimek;
    cap.set(CV_CAP_PROP_POS_FRAMES,cisloReference);
    if (!cap.read(referencniSnimek)){
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",13);
        localErrorDialogHandling[ui->registratePB]->show();
        return;
    }
    else{
        QString _tempVideoName = videoListNames.at(videoCounter);
        QString _tempVideoPath = videoListFull.at(videoCounter);
        QVector<double> _tempFrangi = SharedVariables::getSharedVariables()->getFrangiParameters();
        QVector<int> _evalFrames = videoParametersInt["Ohodnoceni"];
        int _iter = -1;
        double _arMax = 10.0;
        double _angle = 0.1;
        int _vertAnom = videoAnomalies["VerticalAnomaly"][0];
        int _horizAnom = videoAnomalies["HorizontalAnomaly"][0];
        threadPool[indexThread] = new RegistrationThread(indexThread,_tempVideoPath,_tempVideoName,_tempFrangi,
                                                         _evalFrames,
                                                         referencniSnimek,lowerLimit,upperLimit,_iter,_arMax,_angle,
                                                         _vertAnom,_horizAnom,false);

        QObject::connect(threadPool[indexThread],SIGNAL(x_coordInfo(int,int,QString)),this,SLOT(addItem(int,int,QString)));
        QObject::connect(threadPool[indexThread],SIGNAL(y_coordInfo(int,int,QString)),this,SLOT(addItem(int,int,QString)));
        QObject::connect(threadPool[indexThread],SIGNAL(angleInfo(int,int,QString)),this,SLOT(addItem(int,int,QString)));
        QObject::connect(threadPool[indexThread],SIGNAL(statusInfo(int,int,QString)),this,SLOT(addStatus(int,int,QString)));
        QObject::connect(threadPool[indexThread],SIGNAL(allWorkDone(int)),this,SLOT(processAnother(int)));
        QObject::connect(threadPool[indexThread],SIGNAL(errorDetected(int,QString)),this,SLOT(errorHandler(int,QString)));
        QObject::connect(threadPool[indexThread],SIGNAL(readyForFinish(int)),this,SLOT(onFinishThread(int)));
        threadPool[indexThread]->start();
        ui->name_state->setText(tr("Processing video ")+videoListNames.at(videoCounter));
    }
}

void SingleVideoLicovani::totalFramesCompleted(int frameCounter)
{
    cv::VideoCapture cap = cv::VideoCapture(videoListFull.at(videoCounter).toLocal8Bit().constData());
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    ui->progress->setValue(qRound(double(frameCounter)/frameCount)*100);
}

void SingleVideoLicovani::addItem(int row, int column, QString parameter)
{
    ui->vysledkyLicovaniTW->setItem(row,column,new QTableWidgetItem(parameter));
}

void SingleVideoLicovani::addStatus(int row, int column, QString status){
    if (status == "error"){
        QIcon iconError(":/images/noProgress.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(iconError);
        ui->vysledkyLicovaniTW->setItem(row,column,icon_item);
    }
    else if (status == "done"){
        QIcon iconError(":/images/everythingOK.png");
        QTableWidgetItem *icon_item = new QTableWidgetItem;
        icon_item->setIcon(iconError);
        ui->vysledkyLicovaniTW->setItem(row,column,icon_item);
    }
    ui->progress->setValue(qRound((double(internalCounter)/actualFrameCount)*100.0));
    internalCounter+=1;
    /*if (internalCounter == 20){
        processResuluts(1);
        terminateThreads();
        writeToVideo();
    }*/
}
void SingleVideoLicovani::errorHandler(int indexOfThread, QString errorMessage){
    Q_UNUSED(indexOfThread);
    localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
    localErrorDialogHandling[ui->registratePB]->show();
    for (int var = 0; var < threadPool.count(); var++) {
        threadPool[(var+1)]->terminate();
        delete threadPool.take((var+1));
    }
    emit calculationStopped();
}
void SingleVideoLicovani::processAnother(int indexOfThread){
    threadProcessed++;
    processResuluts(indexOfThread);
    qDebug()<<"Thread "<<threadProcessed<<" processed.";
    if (threadProcessed == numberOfThreads){
        terminateThreads();
        ui->name_state->setText(tr("Writing properly translated frames to the new video."));
        if(writeToVideo()){
            ui->name_state->setText(tr("Video written properly."));
            calculationStopped();
        }
    }
}

void SingleVideoLicovani::terminateThreads(){
    for (int threadIndex = 1; threadIndex <= threadPool.count(); threadIndex++){
            qDebug()<<"Termination "<<threadIndex<<". thread.";
            threadPool[threadIndex]->terminate();
    }
}

void SingleVideoLicovani::processResuluts(int analysedThread){
    QVector<int> range = threadPool[analysedThread]->threadFrameRange();
    QMap<QString,QVector<double>> measuredData = threadPool[analysedThread]->provideResults();
    for (int parameterIndex = 0; parameterIndex < 6; parameterIndex++){
        qDebug()<<"Processing "<<videoParameters.at(parameterIndex);
        for (int position = range[0]; position <= range[1]; position++){
            if (videoParametersDouble[videoParameters.at(parameterIndex)][position] == 0.0)
                videoParametersDouble[videoParameters.at(parameterIndex)][position] = measuredData[videoParameters.at(parameterIndex)][position];
        }
    }
    threadPool[analysedThread]->dataObtained();
    //qDebug()<<videoParametersDouble["POCX"][videoCounter];
    //qDebug()<<videoParametersDouble["POCY"][videoCounter];
}
void SingleVideoLicovani::processVideoParameters(QJsonObject &videoData)
{
    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 6){
            QJsonArray arrayDouble = videoData[videoParameters.at(parameter)].toArray();
            QVector<double> pomDouble = arrayDouble2vector(arrayDouble);
            videoParametersDouble[videoParameters.at(parameter)] = pomDouble;
        }
        if (parameter == 6){
            QJsonArray arrayInt = videoData[videoParameters.at(parameter)].toArray();
            QVector<int> pomInt = arrayInt2vector(arrayInt);
            videoParametersInt[videoParameters.at(parameter)] = pomInt;
        }
        if (parameter > 6){
            int anomaly = videoData[videoParameters.at(parameter)].toInt();
            videoAnomalies[videoParameters.at(parameter)].push_back(anomaly);
        }
    }
}

bool SingleVideoLicovani::writeToVideo()
{
    QString fullPath = videoListFull.at(0);//chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",6);
        localErrorDialogHandling[ui->registratePB]->show();
        return false;
    }
    double _width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double _height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Size _frameSize = cv::Size(int(_width),int(_height));
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    QString _writePath = SharedVariables::getSharedVariables()->getPath("ulozeniVidea")+"/"+chosenVideo[1]+"_GUI.avi";
    cv::VideoWriter writer;
    writer.open(_writePath.toLocal8Bit().constData(),static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)),cap.get(CV_CAP_PROP_FPS),_frameSize,true);
    if (!writer.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",12);
        localErrorDialogHandling[ui->registratePB]->show();
        return false;
    }
    for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
        Mat movedOrig;
        cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
        if (cap.read(movedOrig)!=1)
        {
            QString errorMessage = QString(tr("Frame %1 could not be loaded from the video for registration. Process interrupted")).arg(indexImage);
            localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
            localErrorDialogHandling[ui->registratePB]->show();
            return false;
        }
        else if (videoParametersDouble["POCX"][indexImage] == 999.0){
            writer.write(movedOrig);
            movedOrig.release();
        }
        else{
            Point3d finalTranslation(0.0,0.0,0.0);
            cv::Mat _fullyRegistrated = cv::Mat::zeros(cv::Size(movedOrig.cols,movedOrig.rows), CV_32FC3);
            kontrola_typu_snimku_8C3(movedOrig);
            finalTranslation.x = videoParametersDouble["POCX"][indexImage];
            finalTranslation.y = videoParametersDouble["POCY"][indexImage];
            finalTranslation.z = 0.0;
            //qDebug()<<"For frame "<<indexImage<<" the translation is: "<<finalTranslation.x<<" "<<finalTranslation.y;
            _fullyRegistrated = translace_snimku(movedOrig,finalTranslation,movedOrig.rows,movedOrig.cols);
            _fullyRegistrated = rotace_snimku(_fullyRegistrated,videoParametersDouble["Uhel"][indexImage]);
            writer.write(_fullyRegistrated);
            movedOrig.release();
            _fullyRegistrated.release();
       }
    }
    return true;
}

void SingleVideoLicovani::on_savePB_clicked()
{

}

void SingleVideoLicovani::populateLists(QVector<QString> _file){
    videoListFull.append(_file[0]+"/"+_file[1]+"."+_file[2]);
    videoListNames.append(_file[1]);
}

void SingleVideoLicovani::disableWidgets(){
    ui->chooseVideoPB->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->showResultsPB->setEnabled(false);
}

void SingleVideoLicovani::enableWidgets(){
    ui->chooseVideoPB->setEnabled(true);
    ui->savePB->setEnabled(true);
    ui->showResultsPB->setEnabled(true);
}
