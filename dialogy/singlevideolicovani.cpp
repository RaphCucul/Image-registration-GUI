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

    QVector<QVector<double>> pomD;
    QVector<QVector<int>> pomI;
    QVector<int> pomI_;
    videoParametersDouble["FrangiX"]=pomD;
    videoParametersDouble["FrangiX"]=pomD;
    videoParametersDouble["FrangiEuklid"]=pomD;
    videoParametersDouble["POCX"]=pomD;
    videoParametersDouble["POCY"]=pomD;
    videoParametersDouble["Uhel"]=pomD;
    videoParametersInt["Ohodnoceni"]=pomI;
    videoAnomalies["VerticalAnomaly"]=pomI_;
    videoAnomalies["HorizontalAnomaly"]=pomI_;

    ui->vysledkyLicovaniTW->setColumnCount(4);
    QStringList columnHeaders = {"X","Y",tr("Angle"),"Status"};
    ui->vysledkyLicovaniTW->setHorizontalHeaderLabels(columnHeaders);

    QObject::connect(ui->registratePB,SIGNAL(clicked()),this,SLOT(registrateVideoframes()));

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
                videoList.append(chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2]);
                videoListNames.append(chosenVideo[1]);
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
    QString kompletni_cesta = chosenVideo[0]+"/"+arg1+"."+chosenVideo[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chooseVideoLE->setStyleSheet("color: #FF0000");
        ui->chooseVideoLE->setReadOnly(true);
    }
    else
    {
        QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+arg1+".dat");
        if (videoParametersFile.exists()){
            videoParametersJson = readJson(videoParametersFile);
            processVideoParameters(videoParametersJson);
            ui->chooseVideoLE->setText(chosenVideo[1]);
            ui->registratePB->setEnabled(true);
            ui->chooseVideoLE->setReadOnly(false);
            chosenVideo[1] = arg1;
            if (videoList.count() == 0){
                videoList.append(chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2]);
                videoListNames.append(chosenVideo[1]);
            }
            else{
                videoList.insert(0,(chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2]));
                videoListNames.insert(0,chosenVideo[1]);
            }
        }
        ui->chooseVideoLE->setStyleSheet("color: #33aa00");
        chosenVideo[1] = arg1;
    }
}

void SingleVideoLicovani::on_chooseVideoPB_clicked()
{
    fullVideoPath = QFileDialog::getOpenFileName(this,
         tr("Choose video"), SharedVariables::getSharedVariables()->getPath("cestaKvideim"),"*.avi;;All files (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    processFilePath(fullVideoPath,vybrana_slozka,vybrany_soubor,koncovka);
    cv::VideoCapture cap = cv::VideoCapture(fullVideoPath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chooseVideoLE->setText(vybrany_soubor);
        ui->chooseVideoLE->setStyleSheet("color: #FF0000");
    }
    else
    {
        videoList.append(fullVideoPath);
        videoListNames.append(vybrany_soubor);
        if (chosenVideo.length() == 0)
        {
            chosenVideo.push_back(vybrana_slozka);
            chosenVideo.push_back(vybrany_soubor);
            chosenVideo.push_back(koncovka);
        }
        else
        {
            chosenVideo.clear();
            chosenVideo.push_back(vybrana_slozka);
            chosenVideo.push_back(vybrany_soubor);
            chosenVideo.push_back(koncovka);
        }
        ui->chooseVideoLE->setText(chosenVideo[1]);
        ui->chooseVideoLE->setStyleSheet("color: #339900");
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
    cv::VideoCapture cap = cv::VideoCapture(videoList.at(videoCounter).toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",6);
        localErrorDialogHandling[ui->registratePB]->show();
    }
    //framesFinalCompleteDecision.append(videoParametersInt["Ohodnoceni"][videoCounter]);

    //qDebug()<<videoParametersInt;
    //qDebug()<<videoParametersInt["Ohodnoceni"][0];

    /// realizace jednovlaknova
    //qDebug()<<snimkyOhodnoceniKomplet[0];
    /*int cisloReference = findReferenceFrame(snimkyOhodnoceniKomplet[0]);
    qDebug()<<"nalezena reference:"<<cisloReference;
    cv::Mat referencniSnimek;
    cap.set(CV_CAP_PROP_POS_FRAMES,cisloReference);
    if (!cap.read(referencniSnimek))
        qWarning()<<"Frame "+QString::number(cisloReference)+" cannot be opened.";
    float timeS = 0.0;float lightA = 0.0;
    licuj(cap,parametryFrangi,referencniSnimek,0,260,-1,10.0,0.1,timeS,lightA);
    */

    /// realizace v separatnim vlaknu
    actualFrameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    ui->vysledkyLicovaniTW->setRowCount(int(actualFrameCount));
    QVector<QVector<int>> threadRange = divideIntoPeaces(int(actualFrameCount),numberOfThreads);
    int threadIndex = 1;
    for (int indexThreshold = 0; indexThreshold < threadRange[0].length(); indexThreshold++){
        createAndRunThreads(threadIndex,cap,threadRange[0][indexThreshold],threadRange[1][indexThreshold]);
        threadIndex++;
    }
}

void SingleVideoLicovani::createAndRunThreads(int indexThread, cv::VideoCapture &cap,
                                              int lowerLimit, int upperLimit)
{

    int cisloReference = findReferenceFrame(videoParametersInt["Ohodnoceni"][videoCounter]);
    cv::Mat referencniSnimek;
    cap.set(CV_CAP_PROP_POS_FRAMES,cisloReference);
    if (!cap.read(referencniSnimek))
        qWarning()<<"Frame "+QString::number(cisloReference)+" cannot be opened.";
    QString _tempVideoName = videoListNames.at(videoCounter);
    QString _tempVideoPath = videoList.at(videoCounter);
    QVector<double> _tempFrangi = SharedVariables::getSharedVariables()->getFrangiParameters();
    QVector<int> _evalFrames = videoParametersInt["Ohodnoceni"][videoCounter];
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
    threadPool[indexThread]->start();
    ui->name_state->setText(tr("Processing video ")+videoListNames.at(videoCounter));
    emit calculationStarted();
}

void SingleVideoLicovani::totalFramesCompleted(int frameCounter)
{
    cv::VideoCapture cap = cv::VideoCapture(videoList.at(videoCounter).toLocal8Bit().constData());
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
            delete threadPool.take(threadIndex);
    }
    qDebug()<<"Actually active threads: "<<threadPool.size();
}

void SingleVideoLicovani::processResuluts(int analysedThread){
    QVector<int> range = threadPool[analysedThread]->threadFrameRange();
    QMap<QString,QVector<double>> measuredData = threadPool[analysedThread]->provideResults();
    for (int parameterIndex = 0; parameterIndex < 6; parameterIndex++){
        qDebug()<<"Processing "<<videoParameters.at(parameterIndex);
        for (int position = range[0]; position <= range[1]; position++){
            if (videoParametersDouble[videoParameters.at(parameterIndex)][videoCounter][position] == 0.0)
                videoParametersDouble[videoParameters.at(parameterIndex)][videoCounter][position] = measuredData[videoParameters.at(parameterIndex)][position];
        }
    }
    //qDebug()<<videoParametersDouble["POCX"][videoCounter];
    //qDebug()<<videoParametersDouble["POCY"][videoCounter];
}
void SingleVideoLicovani::processVideoParameters(QJsonObject &videoData)
{
    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 6){
            QJsonArray arrayDouble = videoData[videoParameters.at(parameter)].toArray();
            QVector<double> pomDouble = arrayDouble2vector(arrayDouble);
            videoParametersDouble[videoParameters.at(parameter)].append(pomDouble);
        }
        if (parameter == 6){
            QJsonArray arrayInt = videoData[videoParameters.at(parameter)].toArray();
            QVector<int> pomInt = arrayInt2vector(arrayInt);
            videoParametersInt[videoParameters.at(parameter)].append(pomInt);
        }
        if (parameter > 6){
            int anomaly = videoData[videoParameters.at(parameter)].toInt();
            videoAnomalies[videoParameters.at(parameter)].push_back(anomaly);
        }
    }
}

bool SingleVideoLicovani::writeToVideo()
{
    QString kompletni_cesta = videoList.at(0);//chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",6);
        localErrorDialogHandling[ui->registratePB]->show();
        return false;
    }
    double sirka_framu = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska_framu = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Size velikostSnimku = cv::Size(int(sirka_framu),int(vyska_framu));
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    QString cestaZapis = SharedVariables::getSharedVariables()->getPath("ulozeniVidea")+"/"+chosenVideo[1]+"_GUI.avi";
    cv::VideoWriter writer;
    writer.open(cestaZapis.toLocal8Bit().constData(),static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)),cap.get(CV_CAP_PROP_FPS),velikostSnimku,true);
    if (!writer.isOpened())
    {
        localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",12);
        localErrorDialogHandling[ui->registratePB]->show();
        return false;
    }
    for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
        Mat posunutyOrig;
        cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
        if (cap.read(posunutyOrig)!=1)
        {
            QString errorMessage = QString(tr("Frame %1 could not be loaded from the video for registration. Process interrupted")).arg(indexImage);
            localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
            localErrorDialogHandling[ui->registratePB]->show();
            return false;
        }
        else if (videoParametersDouble["POCX"][0][indexImage] == 999.0){
            writer.write(posunutyOrig);
            posunutyOrig.release();
        }
        else{
            Point3d finalTranslation(0.0,0.0,0.0);
            cv::Mat plneSlicovany = cv::Mat::zeros(cv::Size(posunutyOrig.cols,posunutyOrig.rows), CV_32FC3);
            kontrola_typu_snimku_8C3(posunutyOrig);
            finalTranslation.x = videoParametersDouble["POCX"][0][indexImage];
            finalTranslation.y = videoParametersDouble["POCY"][0][indexImage];
            finalTranslation.z = 0.0;
            //qDebug()<<"For frame "<<indexImage<<" the translation is: "<<finalTranslation.x<<" "<<finalTranslation.y;
            plneSlicovany = translace_snimku(posunutyOrig,finalTranslation,posunutyOrig.rows,posunutyOrig.cols);
            plneSlicovany = rotace_snimku(plneSlicovany,videoParametersDouble["Uhel"][0][indexImage]);
            writer.write(plneSlicovany);
            posunutyOrig.release();
            plneSlicovany.release();
       }
    }
    return true;
}

void SingleVideoLicovani::on_savePB_clicked()
{

}

void SingleVideoLicovani::populateLists(QVector<QString> _file){
    videoList.append(_file[0]+"/"+_file[1]+"."+_file[2]);
    videoListNames.append(_file[1]);
}
