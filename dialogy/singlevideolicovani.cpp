#include "dialogy/singlevideolicovani.h"
#include "licovani/registrationthread.h"
#include "licovani/fazova_korelace_funkce.h"
#include "ui_singlevideolicovani.h"
//#include "hlavni_program/t_b_ho.h"
#include "util/licovaniparent.h"
#include "util/souborove_operace.h"
#include "util/prace_s_vektory.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/korelacni_koeficient.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
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

    videoParametersDouble["FrangiX"]=framesFrangiX;
    videoParametersDouble["FrangiX"]=framesFrangiY;
    videoParametersDouble["FrangiEuklid"]=framesFrangiEuklid;
    videoParametersDouble["POCX"]=framesPOCX;
    videoParametersDouble["POCY"]=framesPOCY;
    videoParametersDouble["Uhel"]=framesAngle;
    videoParametersInt["Ohodnoceni"]=framesFinalCompleteDecision;
    videoAnomalies["VerticalAnomaly"]=verticalAnomalyPresent;
    videoAnomalies["HorizontalAnomaly"]=horizontalAnomalyPresent;

    /*QIcon leftArrow(":/images/leftArrow.png");
    QIcon rightArrow(":/images/rightArrow.png");
    ui->leftArrowPushB->setIcon(leftArrow);
    ui->rightArrowPushB->setIcon(rightArrow);*/
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
    if (SharedVariables::getSharedVariables()->getPath("cestaKvideim") == "")
        ui->chooseVideoLE->setPlaceholderText(tr("Chosen video"));
    else
    {
        QString slozka,jmeno,koncovka;
        QStringList nalezenaVideaSlozka;
        int pocetNalezenych;
        analyseFileNames(SharedVariables::getSharedVariables()->getPath("cestaKvideim"),nalezenaVideaSlozka,pocetNalezenych,"avi");
        if (pocetNalezenych != 0)
        {
            fullVideoPath = SharedVariables::getSharedVariables()->getPath("cestaKvideim")+"/"+nalezenaVideaSlozka.at(0);
            videoList.append(fullVideoPath);
            processFilePath(fullVideoPath,slozka,jmeno,koncovka);
            videoListNames.append(jmeno);
            if (chosenVideo.length() == 0)
            {
                chosenVideo.push_back(slozka);
                chosenVideo.push_back(jmeno);
                chosenVideo.push_back(koncovka);
            }
            else
            {
                chosenVideo.clear();
                chosenVideo.push_back(slozka);
                chosenVideo.push_back(jmeno);
                chosenVideo.push_back(koncovka);
            }
            ui->chooseVideoLE->setText(jmeno);
        }
        QFile videoParametersFile(SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+chosenVideo[1]+".dat");
        videoParametersJson = readJson(videoParametersFile);
        processVideoParameters(videoParametersJson);
        ui->registratePB->setEnabled(true);
    }
}

void SingleVideoLicovani::on_chooseVideoLE_textChanged(const QString &arg1)
{
    QString kompletni_cesta = chosenVideo[0]+"/"+arg1+"."+chosenVideo[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chooseVideoLE->setStyleSheet("color: #FF0000");
    }
    else
    {
        ui->chooseVideoLE->setStyleSheet("color: #339900");
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
    cv::VideoCapture cap = cv::VideoCapture(videoList.at(0).toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        qWarning()<<"Unable to open "+videoList.at(0);
    }
    framesFinalCompleteDecision.append(videoParametersInt["Ohodnoceni"][0]);

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
    for (int indexThreshold = 0; indexThreshold < threadRange[0].length(); indexThreshold++){
        createAndRunThreads(videoCounter,cap,threadRange[0][indexThreshold],threadRange[1][indexThreshold]);
    }
}

void SingleVideoLicovani::createAndRunThreads(int indexProcVid, cv::VideoCapture &cap,
                                              int lowerLimit, int upperLimit)
{

    int cisloReference = findReferenceFrame(framesFinalCompleteDecision[indexProcVid]);
    cv::Mat referencniSnimek;
    cap.set(CV_CAP_PROP_POS_FRAMES,cisloReference);
    if (!cap.read(referencniSnimek))
        qWarning()<<"Frame "+QString::number(cisloReference)+" cannot be opened.";
    regThread = new RegistrationThread(cap,
                                       indexProcVid,
                                       videoListNames.at(indexProcVid),
                                       SharedVariables::getSharedVariables()->getFrangiParameters(),
                                       framesFinalCompleteDecision[0],
                                       referencniSnimek,
                                       lowerLimit,upperLimit,
                                       -1,10.0,0.1,
                                       videoAnomalies["VerticalAnomaly"][0],
                                       videoAnomalies["HorizontalAnomaly"][0],
                                       false);

    QObject::connect(regThread,SIGNAL(x_coordInfo(int,int,QTableWidgetItem*)),this,SLOT(addItem(int,int,QTableWidgetItem*)));
    QObject::connect(regThread,SIGNAL(y_coordInfo(int,int,QTableWidgetItem*)),this,SLOT(addItem(int,int,QTableWidgetItem*)));
    QObject::connect(regThread,SIGNAL(angleInfo(int,int,QTableWidgetItem*)),this,SLOT(addItem(int,int,QTableWidgetItem*)));
    QObject::connect(regThread,SIGNAL(statusInfo(int,int,QString)),this,SLOT(addStatus(int,int,QString)));
    QObject::connect(regThread,SIGNAL(allWorkDone(int)),this,SLOT(processAnother(int)));
    regThread->start();
}

void SingleVideoLicovani::totalFramesCompleted(int frameCounter)
{
    cv::VideoCapture cap = cv::VideoCapture(videoList.at(processedVideoNo).toLocal8Bit().constData());
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    ui->progress->setValue(qRound(double(frameCounter)/frameCount)*100);
}
void SingleVideoLicovani::addItem(int row,int column, QTableWidgetItem* item)
{
    ui->vysledkyLicovaniTW->setItem(row,column,item);

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
}
void SingleVideoLicovani::errorHandler(QString errorMessage){
    localErrorDialogHandling[ui->registratePB]->evaluate("left","hardError",errorMessage);
    localErrorDialogHandling[ui->registratePB]->show();
    regThread->quit();
}
void SingleVideoLicovani::processAnother(int processed){
    if (processed == videoCounter){
        threadProcessed++;
        if (threadProcessed == numberOfThreads){
            videoCounter++;
            if (videoCounter <= videoListNames.length()){
                video
            }
        }
    }
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

int SingleVideoLicovani::writeToVideo()
{
    QString kompletni_cesta = chosenVideo[0]+"/"+chosenVideo[1]+"."+chosenVideo[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    double sirka_framu = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska_framu = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    double FPSvidea = cap.get(CV_CAP_PROP_FPS);
    cv::Size velikostSnimku = cv::Size(int(sirka_framu),int(vyska_framu));
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    QString cestaZapis = SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni");+"/"+chosenVideo[1]+"_GUI.avi";
    cv::VideoWriter writer = cv::VideoWriter(cestaZapis.toLocal8Bit().constData(),
            CV_FOURCC('F','F','V','1'),FPSvidea,velikostSnimku,true);
    if (!writer.isOpened())
    {
        return 0;
    }
    for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
        Mat posunutyOrig,posunutyLicovani,posunutyLicovaniFinal;
        cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
        if (cap.read(posunutyOrig)!=1)
        {
            qWarning()<< "Image "<<indexImage<<" cannot be registrated.";
            continue;
        }
        else{
            Point3d finalTranslation;
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

/*void SingleVideoLicovani::licuj(cv::VideoCapture& cap,
                                QVector<double> frangiParam,
                                cv::Mat& referencni_snimek,
                                int startFrame,
                                int stopFrame,
                                int iterace,
                                double oblastMaxima,
                                double uhel,
                                float timeStamp,
                                float lightAnomaly)
{
    /// define helpers for frame preprocessing
    qDebug()<<"zahajeno licovani videa";
    bool lightAnomalyPresent = false;
    bool timeStampPresent = false;
    bool scaling=false;

    /// frame and video properties
    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    QVector<double> pomD(int(frameCount),0.0);
    frangiX = pomD;frangiY = pomD;frangiEuklidean = pomD;finalPOCx = pomD;finalPOCy = pomD;maximalAngles = pomD;
    if (lightAnomaly >0.0f && lightAnomaly < float(sirka))
    {
        ziskane_hranice_anomalie.x = lightAnomaly;
        ziskane_hranice_anomalie.y = 0;
        lightAnomalyPresent=true;
        scaling = true;
    }
    else
    {
        ziskane_hranice_anomalie.x = 0.0f;
        ziskane_hranice_anomalie.y = 0.0f;
    }
    if (timeStamp > 0.0f && timeStamp < float(vyska))
    {
        ziskane_hranice_CasZnac.y = timeStamp;
        ziskane_hranice_CasZnac.x = 0;
        timeStampPresent=true;
        scaling=true;
    }
    else
    {
        ziskane_hranice_CasZnac.y = 0.0f;
        ziskane_hranice_CasZnac.x = 0.0f;
    }
    Rect pom(0,0,0,0);
    correl_standard = pom;
    correl_extra = pom;
    anomalyCutoff = pom;
    Point3d frangiMaxDetected(0.0,0.0,0.0);
    Point3d pt_temp(0.0,0.0,0.0);
    Mat obraz,obraz_vyrez;
    imagePreprocessing(referencni_snimek,
                       obraz,
                       frangiParam,
                       frangiMaxDetected,
                       ziskane_hranice_anomalie,
                       ziskane_hranice_CasZnac,
                       anomalyCutoff,
                       correl_extra,
                       correl_standard,
                       cap,
                       scaling);
    qDebug()<<"preprocessing dokoncen";
    obraz(correl_standard).copyTo(obraz_vyrez);
    Point3d frangiMaxReversal = frangi_analysis(obraz,2,2,0,"",1,false,pt_temp,frangiParam);
    qDebug()<<"reverzni frangiho maximum "<<frangiMaxReversal.x<<" "<<frangiMaxReversal.y;
    for (int indexFrame = startFrame; indexFrame <= stopFrame; indexFrame++){        
        qDebug()<<"Zpracovavam snimek "<<indexFrame;
        if (snimkyOhodnoceniKomplet[0][indexFrame] == 0){
            qDebug()<<"Registruji jako nejvhodnejsiho";
            int iter = -1;int oblM = 10;double U = 0.1;
            registrateTheBest(cap,referencni_snimek,frangiMaxReversal,indexFrame,iter,oblM,
                              U,correl_extra,correl_standard,scaling,frangiParam);
        }
        if (snimkyOhodnoceniKomplet[0][indexFrame] == 2)
        {
            Mat posunuty_temp;
            cap.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (cap.read(posunuty_temp)!=1)
            {
                qWarning()<<"Snimek "<<indexFrame<<" nelze zapsat do videa!";
                continue;
            }
            else
            {
                if (scaling == true)
                {
                    Mat posunuty;
                    posunuty_temp(correl_extra).copyTo(posunuty);
                    Point3d frangi_bod_obraz_reverse = frangi_analysis(posunuty,2,2,0,"",1,timeStampPresent,pt_temp,frangiParam);
                    frangiX[indexFrame] = frangi_bod_obraz_reverse.x;
                    frangiY[indexFrame] = frangi_bod_obraz_reverse.y;
                    frangiEuklidean[indexFrame] = 0;
                    qDebug() << "Referencni snimek "<<indexFrame<<" zapsan.";
                    posunuty_temp.release();
                    posunuty_temp.release();
                }
                else
                {
                    Point3d frangi_bod_obraz_reverse = frangi_analysis(posunuty_temp,2,2,0,"",1,timeStampPresent,pt_temp,frangiParam);
                    qDebug() << "Referencni snimek "<<indexFrame<<" zapsan.";
                    posunuty_temp.release();
                    frangiX[indexFrame] = frangi_bod_obraz_reverse.x;
                    frangiY[indexFrame] = frangi_bod_obraz_reverse.y;
                    frangiEuklidean[indexFrame] = 0;
                }
                maximalAngles[indexFrame] = 0.0;
                finalPOCx[indexFrame] = 0.0;
                finalPOCy[indexFrame] = 0.0;
            }
        }
        if (snimkyOhodnoceniKomplet[0][indexFrame] == 5)
        {
            Mat posunuty;
            cap.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (cap.read(posunuty)!=1)
            {
                qWarning()<<"Snimek "<<indexFrame<<" nelze zapsat do videa!";
                continue;
            }
            else
            {
                qDebug() << "Snimek "<<indexFrame<<" zapsan beze zmen.";
                posunuty.release();
                frangiX[indexFrame] = 999.0;
                frangiY[indexFrame] = 999.0;
                frangiEuklidean[indexFrame] = 999.0;
                maximalAngles[indexFrame] = 999;
                finalPOCx[indexFrame] = 999.0;
                finalPOCy[indexFrame] = 999.0;
            }
        }
        if (snimkyOhodnoceniKomplet[0][indexFrame] == 1 || snimkyOhodnoceniKomplet[0][indexFrame] == 4)
        {
            Mat slicovany_snimek;
            Point3d mira_translace(0.0,0.0,0.0);
            double celkovy_uhel=0.0;
            int uspech_licovani = fullRegistration(cap,
                                                   referencni_snimek,
                                                   indexFrame,
                                                   iterace,
                                                   oblastMaxima,
                                                   uhel,
                                                   correl_extra,
                                                   correl_standard,
                                                   scaling,
                                                   slicovany_snimek,
                                                   mira_translace,
                                                   celkovy_uhel);
            Mat posunuty_temp;
            cap.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (cap.read(posunuty_temp)!=1)
            {
                qWarning()<<"Snimek "<<indexFrame<<" nelze zapsat do videa!";
                mira_translace.x = 0;
                mira_translace.y = 0;
                mira_translace.z = 0;
                celkovy_uhel = 0;
                continue;
            }
            if (uspech_licovani == 0)
            {
                qDebug()<< "Licovani snimku "<<indexFrame<< " skoncilo chybou. Zapsan puvodni snimek.";
                posunuty_temp.release();
                mira_translace.x = 0;
                mira_translace.y = 0;
                mira_translace.z = 0;
                celkovy_uhel = 0;
            }
            else
            {
                Point3d korekce_translace(0,0,0);
                cv::Mat plne_slicovany_snimek;
                int uspechKorekce = registrationCorrection(slicovany_snimek,obraz,plne_slicovany_snimek,
                                                                         correl_standard,
                                                                         korekce_translace);

                if (korekce_translace.x != 0.0)
                {
                    mira_translace.x+=korekce_translace.x;
                    mira_translace.y+=korekce_translace.y;
                    Point3d pt6 = fk_translace_hann(obraz,plne_slicovany_snimek);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = fk_translace(obraz,plne_slicovany_snimek);
                    }
                    qDebug() << "kontrola Y: " << pt6.y <<" X: "<<pt6.x;
                }
                finalPOCx[indexFrame] = mira_translace.x;
                finalPOCy[indexFrame] = mira_translace.y;
                maximalAngles[indexFrame] = celkovy_uhel;

                qDebug() << "Snimek "<<indexFrame<<" zapsan po standardnim slicovani.";
                qDebug()<<" Mira translace: "<<mira_translace.x<<" "<<mira_translace.y<<" "<<celkovy_uhel<<endl<<endl;
                if (scaling == true)
                {
                    int radky = posunuty_temp.rows;
                    int sloupce = posunuty_temp.cols;
                    Mat posunuty_original = translace_snimku(posunuty_temp,mira_translace,radky,sloupce);
                    Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                    plne_slicovany_snimek.release();
                    finalni_licovani2.release();
                    posunuty_original.release();
                    posunuty_temp.release();
                }
                else
                {
                    plne_slicovany_snimek.release();
                    posunuty_temp.release();
                }
                mira_translace.x = 0;
                mira_translace.y = 0;
                mira_translace.z = 0;
                celkovy_uhel = 0;
            }
        }
    }
}

int SingleVideoLicovani::registrateTheBest(cv::VideoCapture& cap,
                                           cv::Mat& referencni_snimek,
                                           cv::Point3d bod_RefS_reverse,
                                           int index_posunuty,
                                           int iterace,
                                           double oblastMaxima,
                                           double uhel,
                                           cv::Rect& vyrez_korelace_extra,
                                           cv::Rect& vyrez_korelace_standard,
                                           bool zmena_meritka,
                                           QVector<double> &parametry_frangi)
{
    qDebug()<<"registrace nejvhodnějšího začala";
    Mat plne_slicovany_snimek = cv::Mat::zeros(cv::Size(referencni_snimek.cols,referencni_snimek.rows), CV_32FC3);
    Point3d mira_translace(0.0,0.0,0.0);
    double celkovy_uhel = 0.0;
    int uspech_licovani_nejlepsich = 0;
    int uspech_licovani_komplet = fullRegistration(cap,
                                           referencni_snimek,
                                           index_posunuty,
                                           iterace,
                                           oblastMaxima,
                                           uhel,
                                           vyrez_korelace_extra,
                                           vyrez_korelace_standard,
                                           zmena_meritka,
                                           plne_slicovany_snimek,
                                           mira_translace,
                                           celkovy_uhel);
    qDebug()<<"kompletně slícován";
    Mat refSnimek_vyrez;
    referencni_snimek(vyrez_korelace_standard).copyTo(refSnimek_vyrez);
    int rows = referencni_snimek.rows;
    int cols = referencni_snimek.cols;
    if (uspech_licovani_komplet == 0)
    {
        qWarning()<< "Licovani skoncilo chybou. Snimek "<<index_posunuty<<" se nepodarilo uspesne slicovat";
        Mat posunuty;
        cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
        if (cap.read(posunuty)!=1)
        {
             qWarning()<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!";
        }
        else
        {
            qDebug()<< "Snimek "<<index_posunuty<<" zapsan beze zmen. Licovani skoncilo chybou.";
            posunuty.release();
            plne_slicovany_snimek.release();
        }
        finalPOCx[index_posunuty]=999.0;
        finalPOCy[index_posunuty]=999.0;
        maximalAngles[index_posunuty]=999.0;
        frangiX[index_posunuty]=999.0;
        frangiY[index_posunuty]=999.0;
        frangiEuklidean[index_posunuty]=999.0;
        return uspech_licovani_nejlepsich;
    }
    else
    {
        if (std::abs(mira_translace.x) >= 55 || std::abs(mira_translace.y) >= 55)
        {
            Mat posunuty;
            cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
            if (cap.read(posunuty)!=1)
            {
                 qWarning()<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!";
            }
            else
            {
                qDebug()<< "Snimek "<<index_posunuty<<" zapsan beze zmen. Hodnota translace prekrocila prah.";
                posunuty.release();
                plne_slicovany_snimek.release();
            }
            finalPOCx[index_posunuty]=999.0;
            finalPOCy[index_posunuty]=999.0;
            maximalAngles[index_posunuty]=999.0;
            frangiX[index_posunuty]=999.0;
            frangiY[index_posunuty]=999.0;
            frangiEuklidean[index_posunuty]=999.0;
            return uspech_licovani_nejlepsich;
        }
        else
        {
            Mat posunuty_temp;
            cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
            if (cap.read(posunuty_temp)!=1)
            {
                 qWarning()<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!";
            }
            Mat mezivysledek32f,mezivysledek32f_vyrez,posunuty;
            if (zmena_meritka == true)
            {
                posunuty_temp(vyrez_korelace_extra).copyTo(posunuty);
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
            }
            Point3d translace_korekce(0.0,0.0,0.0);
            Mat plneSlicovanyKorekce = cv::Mat::zeros(cv::Size(referencni_snimek.cols,referencni_snimek.rows), CV_32FC3);
            int uspechKorekce = registrationCorrection(plne_slicovany_snimek,referencni_snimek,plneSlicovanyKorekce,
                                                              vyrez_korelace_standard,translace_korekce);
            if (uspechKorekce == 0)
                return uspech_licovani_nejlepsich;
            else{
                if (translace_korekce.x != 0.0)
                {
                    mira_translace.x+=translace_korekce.x;
                    mira_translace.y+=translace_korekce.y;
                    Point3d pt6 = fk_translace_hann(referencni_snimek,plneSlicovanyKorekce);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = fk_translace(referencni_snimek,plneSlicovanyKorekce);
                    }
                    qDebug() << "kontrola Y: " << pt6.y <<" X: "<<pt6.x;
                    //fPOCx[index_posunuty] = mira_translace.x;
                    //fPOCy[index_posunuty] = mira_translace.y;
                }
                plneSlicovanyKorekce.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
                double R_prvni = vypocet_KK(referencni_snimek,plneSlicovanyKorekce,vyrez_korelace_standard);
                Point3d frangi_bod_slicovany_reverse = frangi_analysis(plneSlicovanyKorekce,2,2,0,"",2,false,mira_translace,parametry_frangi);
                frangiX[index_posunuty] = frangi_bod_slicovany_reverse.x;
                frangiY[index_posunuty] = frangi_bod_slicovany_reverse.y;
                double yydef = bod_RefS_reverse.x - frangi_bod_slicovany_reverse.x;
                double xxdef = bod_RefS_reverse.y - frangi_bod_slicovany_reverse.y;
                double suma_rozdilu = std::pow(xxdef,2.0) + std::pow(yydef,2.0);
                double euklid = std::sqrt(suma_rozdilu);
                frangiEuklidean[index_posunuty] = euklid;
                xxdef = 0;
                yydef = 0;
                suma_rozdilu = 0;
                //cout << yydef << " " << xxdef << endl;
                Point3d vysledne_posunuti(0.0,0.0,0.0);
                vysledne_posunuti.y = mira_translace.y - yydef;
                vysledne_posunuti.x = mira_translace.x - xxdef;
                vysledne_posunuti.z = 0;
                //cout << vysledne_posunuti.y << " " << vysledne_posunuti.x << endl;//pt3.x <<endl;
                Mat posunuty2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
                Mat finalni_licovani = rotace_snimku(posunuty2,celkovy_uhel);
                Mat finalni_licovani_32f,finalni_licovani_32f_vyrez;
                finalni_licovani.copyTo(finalni_licovani_32f);
                kontrola_typu_snimku_32C1(finalni_licovani_32f);
                finalni_licovani_32f(vyrez_korelace_standard).copyTo(finalni_licovani_32f_vyrez);
                finalni_licovani_32f.release();
                posunuty2.release();
                double R_druhy = vypocet_KK(referencni_snimek,finalni_licovani,vyrez_korelace_standard);
                finalni_licovani_32f_vyrez.release();
                if (R_prvni >= R_druhy)
                {
                    qDebug()<< "Snimek "<<index_posunuty<<" zapsan pouze po standardnim slicovani.";
                    qDebug()<<" Mira translace: "<<mira_translace.x<<" "<<mira_translace.y<<" "<<celkovy_uhel;
                    if (zmena_meritka == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,mira_translace,radky,sloupce);
                        Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                        plne_slicovany_snimek.release();
                        plneSlicovanyKorekce.release();
                        finalni_licovani2.release();
                        finalni_licovani.release();
                        posunuty_original.release();
                        posunuty_temp.release();
                    }
                    else
                    {
                        plne_slicovany_snimek.release();
                        finalni_licovani.release();
                    }
                    finalPOCx[index_posunuty] = mira_translace.x;
                    finalPOCy[index_posunuty] = mira_translace.y;
                }
                else
                {
                    qDebug()<< "Snimek "<<index_posunuty<<" zapsan po uprave translace skrze cevy.";
                    qDebug()<<" Mira translace: "<<vysledne_posunuti.x<<" "<<vysledne_posunuti.y<<" "<<celkovy_uhel;
                    if (zmena_meritka == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,vysledne_posunuti,radky,sloupce);
                        Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                        plne_slicovany_snimek.release();
                        plneSlicovanyKorekce.release();
                        finalni_licovani2.release();
                        finalni_licovani.release();
                        posunuty_original.release();
                        posunuty_temp.release();
                    }
                    else
                    {
                        plne_slicovany_snimek.release();
                        finalni_licovani.release();
                    }
                    finalPOCx[index_posunuty] = vysledne_posunuti.x;
                    finalPOCy[index_posunuty] = vysledne_posunuti.y;
                }
                euklid = 0;
                mezivysledek32f.release();
                mezivysledek32f_vyrez.release();
                return uspech_licovani_nejlepsich = 1;
            }
        }
    }
}
int SingleVideoLicovani::fullRegistration(cv::VideoCapture& cap,
                     cv::Mat& referencni_snimek,
                     int cislo_posunuty,
                     int iterace,
                     double oblastMaxima,
                     double uhel,
                     cv::Rect& korelacni_vyrez_navic,
                     cv::Rect& korelacni_vyrez_standardni,
                     bool nutnost_zmenit_velikost_snimku,
                     cv::Mat& slicovany_kompletne,
                     cv::Point3d& mira_translace,
                     double& celkovy_uhel)
{
    qDebug()<<"Kompletní registrace začala";
    Mat posunuty_temp;//,,posunuty_32f;
    int uspech_licovani;
    cap.set(CV_CAP_PROP_POS_FRAMES,cislo_posunuty);

    if(!cap.read(posunuty_temp))
        return uspech_licovani = 0;

    kontrola_typu_snimku_8C3(referencni_snimek);
    kontrola_typu_snimku_8C3(posunuty_temp);
    int rows = referencni_snimek.rows;
    int cols = referencni_snimek.cols;
    Mat hann;
    createHanningWindow(hann, referencni_snimek.size(), CV_32FC1);
    Mat referencni_snimek_32f,referencni_snimek_vyrez;
    referencni_snimek.copyTo(referencni_snimek_32f);
    kontrola_typu_snimku_32C1(referencni_snimek_32f);
    referencni_snimek_32f(korelacni_vyrez_standardni).copyTo(referencni_snimek_vyrez);
    Mat posunuty, posunuty_vyrez;
    if (nutnost_zmenit_velikost_snimku == true)
    {
        posunuty_temp(korelacni_vyrez_navic).copyTo(posunuty);
        posunuty(korelacni_vyrez_standardni).copyTo(posunuty_vyrez);
        posunuty_temp.release();
    }
    else
    {
        posunuty_temp.copyTo(posunuty);
        posunuty(korelacni_vyrez_standardni).copyTo(posunuty_vyrez);
        posunuty_temp.release();
    }
    Mat posunuty_32f;
    posunuty.copyTo(posunuty_32f);
    kontrola_typu_snimku_32C1(posunuty_32f);

    Mat slicovany1;
    Point3d pt1(0.0,0.0,0.0);
    if (nutnost_zmenit_velikost_snimku == true)
    {
        pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        //qDebug() << "PT1 pri zmene velikosti Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
            pt1 = fk_translace(referencni_snimek_32f,posunuty_32f);

        if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
            pt1 = fk_translace(referencni_snimek_vyrez,posunuty_vyrez);
    }
    if (nutnost_zmenit_velikost_snimku == false)
    {
        pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        //qDebug() << "PT1 bez zmeny velikosti Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
    }
    qDebug()<<"Prvni licovani";
    if (pt1.x>=55 || pt1.y>=55)
    {
        mira_translace = pt1;
        finalPOCx[cislo_posunuty] = pt1.x;
        finalPOCy[cislo_posunuty] = pt1.y;
        maximalAngles[cislo_posunuty] = 0.0;
        //qDebug() << "PT1 nad 55 - Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        return uspech_licovani = 0;
    }
    else
    {
        qDebug() << "PT1 v normálu Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        slicovany1 = translace_snimku(posunuty,pt1,rows,cols);
        cv::Mat slicovany1_32f_rotace,slicovany1_32f,slicovany1_vyrez;
        slicovany1.copyTo(slicovany1_32f);
        kontrola_typu_snimku_32C1(slicovany1_32f);
        Point3d vysledek_rotace = fk_rotace(referencni_snimek_32f,slicovany1_32f,uhel,pt1.z,pt1);
        if (std::abs(vysledek_rotace.y) > uhel)
            vysledek_rotace.y=0;

        slicovany1_32f_rotace = rotace_snimku(slicovany1_32f,vysledek_rotace.y);
        slicovany1_32f_rotace(korelacni_vyrez_standardni).copyTo(slicovany1_vyrez);
        Point3d pt2 = fk_translace(referencni_snimek_vyrez,slicovany1_vyrez);
        if (pt2.x >= 55 || pt2.y >= 55)
        {
            mira_translace = pt1;
            finalPOCx[cislo_posunuty] = pt1.x;
            finalPOCy[cislo_posunuty] = pt1.y;
            maximalAngles[cislo_posunuty] = 0.0;
            slicovany1.copyTo(slicovany_kompletne);
            slicovany1.release();
            return uspech_licovani = 0;
        }
        else
        {
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt2.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            qDebug()<<"FWHM: "<<FWHM;
            slicovany1.release();
            slicovany1_32f.release();
            slicovany1_vyrez.release();
            Point3d pt3;
            pt3.x = pt1.x+pt2.x;
            pt3.y = pt1.y+pt2.y;
            pt3.z = pt2.z;
            Mat slicovany2 = translace_snimku(posunuty,pt3,rows,cols);
            qDebug()<<"Translace";
            Mat slicovany2_32f;//,slicovany2_vyrez;
            slicovany2.copyTo(slicovany2_32f);
            kontrola_typu_snimku_32C1(slicovany2_32f);
            Mat slicovany2_rotace = rotace_snimku(slicovany2_32f,vysledek_rotace.y);
            qDebug()<<"rotace";
            //slicovany2_rotace(korelacni_vyrez_standardni).copyTo(slicovany2_vyrez);
            Mat mezivysledek_vyrez,mezivysledek;
            slicovany2_rotace.copyTo(mezivysledek);
            //slicovany2_vyrez.copyTo(mezivysledek_vyrez);
            //slicovany2.release();
            //slicovany2_vyrez.release();
            slicovany2_32f.release();
            slicovany2_rotace.release();
            celkovy_uhel+=vysledek_rotace.y;
            vysledek_rotace.y = 0;
            int max_pocet_iteraci = 0;
            if (iterace == -1)
            {
                if (FWHM <= 20){max_pocet_iteraci = 2;}
                else if (FWHM > 20 && FWHM <= 30){max_pocet_iteraci = 4;}
                else if (FWHM > 30 && FWHM <= 35){max_pocet_iteraci = 6;}
                else if (FWHM > 35 && FWHM <= 40){max_pocet_iteraci = 8;}
                else if (FWHM > 40 && FWHM <= 45){max_pocet_iteraci = 10;}
                else if (FWHM > 45){max_pocet_iteraci = 5;};
            }
            if (iterace >= 1)
            {
                max_pocet_iteraci = iterace;
            }
            qDebug()<<"Iterativni licovani";
            for (int i = 0; i < max_pocet_iteraci; i++)
            {
                Point3d rotace_ForLoop = fk_rotace(referencni_snimek,mezivysledek,uhel,pt3.z,pt3);
                if (std::abs(rotace_ForLoop.y) > uhel)
                    rotace_ForLoop.y = 0.0;
                else if (std::abs(celkovy_uhel+rotace_ForLoop.y)>uhel)
                    rotace_ForLoop.y=0.0;
                else
                    celkovy_uhel+=rotace_ForLoop.y;

                qDebug() << "rotace " << celkovy_uhel <<" se zmenou " << rotace_ForLoop.y << endl;
                Mat rotovany;
                if (rotace_ForLoop.y != 0.0)
                    rotovany = rotace_snimku(mezivysledek,rotace_ForLoop.y);
                else
                    rotovany = mezivysledek;

                rotace_ForLoop.y = 0.0;
                Mat rotovany_vyrez;
                rotovany(korelacni_vyrez_standardni).copyTo(rotovany_vyrez);
                rotovany.release();
                Point3d pt4 = fk_translace(referencni_snimek_vyrez,rotovany_vyrez);
                rotovany_vyrez.release();
                if (pt4.x >= 55 || pt4.y >= 55)
                {
                    mira_translace = pt3;
                    slicovany2.copyTo(slicovany_kompletne);
                    finalPOCx[cislo_posunuty] = pt3.x;
                    finalPOCy[cislo_posunuty] = pt3.y;
                    return uspech_licovani = 1;
                }
                else
                {
                    //qDebug() << "Y: "<< (pt4.y) <<" a X: "<<(pt4.x)<<endl;
                    pt3.x += pt4.x;
                    pt3.y += pt4.y;
                    pt3.z = pt4.z;
                    qDebug() << "Y: "<< (pt3.y) <<" a X: "<<(pt3.x)<<endl<<endl;
                    Mat posunuty_temp = translace_snimku(posunuty,pt3,rows,cols);
                    Mat rotovany_temp = rotace_snimku(posunuty_temp,celkovy_uhel);
                    posunuty_temp.release();
                    rotovany_temp.copyTo(mezivysledek);
                    //Mat rotovany_temp32f_vyrez;
                    kontrola_typu_snimku_32C1(rotovany_temp);
                    //rotovany_temp(korelacni_vyrez_standardni).copyTo(rotovany_temp32f_vyrez);
                    //rotovany_temp32f_vyrez.copyTo(mezivysledek_vyrez);
                    rotovany_temp.release();
                    //rotovany_temp32f_vyrez.release();
                }
            }
            mira_translace = pt3;
            mezivysledek.copyTo(slicovany_kompletne);
            finalPOCx[cislo_posunuty] = pt3.x;
            finalPOCy[cislo_posunuty] = pt3.y;
            return uspech_licovani = 1;
        }
    }
}
int SingleVideoLicovani::imagePreprocessing(cv::Mat &reference,
                       cv::Mat &obraz,
                       QVector<double> &parFrang,
                       cv::Point3d& frangiMaxEstimated,
                       cv::Point2f &hraniceAnomalie,
                       cv::Point2f &hraniceCasu,
                       cv::Rect &oblastAnomalie,
                       cv::Rect &vyrezKoreEx,
                       cv::Rect &vyrezKoreStand,
                       cv::VideoCapture &cap,
                       bool &zmeMer)
{
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Point3d frangi_bod(0.0,0.0,0.0);
    double sirka_framu = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska_framu = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    bool pritomnostAnomalie = false;
    bool pritomnostCasZn = false;
    if (hraniceAnomalie.x != 0.0f) // světelná anomálie
    {
        if (hraniceAnomalie.x < float(sirka_framu/2))
        {
            oblastAnomalie.x = 0;
            oblastAnomalie.y = int(hraniceAnomalie.x);
            oblastAnomalie.width = int(sirka_framu-int(hraniceAnomalie.x)-1);
            oblastAnomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
        }
        if (hraniceAnomalie.x > float(sirka_framu/2))
        {
            oblastAnomalie.x = 0;
            oblastAnomalie.y = 0;
            oblastAnomalie.width = int(hraniceAnomalie.x);
            oblastAnomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
        }
        pritomnostAnomalie = true;
    }
    if (hraniceCasu.y != 0.0f) // časová anomálie
    {
        if (hraniceCasu.x < float(vyska_framu/2))
        {
            oblastAnomalie.x = int(hraniceCasu.y);
            oblastAnomalie.y = 0;
            if (pritomnostAnomalie != true)
                oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
            oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
        }
        if (hraniceCasu.x > float(sirka_framu/2))
        {
            oblastAnomalie.x = 0;
            oblastAnomalie.y = 0;
            if (pritomnostAnomalie != true)
                oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
            oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
        }
        pritomnostCasZn = true;
    }
    if (pritomnostAnomalie == true || pritomnostCasZn == true)
        frangi_bod = frangi_analysis(reference(oblastAnomalie),1,1,0,"",1,pritomnostCasZn,pt_temp,parFrang);
    else
        frangi_bod = frangi_analysis(reference,1,1,0,"",1,pritomnostCasZn,pt_temp,parFrang);

    if (frangi_bod.z == 0.0)
    {
        qDebug()<<"Nalezeni maxima frangiho funkce se nezdarilo, proverte snimek!"<<endl;
    }
    else
    {
        qDebug()<<"Nalezen Frangi max v referenci:"<<frangi_bod.x<<" "<<frangi_bod.y;
        bool nutnost_zmenit_velikost = false;
        int rows = reference.rows;
        int cols = reference.cols;
        int radek_od = int(round(frangi_bod.y-0.9*frangi_bod.y));
        int radek_do = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
        int sloupec_od = 0;
        int sloupec_do = 0;

        if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0f && int(hraniceAnomalie.y)<(cols/2))
        {
            sloupec_od = int(hraniceAnomalie.y);
            nutnost_zmenit_velikost = true;
        }
        else
            sloupec_od = int(round(frangi_bod.x-0.9*(frangi_bod.x)));

        if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0f &&  int(hraniceAnomalie.y)>(cols/2))
        {
            sloupec_do = int(hraniceAnomalie.y);
            nutnost_zmenit_velikost = true;
        }
        else
            sloupec_do = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));

        int vyrez_sirka = sloupec_do-sloupec_od;
        int vyrez_vyska = radek_do - radek_od;

        if ((vyrez_vyska>480 || vyrez_sirka>640)|| nutnost_zmenit_velikost == true)
        {
            vyrezKoreEx.x = sloupec_od;
            vyrezKoreEx.y = radek_od;
            vyrezKoreEx.width = vyrez_sirka;
            vyrezKoreEx.height = vyrez_vyska;
            reference(vyrezKoreEx).copyTo(obraz);
            frangi_bod = frangi_analysis(obraz,1,1,0,"",1,false,pt_temp,parFrang);
            rows = obraz.rows;
            cols = obraz.cols;
            radek_od = int(round(frangi_bod.y-0.9*frangi_bod.y));
            radek_do = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
            sloupec_od = int(round(frangi_bod.x-0.9*(frangi_bod.x)));
            sloupec_do = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));
            vyrez_sirka = sloupec_do-sloupec_od;
            vyrez_vyska = radek_do - radek_od;
            vyrezKoreStand.x = sloupec_od;
            vyrezKoreStand.y = radek_od;
            vyrezKoreStand.width = vyrez_sirka;
            vyrezKoreStand.height = vyrez_vyska;
            zmeMer = true;
            frangiMaxEstimated = frangi_bod;
        }
        else
        {
            qDebug()<<"vytvarim standardni korelacni vyrez";
            vyrezKoreStand.x = int(round(frangi_bod.x-0.9*(frangi_bod.x)));
            vyrezKoreStand.y = int(round(frangi_bod.y-0.9*frangi_bod.y));
            radek_do = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
            sloupec_do = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));
            vyrezKoreStand.width = sloupec_do-vyrezKoreStand.x;
            vyrezKoreStand.height = radek_do - vyrezKoreStand.y;
            reference.copyTo(obraz);
            frangiMaxEstimated = frangi_bod;
            //obraz_temp.release();
        }
    }
}
int SingleVideoLicovani::registrationCorrection(cv::Mat& slicovany_snimek,
                           cv::Mat& obraz,
                           cv::Mat& snimek_korigovany,
                           cv::Rect& vyrez_korelace_standard,
                           cv::Point3d& korekce_bod)
{
    int uspechKorekce = 0;
    Mat mezivysledek,mezivysledek32f,mezivysledek32f_vyrez,obraz_vyrez;
    slicovany_snimek.copyTo(mezivysledek);
    int rows = slicovany_snimek.rows;
    int cols = slicovany_snimek.cols;
    mezivysledek.copyTo(mezivysledek32f);
    kontrola_typu_snimku_32C1(mezivysledek32f);
    mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
    obraz(vyrez_korelace_standard).copyTo(obraz_vyrez);
    double R1 = vypocet_KK(obraz,mezivysledek,vyrez_korelace_standard);
    qDebug() << "Korelace 1: "<<R1;
    mezivysledek32f.release();
    mezivysledek32f_vyrez.release();
    korekce_bod = fk_translace_hann(obraz,mezivysledek);
    Point3d mira_korekcniho_posunuti = fk_translace_hann(obraz,mezivysledek);
    if (std::abs(mira_korekcniho_posunuti.x) > 290 || std::abs(mira_korekcniho_posunuti.y) > 290)
    {
        mira_korekcniho_posunuti = fk_translace(obraz,mezivysledek);
    }
    if (std::abs(mira_korekcniho_posunuti.x) > 290 || std::abs(mira_korekcniho_posunuti.y) > 290)
    {
        mira_korekcniho_posunuti = fk_translace(obraz_vyrez,mezivysledek32f_vyrez);
    }
    qDebug() << "korekce Y: " << korekce_bod.y <<" X: "<<korekce_bod.x;
    snimek_korigovany = translace_snimku(mezivysledek,korekce_bod,rows,cols);
    double R2 = vypocet_KK(obraz,snimek_korigovany,vyrez_korelace_standard);
    qDebug() << "Korelace 2: "<<R2;
    //double rozdil = R2-R1;
    if ((R2 > R1) && ((std::abs(mira_korekcniho_posunuti.x) > 0.3) || (std::abs(mira_korekcniho_posunuti.y) > 0.3)))
    {
        qDebug() << "snimek zkorigovan.";
        korekce_bod = mira_korekcniho_posunuti;
        return uspechKorekce = 1;
    }
    else
    {
        qDebug() << "snimek nezkorigovan.";
        return uspechKorekce = 1;
    }
}
*/

/*void SingleVideoLicovani::on_slozkaParametruLE_textChanged(const QString &arg1)
{

}

void SingleVideoLicovani::on_slozkaParametruPB_clicked()
{
    QString kompletnicesta = QFileDialog::getOpenFileName(this,
                                                          "Vyberte soubor s parametry", videaKanalyzeAktual,"*.avi;;Všechny soubory (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    processFilePath(kompletnicesta,vybrana_slozka,vybrany_soubor,koncovka);
    QString dir = TXTnacteniAktual;
    QDir chosenDirectory(dir);
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.json" << "*.JSON",QDir::Files);

    ui->slozkaParametruLE->setStyleSheet("color: #339900");
    if (vybranyJSONLicovaniSingle.length() == 0)
    {
        vybranyJSONLicovaniSingle.push_back(vybrana_slozka);
        vybranyJSONLicovaniSingle.push_back(vybrany_soubor);
        vybranyJSONLicovaniSingle.push_back(koncovka);
    }
    else
    {
        vybranyJSONLicovaniSingle.clear();
        vybranyJSONLicovaniSingle.push_back(vybrana_slozka);
        vybranyJSONLicovaniSingle.push_back(vybrany_soubor);
        vybranyJSONLicovaniSingle.push_back(koncovka);
    }
    ui->chooseVideoLE->setText(vybranyJSONLicovaniSingle[1]);


}

void SingleVideoLicovani::on_registratePB_clicked()
{

}

void SingleVideoLicovani::on_showResultsPB_clicked()
{

}

void SingleVideoLicovani::on_grafickeHodnoceniPB_clicked()
{

}

void SingleVideoLicovani::on_savePB_clicked()
{

}

void SingleVideoLicovani::on_FF_PB_clicked()
{

}

void SingleVideoLicovani::on_parametryFF_LE_textChanged(const QString &arg1)
{

}*/
