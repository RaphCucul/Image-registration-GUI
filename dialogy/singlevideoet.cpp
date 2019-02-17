#include "dialogy/singlevideoet.h"
#include "ui_singlevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "hlavni_program/frangi_detektor.h"
#include "dialogy/grafet.h"
#include "dialogy/clickimageevent.h"
#include "multithreadET/qThreadFirstPart.h"
#include "util/prace_s_vektory.h"
#include "util/souborove_operace.h"
#include "licovani/rozhodovaci_algoritmy.h"
#include "licovani/multiPOC_Ai1.h"
#include "fancy_staff/sharedvariables.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <limits>

#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <QLabel>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

SingleVideoET::SingleVideoET(QWidget *parent) :
    ETanalysisParent (parent),
    ui(new Ui::SingleVideoET)
{
    ui->setupUi(this);
    ui->horizontalAnomalyCB->setEnabled(false);
    ui->verticalAnomalyCB->setEnabled(false);
    ui->calculateET->setEnabled(false);
    ui->showGraphET->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->areaMaximum->setPlaceholderText("0 - 20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationCount->setPlaceholderText("1 - Inf; -1~automatic settings");

    ui->chooseVideoPB->setText(tr("Choose video"));
    ui->referencialNumberLE->setPlaceholderText(tr("Write number of referencial frame"));
    ui->horizontalAnomalyCB->setText(tr("Top/bottom anomaly"));
    ui->verticalAnomalyCB->setText(tr("Left/right anomaly"));
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerancelabel->setText(tr("Maximal tolerated rotation angle"));
    ui->iterationNumberlabel->setText(tr("Number of iterations of algorithm"));
    ui->calculateET->setText(tr("Compute entropy and tennengrade"));
    ui->showGraphET->setText(tr("Show computed results"));
    ui->savePB->setText(tr("Save results"));

    mapDouble["entropie"]=entropy;
    mapDouble["tennengrad"]=tennengrad;
    mapDouble["FrangiX"]=framesFrangiX;
    mapDouble["FrangiY"]=framesFrangiY;
    mapDouble["FrangiEuklid"]=framesFrangiEuklid;
    mapDouble["POCX"]=framesPOCX;
    mapDouble["POCY"]=framesPOCY;
    mapDouble["Uhel"]=framesAngle;
    mapInt["Ohodnoceni"]=framesFinalCompleteDecision;
    mapInt["PrvotOhodEntropie"]=framesFirstFullCompleteEntropyEvaluation;
    mapInt["PrvotOhodTennengrad"]=framesFirstFullCompleteTennengradEvaluation;
    mapInt["PrvniRozhod"]=framesFirstFullCompleteDecision;
    mapInt["DruheRozhod"]=framesSecondFullCompleteDecision;
    mapAnomalies["VerticalAnomaly"]=horizontalAnomalyPresent;
    mapAnomalies["HorizontalAnomaly"]=verticalAnomalyPresent;

    connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    connect(ui->verticalAnomalyCB,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    connect(ui->horizontalAnomalyCB,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
}

SingleVideoET::~SingleVideoET()
{
    delete ui;
}

void SingleVideoET::checkPaths(){
    if (SharedVariables::getSharedVariables()->getPath("cestaKvideim") == "")
        ui->chosenVideoLE->setPlaceholderText(tr("Chosen video"));
    else{
       analyseAndSaveFirst(SharedVariables::getSharedVariables()->getPath("cestaKvideim"),chosenVideoETSingle);
    }
}
void SingleVideoET::on_chooseVideoPB_clicked()
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         "Choose referencial image", SharedVariables::getSharedVariables()->getPath("cestaKvideim"),"*.avi;;All files (*)");
    QString folder,filename,suffix;
    processFilePath(referencialImagePath,folder,filename,suffix);
    if (chosenVideoETSingle.length() == 0)
    {
        chosenVideoETSingle.push_back(folder);
        chosenVideoETSingle.push_back(filename);
        chosenVideoETSingle.push_back(suffix);
    }
    else
    {
        chosenVideoETSingle.clear();
        chosenVideoETSingle.push_back(folder);
        chosenVideoETSingle.push_back(filename);
        chosenVideoETSingle.push_back(suffix);
    }
    ui->chosenVideoLE->setText(chosenVideoETSingle[1]);
    cv::VideoCapture cap = cv::VideoCapture(referencialImagePath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        ui->referencialNumberLE->setEnabled(false);
    }
    else
    {
        ui->chosenVideoLE->setStyleSheet("color: #339900");
        videoETScorrect = true;
        ui->referencialNumberLE->setEnabled(true);
        int pocet_snimku = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        actualEntropy.fill(0.0,pocet_snimku);
        actualTennengrad.fill(0.0,pocet_snimku);
    }
}

void SingleVideoET::on_chosenVideoLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenVideoETSingle[0]+"/"+arg1+"."+chosenVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        videoETScorrect = false;
        ui->referencialNumberLE->setEnabled(false);
    }
    else
    {
        ui->chosenVideoLE->setStyleSheet("color: #339900");
        videoETScorrect = true;
        chosenVideoETSingle[1] = arg1;
        ui->referencialNumberLE->setEnabled(true);
    }
}

void SingleVideoET::on_horizontalAnomalyCB_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,2);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
}

void SingleVideoET::on_verticalAnomalyCB_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,1);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }

}

void SingleVideoET::on_calculateET_clicked()
{
    QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    //double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    //double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    //pocetSnimkuVidea = cap.get(CV_CAP_PROP_FRAME_COUNT);*/
    /*int uspech_analyzy = entropy_tennengrad_video(cap,ui->actualEntropy,aktualniTennengrad,ui->computationProgress);
    if (uspech_analyzy == 0)
        qDebug()<<"Výpočty skončily chybou.";
    else
    {
        entropie.push_back(ui->actualEntropy);
        tennengrad.push_back(aktualniTennengrad);
        ui->showGraphET->setEnabled(true);
    }*/
    /*if (oznacena_hranice_svetelne_anomalie.x >0.0f && oznacena_hranice_svetelne_anomalie.x < float(sirka))
    {
        obtainedVerticalAnomaly.x = oznacena_hranice_svetelne_anomalie.x;
        obtainedVerticalAnomaly.y = oznacena_hranice_svetelne_anomalie.y;
    }
    else
    {
        obtainedVerticalAnomaly.x = 0.0f;
        obtainedVerticalAnomaly.y = 0.0f;
    }
    if (oznacena_hranice_casove_znacky.y > 0.0f && oznacena_hranice_casove_znacky.y < float(vyska))
    {
        obtainedHorizontalAnomaly.y = oznacena_hranice_casove_znacky.y;
        obtainedHorizontalAnomaly.x = oznacena_hranice_casove_znacky.x;
    }
    else
    {
        obtainedHorizontalAnomaly.y = 0.0f;
        obtainedHorizontalAnomaly.x = 0.0f;
    }*/
    analysedVideos.append(fullPath);
    //temp(pom);
    TFirstP = new qThreadFirstPart(analysedVideos,
                                   SharedVariables::getSharedVariables()->getVerticalAnomalyCoords(),
                                   SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords(),
                                   SharedVariables::getSharedVariables()->getFrangiParameters());
    connect(TFirstP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
    connect(TFirstP,SIGNAL(done(int)),this,SLOT(done(int)));
    connect(TFirstP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
    connect(TFirstP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
    TFirstP->start();
}

void SingleVideoET::on_showGraphET_clicked()
{
    QStringList vektorKzapisu;
    vektorKzapisu.append(chosenVideoETSingle[1]);
    GrafET* graf_ET = new GrafET(entropy,tennengrad,framesFirstFullCompleteEntropyEvaluation,
                                 framesFirstFullCompleteTennengradEvaluation,framesFirstFullCompleteDecision,
                                 framesSecondFullCompleteDecision,framesFinalCompleteDecision,vektorKzapisu,this);
    graf_ET->setModal(true);
    graf_ET->show();
}

void SingleVideoET::on_savePB_clicked()
{   
    QJsonDocument document;
    QJsonObject object;
    QString aktualJmeno = chosenVideoETSingle[1];
    QString cesta = SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+aktualJmeno+".dat";
    qDebug()<<mapDouble["entropie"].length()<<" "<<videoParameters.at(0);
    for (int indexVideo=0; indexVideo<mapDouble["entropie"].length(); indexVideo++){
        for (int parameter = 0; parameter < videoParameters.count(); parameter++){
            qDebug()<<mapDouble[videoParameters.at(parameter)][indexVideo];
            if (parameter < 8){
                QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][indexVideo];
                QJsonArray pomArray = vector2array(pomDouble);
                object[videoParameters.at(parameter)] = pomArray;
            }
            if (parameter >= 8 && parameter < 14){
                QVector<int> pomInt = mapInt[videoParameters.at(parameter)][indexVideo];
                if (parameter == 13)
                    pomInt[framesReferencial[indexVideo]]=2;

                QJsonArray pomArray = vector2array(pomInt);
                object[videoParameters.at(parameter)] = pomArray;
            }
            else{
                if (videoParameters.at(parameter) == "VerticalAnomaly")
                    object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().y);
                else
                    object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().x);
            }
        }

        /*QVector<double> pomVecE = entropie[indexVideo];
        QVector<double> pomVecT = tennengrad[indexVideo];
        QVector<int> pomVecPOE = framesFirstFullCompleteEntropyEvaluation[indexVideo];
        QVector<int> pomVecPOT = framesFirstFullCompleteTennengradEvaluation[indexVideo];
        QVector<int> pomVecPR = framesFirstFullCompleteDecision[indexVideo];
        QVector<int> pomVecDR = framesSecondFullCompleteDecision[indexVideo];
        QVector<int> pomVecO = framesFinalCompleteDecision[indexVideo];
        pomVecO[framesReferencial[indexVideo]] = 2;
        QVector<double> pomVecFX = framesFrangiX[indexVideo];
        QVector<double> pomVecFY = framesFrangiY[indexVideo];
        QVector<double> pomVecFE = framesFrangiEuklid[indexVideo];
        QVector<double> pomVecPX = framesPOCX[indexVideo];
        QVector<double> pomVecPY = framesPOCY[indexVideo];
        QVector<double> pomVecU = framesAngle[indexVideo];*/
    }
    /*
    QJsonArray poleE = vector2array(pomVecE);
    QJsonArray poleT = vector2array(pomVecT);
    QJsonArray polePOE = vector2array(pomVecPOE);
    QJsonArray polePOT = vector2array(pomVecPOT);
    QJsonArray poleO = vector2array(pomVecO);
    QJsonArray polePR = vector2array(pomVecPR);
    QJsonArray poleDR = vector2array(pomVecDR);
    QJsonArray poleFX = vector2array(pomVecFX);
    QJsonArray poleFY = vector2array(pomVecFY);
    QJsonArray poleFE = vector2array(pomVecFE);
    QJsonArray polePX = vector2array(pomVecPX);
    QJsonArray polePY = vector2array(pomVecPY);
    QJsonArray poleU = vector2array(pomVecU);
    QString aktualJmeno = chosenVideoETSingle[1];
    QString cesta = TXTulozeniAktual+"/"+aktualJmeno+".dat";
    object[] = poleE;
    object[] = poleT;
    object[] = poleO;
    object[] = polePOE;
    object[] = polePOT;
    object[] = polePR;
    object[] = poleDR;
    object[] = poleFX;
    object[] = poleFY;
    object[] = poleFE;
    object[] = polePX;
    object[] = polePY;
    object[] = poleU;*/
    document.setObject(object);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName(cesta);
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

void SingleVideoET::done(int done)
{
    if (done == 1)
    {
        mapDouble["entropie"] = TFirstP->computedEntropy();
        mapDouble["tennengrad"] = TFirstP->computedTennengrad();
        mapInt["PrvotOhodEntropie"] = TFirstP->computedFirstEntropyEvaluation();
        mapInt["PrvotOhodTennengrad"] = TFirstP->computedFirstTennengradEvalueation();
        obtainedCutoffStandard = TFirstP->computedCOstandard();
        obtainedCutoffExtra = TFirstP->computedCOextra();
        mapInt["Ohodnoceni"] = TFirstP->computedCompleteEvaluation();
        framesReferencial = TFirstP->estimatedReferencialFrames();
        badFramesComplete = TFirstP->computedBadFrames();
        TFirstP->quit();
        TSecondP = new qThreadSecondPart(analysedVideos,obtainedCutoffStandard,obtainedCutoffExtra,
                                         badFramesComplete,framesReferencial,false);
        connect(TSecondP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TSecondP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TSecondP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TSecondP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        qDebug()<<"First done, starting second...";
        TSecondP->start();

    }
    if (done == 2)
    {
        averageCCcomplete = TSecondP->computedCC();
        averageFWHMcomplete = TSecondP->computedFWHM();
        TSecondP->quit();
        TThirdP = new qThreadThirdPart(analysedVideos,badFramesComplete,mapInt["Ohodnoceni"],
                                       framesReferencial,averageCCcomplete,averageFWHMcomplete,
                                       obtainedCutoffStandard,obtainedCutoffExtra,false);
        connect(TThirdP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TThirdP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TThirdP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TThirdP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        qDebug()<<"Second done, starting third";
        TThirdP->start();

    }
    if (done == 3)
    {
        mapInt["Ohodnoceni"] = TThirdP->framesUpdateEvaluation();
        mapInt["PrvniRozhod"] = TThirdP->framesFirstEvaluationComplete();
        QVector<QVector<double>> KKproblematickychSnimku = TThirdP->framesProblematic_CC();
        QVector<QVector<double>> FWHMproblematickychSnimku = TThirdP->framesProblematic_FWHM();
        mapDouble["FrangiX"] = TThirdP->framesFrangiXestimated();
        mapDouble["FrangiY"] = TThirdP->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = TThirdP->framesFrangiEuklidestimated();
        mapDouble["POCX"] = TThirdP->framesPOCXestimated();
        mapDouble["POCY"] = TThirdP->framesPOCYestimated();
        mapDouble["Uhel"] = TThirdP->framesUhelestimated();
        TThirdP->quit();
        TFourthP = new qThreadFourthPart(analysedVideos,mapInt["PrvniRozhod"],mapInt["Ohodnoceni"],
                                         KKproblematickychSnimku,FWHMproblematickychSnimku,mapDouble["POCX"],
                                         mapDouble["POCY"],mapDouble["Uhel"],mapDouble["FrangiX"],mapDouble["FrangiY"],
                                         mapDouble["FrangiEuklid"],
                                         averageCCcomplete,averageFWHMcomplete);
        connect(TFourthP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TFourthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFourthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFourthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        qDebug()<<"Third done, starting fourth";
        TFourthP->start();
    }
    if (done == 4)
    {
        mapInt["Ohodnoceni"] = TFourthP->framesUpdateEvaluationComplete();
        mapInt["DruheRozhod"] = TFourthP->framesSecondEvaluation();
        mapDouble["FrangiX"] = TFourthP->framesFrangiXestimated();
        mapDouble["FrangiY"] = TFourthP->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = TFourthP->framesFrangiEuklidestimated();
        mapDouble["POCX"] = TFourthP->framesPOCXestimated();
        mapDouble["POCY"] = TFourthP->framesPOCYestimated();
        mapDouble["Uhel"] = TFourthP->framesAngleestimated();
        TFourthP->quit();
        TFifthP = new qThreadFifthPart(analysedVideos,
                                       obtainedCutoffStandard,
                                       obtainedCutoffExtra,
                                       mapDouble["POCX"],
                                       mapDouble["POCY"],
                                       mapDouble["Uhel"],
                                       mapDouble["FrangiX"],
                                       mapDouble["FrangiY"],
                                       mapDouble["FrangiEuklid"],
                                       false,
                                       mapInt["Ohodnoceni"],
                                       mapInt["DruheRozhod"],
                                       framesReferencial,
                                       SharedVariables::getSharedVariables()->getFrangiParameters());
        connect(TFifthP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TFifthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFifthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFifthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        qDebug()<<"Fourth done, starting fifth";
        TFifthP->start();
    }
    if (done == 5)
    {
        mapDouble["FrangiX"] = TFifthP->framesFrangiXestimated();
        mapDouble["FrangiY"] = TFifthP->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = TFifthP->framesFrangiEuklidestimated();
        mapDouble["POCX"] = TFifthP->framesPOCXestimated();
        mapDouble["POCY"] = TFifthP->framesPOCYestimated();
        mapDouble["Uhel"] = TFifthP->framesAngleestimated();
        mapInt["Ohodnoceni"] = TFifthP->framesUpdateEvaluationComplete();
        TFifthP->quit();
        ui->showGraphET->setEnabled(true);
        ui->savePB->setEnabled(true);
        qDebug()<<"Fifth done. Analysis completed";
    }
}

void SingleVideoET::newVideoProcessed(int index)
{
    ui->analysedVideo_label->setText("Analysing: "+analysedVideos.at(index)+" ("+QString::number((index+1))+"/"+QString::number(analysedVideos.size())+")");
}

void SingleVideoET::movedToMethod(int method)
{
    if (method == 0)
        ui->actualAlgorithmPart_label->setText("1/5 Entropy and tennengrad computation");
    if (method == 1)
        ui->actualAlgorithmPart_label->setText("2/5 Average correlation and FWHM");
    if (method == 2)
        ui->actualAlgorithmPart_label->setText("3/5 First decision algorithm started");
    if (method == 3)
        ui->actualAlgorithmPart_label->setText("4/5 Second decision algorithm started");
    if (method == 4)
        ui->actualAlgorithmPart_label->setText("5/5 Third decision algorithm started");
}

void SingleVideoET::on_areaMaximum_editingFinished()
{
    bool ok;
    double input = ui->areaMaximum->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,0.0,20.0,ui->areaMaximum,areaMaximum,areaMaximumCorrect);
        checkValuesPass();
    }
    else
        ui->areaMaximum->setText("");
}

void SingleVideoET::evaluateCorrectValues(){
    if (areaMaximumCorrect && rotationAngleCorrect && iterationCountCorrect){
        ui->calculateET->setEnabled(true);
    }
    else
        ui->calculateET->setEnabled(false);
}

void SingleVideoET::on_rotationAngle_editingFinished()
{
    bool ok;
    double input = ui->rotationAngle->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,0.0,0.5,ui->rotationAngle,rotationAngle,rotationAngleCorrect);
        checkValuesPass();
    }
    else
        ui->rotationAngle->setText("");
}

void SingleVideoET::on_iterationCount_editingFinished()
{
    bool ok;
    double input = ui->iterationCount->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,-1.0,0.0,ui->iterationCount,iterationCount,iterationCountCorrect);
        checkValuesPass();
    }
    else
        ui->iterationCount->setText("");
}

void SingleVideoET::on_referencialNumberLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    int pocet_snimku_videa = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 || zadane_cislo > pocet_snimku_videa)
    {
        ui->referencialNumberLE->setStyleSheet("color: #FF0000");
        referencialNumber = -1;
    }
    else
    {
        ui->referencialNumberLE->setStyleSheet("color: #339900");
        referencialNumber = zadane_cislo;
        ui->horizontalAnomalyCB->setEnabled(true);
        ui->verticalAnomalyCB->setEnabled(true);
    }
}

void SingleVideoET::showDialog(){
    if (ui->verticalAnomalyCB->isChecked())
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,1);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
    if (ui->horizontalAnomalyCB->isChecked())
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,2);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
}

/*void SingleVideoET::temp(QStringList pomList)
{

    /// Firstly, entropy and tennengrad are computed for each frame of each video
    /// This task is indicated by emiting 0 in typeOfMethod
    int kolikateVideo = 0;
    //for (int kolikateVideo = 0; kolikateVideo < pom.count(); kolikateVideo++)
    //{
    QString fullPath = pomList.at(kolikateVideo);
    qDebug()<<"Processing: "<<fullPath;
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    QVector<double> pom(frameCount,0);

    QVector<double> frangi_x,frangi_y,frangi_euklid,POC_x,POC_y,uhel;
    frangi_x = pom;frangi_y=pom;frangi_euklid=pom;POC_x=pom;POC_y=pom;uhel=pom;

    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    cv::Rect vyrez_anomalie(0,0,0,0);
    cv::Point2f hranice_anomalie;
    cv::Point2f hranice_CasZnac;
    if (ziskane_hranice_anomalie.x >0.0f && ziskane_hranice_anomalie.x < float(sirka))
    {
        hranice_anomalie.x = ziskane_hranice_anomalie.x;
        hranice_anomalie.y = ziskane_hranice_anomalie.y;
    }
    else
    {
        hranice_anomalie.x = 0.0f;
        hranice_anomalie.y = 0.0f;
    }
    if (ziskane_hranice_anomalie.y > 0.0f && ziskane_hranice_anomalie.y < float(vyska))
    {
        hranice_CasZnac.y = ziskane_hranice_anomalie.y;
        hranice_CasZnac.x = ziskane_hranice_anomalie.x;
    }
    else
    {
        hranice_CasZnac.x = 0.0f;
        hranice_CasZnac.y = 0.0f;
    }
    if (hranice_anomalie.x != 0.0f)
    {
        if (hranice_anomalie.x < float(sirka/2))
        {
            vyrez_anomalie.x = 0;
            vyrez_anomalie.y = int(hranice_anomalie.y);
            vyrez_anomalie.width = int(sirka)-int(hranice_anomalie.x)-1;
            vyrez_anomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
        }
        if (hranice_anomalie.x > float(sirka/2))
        {
            vyrez_anomalie.x = 0;
            vyrez_anomalie.y = 0;
            vyrez_anomalie.width = int(hranice_anomalie.x);
            vyrez_anomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
        }
    }
    QVector<double> entropyActual,tennengradActual;
    entropyActual.fill(0.0,frameCount);
    tennengradActual.fill(0.0,frameCount);
    int uspech_analyzy = 0;
    if (cap.isOpened() == 0)
        qWarning()<<"Nelze nacist";
    else
    {
        double pocet_snimku_videa = (cap.get(CV_CAP_PROP_FRAME_COUNT));
        qDebug()<< "Analyza videa: "<<kolikateVideo;
        for (double a = 0; a < pocet_snimku_videa; a++)
        {
            //qDebug()<<(kolikateVideo/pocetVidei)*100;//+((a/pocet_snimku_videa)*100.0)/pocetVidei;
            //emit percentageCompleted(qRound((kolikateVideo/pocetVidei)*100+((a/pocet_snimku_videa)*100.0)/pocetVidei));
            cv::Mat snimek;
            double hodnota_entropie;
            cv::Scalar hodnota_tennengrad;
            cap.set(CV_CAP_PROP_POS_FRAMES,(a));
            if (!cap.read(snimek))
                continue;
            else
            {
                kontrola_typu_snimku_8C3(snimek);
                calculateParametersET(snimek,hodnota_entropie,hodnota_tennengrad); /// výpočty proběhnou v pořádku
                double pomocna = hodnota_tennengrad[0];
                //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
                entropyActual[int(a)] = (hodnota_entropie);
                tennengradActual[int(a)] = (pomocna);
                snimek.release();
            }
        }
        uspech_analyzy = 1;
        //procento = qRound(100.0/kolikateVideo+2);
    }
    entropie.push_back(entropyActual);
    tennengrad.push_back(tennengradActual);
    qDebug()<<entropie.size()<<tennengrad.size();

    /// Secondly, it is necessary to select proper maximal value for later selection of bad images
    /// after this procedure, windows for detail analysis of entropy and tennengrad vectors are computed
    /// medians of values in the windows are computed
    double correctEntropyMax = checkMaximum(entropie[kolikateVideo]);
    double correctTennengradMax = checkMaximum(tennengrad[kolikateVideo]);
    QVector<double> windows_tennengrad,windows_entropy,windowsEntropy_medians,windowsTennengrad_medians;
    double restEntropy = 0.0,restTennengrad = 0.0;
    vectorWindows(entropie[kolikateVideo],windows_entropy,restEntropy);
    vectorWindows(tennengrad[kolikateVideo],windows_tennengrad,restTennengrad);
    windowsEntropy_medians = mediansOfVector(entropie[kolikateVideo],windows_entropy,restEntropy);
    windowsTennengrad_medians = mediansOfVector(tennengrad[kolikateVideo],windows_tennengrad,restTennengrad);
    qDebug()<<windowsEntropy_medians;
    qDebug()<<windowsTennengrad_medians;

    /// Thirdly, values of entropy and tennengrad are evaluated and frames get mark good/bad, if they are
    /// or they are not suitable for image registration
    QVector<double> thresholdsEntropy(2,0);
    QVector<double> thresholdsTennengrad(2,0);
    thresholdsEntropy[0] = 0.01;thresholdsEntropy[1] = 0.01;
    thresholdsTennengrad[0] = 10;thresholdsTennengrad[1] = 10;
    double toleranceEntropy = 0.001;
    double toleranceTennengrad = 0.1;
    int dmin = 1;
    QVector<double> badFramesEntropy, badFramesTennengrad;
    QVector<double> nextAnalysisEntropy, nextAnalysisTennengrad;
    analysisFunctionValues(entropie[kolikateVideo],windowsEntropy_medians,windows_entropy,
                           correctEntropyMax,thresholdsEntropy,toleranceEntropy,dmin,restEntropy,
                           badFramesEntropy,nextAnalysisEntropy);
    analysisFunctionValues(tennengrad[kolikateVideo],windowsTennengrad_medians,windows_tennengrad,
                           correctTennengradMax,thresholdsTennengrad,toleranceTennengrad,dmin,restTennengrad,
                           badFramesTennengrad,nextAnalysisTennengrad);
    int referencni_snimek = findReferencialNumber(correctEntropyMax,nextAnalysisEntropy,
                                                         entropie[kolikateVideo]);
    /// Fourth part - frangi filter is applied on the frame marked as the reference
    cv::Mat reference, reference_vyrez;
    cap.set(CV_CAP_PROP_POS_FRAMES,referencni_snimek);
    cap.read(reference);

    cv::Rect vyrez_korelace_extra(0,0,0,0);
    cv::Rect vyrez_korelace_standard(0,0,0,0);
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Mat obraz;
    cv::Point3d frangi_bod(0,0,0);
    bool zmenaMeritka = false;
    preprocessingCompleteRegistration(reference,
                                      obraz,
                                      FrangiParametersVector,
                                      hranice_anomalie,
                                      hranice_CasZnac,
                                      frangi_bod,
                                      vyrez_anomalie,
                                      vyrez_korelace_extra,
                                      vyrez_korelace_standard,
                                      cap,
                                      verticalAnomalySelected,
                                      horizontalAnomalySelected,
                                      zmenaMeritka);
    /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
    /// for decision algorithms
    double prumerny_korelacni_koeficient = 0.0;
    double prumerne_FWHM = 0.0;
    QVector<double> spatne_snimky_komplet = mergeVectors(badFramesEntropy,badFramesTennengrad);
    integrityCheck(spatne_snimky_komplet);
    analyza_FWHM(cap,referencni_snimek,frameCount,zmenaMeritka,prumerny_korelacni_koeficient,prumerne_FWHM,
                                        vyrez_korelace_standard,vyrez_korelace_extra,spatne_snimky_komplet);
    QVector<double> hodnoceniSnimku;
    hodnoceniSnimku = pom;
    for (int a = 0; a < spatne_snimky_komplet.length(); a++)
    {
        hodnoceniSnimku[int(spatne_snimky_komplet[a])] = 1;
    }
    if (restEntropy == 1.0)
    {
        spatne_snimky_komplet.push_back(frameCount-1);
    }
    int do_zbytku = int(restEntropy-1.0);
    if (restEntropy > 1)
    {
        while (do_zbytku >= 0)
        {
            spatne_snimky_komplet.push_back(frameCount-1-do_zbytku);
            do_zbytku -= 1;
        }
        spatne_snimky_komplet.push_back(frameCount-1);
    }
    /// Sixth part - first decision algorithm
    QVector<double> vypoctena_R,vypoctena_FWHM,snimkyKprovereniPrvni;
    firstDecisionAlgorithm(spatne_snimky_komplet,
                      hodnoceniSnimku,
                      POC_x,
                      POC_y,
                      uhel,
                      frangi_x,
                      frangi_y,
                      frangi_euklid,
                      prumerny_korelacni_koeficient,
                      prumerne_FWHM,
                      cap,
                      reference,
                      vyrez_korelace_standard,
                      vyrez_korelace_extra,
                      zmenaMeritka,
                      snimkyKprovereniPrvni,
                      vypoctena_R,
                      vypoctena_FWHM);
    if (snimkyKprovereniPrvni.empty() != true)
    {
        QVector<double> snimkyKprovereniDruhy;
        /// Seventh part - second decision algorithm
        rozhodovani_druhe(snimkyKprovereniPrvni,
                          hodnoceniSnimku,
                          vypoctena_R,
                          vypoctena_FWHM,
                          POC_x,
                          POC_y,
                          uhel,
                          frangi_x,
                          frangi_y,
                          frangi_euklid,
                          prumerny_korelacni_koeficient,
                          prumerne_FWHM,
                          snimkyKprovereniDruhy);
        if (snimkyKprovereniDruhy.empty() != true)
        {
            /// Eigth part - third decision algorithm
            rozhodovani_treti(obraz,
                              vyrez_korelace_extra,
                              vyrez_korelace_standard,
                              frangi_x,
                              frangi_y,
                              frangi_euklid,
                              POC_x,
                              POC_y,
                              uhel,
                              zmenaMeritka,
                              horizontalAnomalySelected,
                              cap,
                              hodnoceniSnimku,
                              snimkyKprovereniDruhy,
                              FrangiParametersVector);
        }
        else
            qWarning()<<"Continuing";
    }
    else
        qWarning()<<"Continuing";
}
//}*/
