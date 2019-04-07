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

    initMaps();

    connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    connect(ui->verticalAnomalyCB,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    connect(ui->horizontalAnomalyCB,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));

    localErrorDialogHandling[ui->calculateET] = new ErrorDialog(ui->calculateET);
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
       ui->chosenVideoLE->setText(chosenVideoETSingle[1]);
    }
}
void SingleVideoET::on_chooseVideoPB_clicked()
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         tr("Choose referencial image"), SharedVariables::getSharedVariables()->getPath("cestaKvideim"),"*.avi;;All files (*)");
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
        ui->chosenVideoLE->setStyleSheet("color: #33aa00");
        videoETScorrect = true;
        ui->referencialNumberLE->setEnabled(true);
        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        actualEntropy.fill(0.0,frameCount);
        actualTennengrad.fill(0.0,frameCount);
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
        ui->chosenVideoLE->setStyleSheet("color: #33aa00");
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
    analysedVideos.append(fullPath);
    TFirstP = new qThreadFirstPart(analysedVideos,
                                   SharedVariables::getSharedVariables()->getVerticalAnomalyCoords(),
                                   SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords(),
                                   SharedVariables::getSharedVariables()->getFrangiParameters());
    connect(TFirstP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
    connect(TFirstP,SIGNAL(done(int)),this,SLOT(done(int)));
    connect(TFirstP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
    connect(TFirstP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
    connect(TFirstP,SIGNAL(unexpectedTermination(QString,int,QString)),this,SLOT(onUnexpectedTermination(QString,int,QString)));
    TFirstP->start();
}

void SingleVideoET::on_showGraphET_clicked()
{
    QStringList videoList;
    videoList.append(chosenVideoETSingle[1]);
    GrafET* graf_ET = new GrafET(
                mapDouble["entropie"],
                mapDouble["tennengrad"],
                mapInt["PrvotOhodEntropie"],
                mapInt["PrvotOhodTennengrad"],
                mapInt["PrvniRozhod"],
                mapInt["DruheRozhod"],
                mapInt["Ohodnoceni"],
                videoList,
                this);
    graf_ET->setModal(true);
    graf_ET->show();
}

void SingleVideoET::on_savePB_clicked()
{   
    QJsonDocument document;
    QJsonObject object;
    QString actualName = chosenVideoETSingle[1];
    QString path = SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+actualName+".dat";
    qDebug()<<mapDouble["entropie"].length()<<" "<<videoParameters.at(0);
    for (int indexVideo=0; indexVideo<mapDouble["entropie"].length(); indexVideo++){
        for (int parameter = 0; parameter < videoParameters.count(); parameter++){
            qDebug()<<videoParameters.at(parameter);
            //qDebug()<<mapDouble[videoParameters.at(parameter)][indexVideo];
            if (parameter < 8){
                QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][indexVideo];
                QJsonArray pomArray = vector2array(pomDouble);
                object[videoParameters.at(parameter)] = pomArray;
            }
            else if (parameter >= 8 && parameter <= 12){
                QVector<int> pomInt = mapInt[videoParameters.at(parameter)][indexVideo];
                if (videoParameters.at(parameter) == "Ohodnoceni")
                    pomInt[framesReferencial[indexVideo]]=2;

                QJsonArray pomArray = vector2array(pomInt);
                object[videoParameters.at(parameter)] = pomArray;
            }
            else{
                if (videoParameters.at(parameter) == "VerticalAnomaly")
                    object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().y);
                else
                    object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().x);
            }
        }
        document.setObject(object);
        QString documentString = document.toJson();
        QFile writer;
        writer.setFileName(path);
        writer.open(QIODevice::WriteOnly);
        writer.write(documentString.toLocal8Bit());
        writer.close();
    }
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
        TSecondP = new qThreadSecondPart(analysedVideos,
                                         obtainedCutoffStandard,
                                         obtainedCutoffExtra,
                                         badFramesComplete,
                                         framesReferencial,false);
        connect(TSecondP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TSecondP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TSecondP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TSecondP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TFirstP->isFinished()){
            qDebug()<<"First done, starting second...";
            TFirstP->terminate();
            TSecondP->start();
        //}
    }
    if (done == 2)
    {
        averageCCcomplete = TSecondP->computedCC();
        averageFWHMcomplete = TSecondP->computedFWHM();
        TThirdP = new qThreadThirdPart(analysedVideos,
                                       badFramesComplete,
                                       mapInt["Ohodnoceni"],
                framesReferencial,
                averageCCcomplete,
                averageFWHMcomplete,
                obtainedCutoffStandard,
                obtainedCutoffExtra,false);
        connect(TThirdP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TThirdP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TThirdP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TThirdP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TSecondP->isFinished()){
            qDebug()<<"Second done, starting third...";
            TSecondP->terminate();
            TThirdP->start();
        //}
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
        TFourthP = new qThreadFourthPart(analysedVideos,mapInt["PrvniRozhod"],mapInt["Ohodnoceni"],
                                         KKproblematickychSnimku,FWHMproblematickychSnimku,mapDouble["POCX"],
                                         mapDouble["POCY"],mapDouble["Uhel"],mapDouble["FrangiX"],mapDouble["FrangiY"],
                                         mapDouble["FrangiEuklid"],
                                         averageCCcomplete,averageFWHMcomplete);
        connect(TFourthP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TFourthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFourthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFourthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TThirdP->isFinished()){
            qDebug()<<"Third done, starting fourth...";
            TThirdP->terminate();
            TFourthP->start();
        //}
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
        //if (TFourthP->isFinished()){
            qDebug()<<"Fourth done, starting fifth";
            TFourthP->terminate();
            TFifthP->start();
        //}
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
        ui->showGraphET->setEnabled(true);
        ui->savePB->setEnabled(true);
        ui->actualAlgorithmPart_label->setText(tr("Fifth part done. Analysis completed"));
        //if (TFifthP->isFinished()){
            qDebug()<<"Fifth done.";
            TFifthP->terminate();
       // }
    }
}

void SingleVideoET::newVideoProcessed(int index)
{
    ui->analysedVideo_label->setText("Analysing: "+analysedVideos.at(index)+" ("+QString::number((index+1))+"/"+QString::number(analysedVideos.size())+")");
}

void SingleVideoET::movedToMethod(int method)
{
    if (method == 0)
        ui->actualAlgorithmPart_label->setText(tr("1/5 Entropy and tennengrad computation"));
    if (method == 1)
        ui->actualAlgorithmPart_label->setText(tr("2/5 Average correlation and FWHM"));
    if (method == 2)
        ui->actualAlgorithmPart_label->setText(tr("3/5 First decision algorithm started"));
    if (method == 3)
        ui->actualAlgorithmPart_label->setText(tr("4/5 Second decision algorithm started"));
    if (method == 4)
        ui->actualAlgorithmPart_label->setText(tr("5/5 Third decision algorithm started"));
}

void SingleVideoET::onUnexpectedTermination(QString message, int threadNumber, QString errorType){
    localErrorDialogHandling[ui->calculateET]->evaluate("left",errorType,message);
    localErrorDialogHandling[ui->calculateET]->show();
    if (errorType == "hardError"){
        if (threadNumber == 1)
            TFirstP->terminate();
        if(threadNumber == 2)
            TSecondP->terminate();
        if(threadNumber == 3)
            TThirdP->terminate();
        if(threadNumber == 5)
            TFifthP->terminate();
    }
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
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int input = arg1.toInt();
    if (input < 0 || input > frameCount)
    {
        ui->referencialNumberLE->setStyleSheet("color: #FF0000");
        referencialNumber = -1;
    }
    else
    {
        ui->referencialNumberLE->setStyleSheet("color: #33aa00");
        referencialNumber = input;
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
