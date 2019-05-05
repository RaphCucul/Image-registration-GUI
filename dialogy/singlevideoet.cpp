#include "dialogy/singlevideoet.h"
#include "ui_singlevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "hlavni_program/frangi_detektor.h"
#include "dialogy/graphet_parent.h"
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
    ui->referencialNumberLE->setPlaceholderText(tr("Ref."));
    ui->horizontalAnomalyCB->setText(tr("Top/bottom anomaly"));
    ui->verticalAnomalyCB->setText(tr("Left/right anomaly"));
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerancelabel->setText(tr("Maximal tolerated rotation angle"));
    ui->iterationNumberlabel->setText(tr("Number of iterations of algorithm"));
    ui->calculateET->setText(tr("Analyze video"));
    ui->showGraphET->setText(tr("Show computed results"));
    ui->savePB->setText(tr("Save results"));

    QObject::connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    QObject::connect(ui->verticalAnomalyCB,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(ui->horizontalAnomalyCB,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));

    localErrorDialogHandling[ui->calculateET] = new ErrorDialog(ui->calculateET);
}

SingleVideoET::~SingleVideoET()
{
    delete ui;
}

void SingleVideoET::checkPaths(){
    if (SharedVariables::getSharedVariables()->getPath("cestaKvideim") == ""){
        ui->chosenVideoLE->setPlaceholderText(tr("Chosen video"));
        ui->chosenVideoLE->setReadOnly(true);
    }
    else{
       analyseAndSaveFirst(SharedVariables::getSharedVariables()->getPath("cestaKvideim"),chosenVideoETSingle);
       ui->chosenVideoLE->setText(chosenVideoETSingle[1]);
       ui->chosenVideoLE->setReadOnly(false);
    }
}
void SingleVideoET::on_chooseVideoPB_clicked()
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         tr("Choose referencial image"), SharedVariables::getSharedVariables()->getPath("cestaKvideim"),tr("*.avi;;;"));
    if (referencialImagePath != ""){
        ui->chosenVideoLE->setReadOnly(false);
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
            //ui->referencialNumberLE->setEnabled(true);
            int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
            actualEntropy.fill(0.0,frameCount);
            actualTennengrad.fill(0.0,frameCount);
            ui->horizontalAnomalyCB->setEnabled(true);
            ui->verticalAnomalyCB->setEnabled(true);
        }
    }
    else{
        ui->chosenVideoLE->setReadOnly(true);
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
        ui->horizontalAnomalyCB->setEnabled(false);
        ui->verticalAnomalyCB->setEnabled(false);
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
    if (runStatus){
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());

        if (cap.isOpened()){
            analysedVideos.clear();
            analysedVideos.append(fullPath);
            First[1] = new qThreadFirstPart(analysedVideos,
                                            SharedVariables::getSharedVariables()->getVerticalAnomalyCoords(),
                                            SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords(),
                                            SharedVariables::getSharedVariables()->getFrangiParameters());
            QObject::connect(First[1],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
            QObject::connect(First[1],SIGNAL(done(int)),this,SLOT(onDone(int)));
            QObject::connect(First[1],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
            QObject::connect(First[1],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
            QObject::connect(First[1],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
            QObject::connect(First[1],SIGNAL(readyForFinish()),First[1],SLOT(deleteLater()));
            First[1]->start();

            initMaps();
            canProceed = true;
            emit calculationStarted();
            ui->calculateET->setText(tr("Cancel"));
            runStatus = false;
        }
        else{
            localErrorDialogHandling[ui->calculateET]->evaluate("left","hardError",6);
            localErrorDialogHandling[ui->calculateET]->show();
        }
    }
    else {
        cancelAllCalculations();
        runStatus = true;
        ui->calculateET->setText(tr("Analyze video"));
        ui->computationProgress->setValue(0);
        ui->actualAlgorithmPart_label->setText("");
        ui->analysedVideo_label->setText("");
    }
}

void SingleVideoET::on_showGraphET_clicked()
{
    QStringList videoList;
    videoList.append(chosenVideoETSingle[1]);
    GraphET_parent* graph = new GraphET_parent(videoList,
                                               mapDouble["entropie"],
                                               mapDouble["tennengrad"],
                                               mapInt["PrvotOhodEntropie"],
                                               mapInt["PrvotOhodTennengrad"],
                                               mapInt["PrvniRozhod"],
                                               mapInt["DruheRozhod"],
                                               mapInt["Ohodnoceni"],
                                               this);
    graph->setModal(true);
    graph->show();
}

void SingleVideoET::on_savePB_clicked()
{   
    QJsonDocument document;
    QJsonObject object;
    QString actualName = chosenVideoETSingle[1];
    QString path = SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+actualName+".dat";
    for (int indexVideo=0; indexVideo<mapDouble["entropie"].length(); indexVideo++){
        for (int parameter = 0; parameter < videoParameters.count(); parameter++){
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

void SingleVideoET::onDone(int thread){
    done(thread);

    if (thread == 1 && canProceed){
        qDebug()<<"First done, starting second...";

        Second[2] = new qThreadSecondPart(analysedVideos,
                                          badVideos,
                                          obtainedCutoffStandard,
                                          obtainedCutoffExtra,
                                          badFramesComplete,
                                          framesReferencial,false);
        QObject::connect(Second[2],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Second[2],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Second[2],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Second[2],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Second[2],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_second()),Second[2],SLOT(onDataObtained()));
        QObject::connect(Second[2],SIGNAL(readyForFinish()),Second[2],SLOT(deleteLater()));

        Second[2]->start();
        qDebug()<<"Started";
    }
    else if (thread == 2 && canProceed){
        qDebug()<<"Second done, starting third...";

        Third[3] = new qThreadThirdPart(analysedVideos,
                                        badVideos,
                                        badFramesComplete,
                                        mapInt["Ohodnoceni"],
                                        framesReferencial,
                                        averageCCcomplete,
                                        averageFWHMcomplete,
                                        obtainedCutoffStandard,
                                        obtainedCutoffExtra,false);
        QObject::connect(Third[3],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Third[3],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Third[3],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Third[3],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Third[3],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_third()),Third[3],SLOT(onDataObtained()));
        QObject::connect(Third[3],SIGNAL(readyForFinish()),Third[3],SLOT(deleteLater()));

        Third[3]->start();
        qDebug()<<"Started";
    }
    else if (thread == 3 && canProceed){
        qDebug()<<"Third done, starting fourth...";

        Fourth[4] = new qThreadFourthPart(analysedVideos,
                                          badVideos,
                                          mapInt["PrvniRozhod"],
                                          mapInt["Ohodnoceni"],
                                          CC_problematicFrames,
                                          FWHM_problematicFrames,
                                          mapDouble["POCX"],
                                          mapDouble["POCY"],
                                          mapDouble["Uhel"],
                                          mapDouble["FrangiX"],
                                          mapDouble["FrangiY"],
                                          mapDouble["FrangiEuklid"],
                                          averageCCcomplete,
                                          averageFWHMcomplete);
        QObject::connect(Fourth[4],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Fourth[4],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Fourth[4],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Fourth[4],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(this,SIGNAL(dataObtained_fourth()),Fourth[4],SLOT(onDataObtained()));
        QObject::connect(Fourth[4],SIGNAL(readyForFinish()),Fourth[4],SLOT(deleteLater()));

        Fourth[4]->start();
        qDebug()<<"Started";
    }
    else if (thread == 4 && canProceed){
        qDebug()<<"Fourth done, starting fifth";

        Fifth[5] = new qThreadFifthPart(analysedVideos,
                                        badVideos,
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
                                       SharedVariables::getSharedVariables()->getFrangiParameters(),
                                        int(iterationCount),
                                        areaMaximum,
                                        rotationAngle);
        QObject::connect(Fifth[5],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Fifth[5],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Fifth[5],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Fifth[5],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Fifth[5],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_fifth()),Fifth[5],SLOT(onDataObtained()));
        QObject::connect(Fifth[5],SIGNAL(readyForFinish()),Fifth[5],SLOT(deleteLater()));

        Fifth[5]->start();
        qDebug()<<"Started";
    }
    else if (thread == 5 && canProceed){
        ui->showGraphET->setEnabled(true);
        ui->savePB->setEnabled(true);
        ui->actualAlgorithmPart_label->setText(tr("Fifth part done. Analysis completed"));
        qDebug()<<"Fifth done.";
        emit calculationStopped();
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

void SingleVideoET::onUnexpectedTermination(int videoIndex, QString errorType){
    Q_UNUSED(videoIndex);
    localErrorDialogHandling[ui->calculateET]->evaluate("left",errorType,"Video could not be analysed.");
    localErrorDialogHandling[ui->calculateET]->show();
    if (errorType == "hardError"){
        cancelAllCalculations();
        emit calculationStopped();
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

void SingleVideoET::disableWidgets(){
    ui->chooseVideoPB->setEnabled(false);
    ui->chosenVideoLE->setEnabled(false);
    ui->verticalAnomalyCB->setEnabled(false);
    ui->horizontalAnomalyCB->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->showGraphET->setEnabled(false);
}

void SingleVideoET::enableWidgets(){
    ui->chooseVideoPB->setEnabled(true);
    ui->chosenVideoLE->setEnabled(true);
    ui->verticalAnomalyCB->setEnabled(true);
    ui->horizontalAnomalyCB->setEnabled(true);
    ui->savePB->setEnabled(true);
    ui->showGraphET->setEnabled(true);
}
