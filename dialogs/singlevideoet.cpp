#include "dialogs/singlevideoet.h"
#include "ui_singlevideoet.h"
#include "image_analysis/entropy.h"
#include "image_analysis/frangi_utilization.h"
#include "image_analysis/image_processing.h"
#include "main_program/frangi_detektor.h"
#include "dialogs/graphet_parent.h"
#include "dialogs/clickimageevent.h"
#include "multithreadET/qThreadFirstPart.h"
#include "util/vector_operations.h"
#include "util/files_folders_operations.h"
#include "registration/multiPOC_Ai1.h"
#include "shared_staff/sharedvariables.h"

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
    ui->standardCutout->setEnabled(false);
    ui->extraCutout->setEnabled(false);
    ui->calculateET->setEnabled(false);
    ui->showGraphET->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->areaMaximum->setPlaceholderText("0 - 20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationCount->setPlaceholderText("1 - Inf; -1~automatic settings");

    ui->chooseVideoPB->setText(tr("Choose video"));
    ui->standardCutout->setText(tr("Modify standard cutout"));
    ui->extraCutout->setText(tr("Modify extra cutout"));
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerancelabel->setText(tr("Maximal tolerated rotation angle"));
    ui->iterationNumberlabel->setText(tr("Number of iterations of algorithm"));
    ui->calculateET->setText(tr("Analyze video"));
    ui->showGraphET->setText(tr("Show computed results"));
    ui->savePB->setText(tr("Save results"));
    ui->previousThresholdsCB->setText(tr("Use previous thresholds"));
    ui->previousThresholdsCB->setEnabled(false);

    QObject::connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    QObject::connect(ui->standardCutout,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(ui->extraCutout,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));

    localErrorDialogHandling[ui->calculateET] = new ErrorDialog(ui->calculateET);
}

SingleVideoET::~SingleVideoET()
{
    delete ui;
}

void SingleVideoET::checkPaths(){
    if (SharedVariables::getSharedVariables()->getPath("videosPath") == ""){
        ui->chosenVideoLE->setPlaceholderText(tr("Chosen video"));
        ui->chosenVideoLE->setReadOnly(true);
    }
    else{
       analyseAndSaveFirst(SharedVariables::getSharedVariables()->getPath("videosPath"),chosenVideoETSingle);
       ui->chosenVideoLE->setText(chosenVideoETSingle[1]);
       ui->chosenVideoLE->setReadOnly(false);
    }
}
void SingleVideoET::on_chooseVideoPB_clicked()
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         tr("Choose referencial image"), SharedVariables::getSharedVariables()->getPath("videosPath"),tr("*.avi;;;"));
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
            cap.release();
        }
        else
        {
            ui->chosenVideoLE->setStyleSheet("color: #33aa00");
            videoETScorrect = true;
            int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
            actualEntropy.fill(0.0,frameCount);
            actualTennengrad.fill(0.0,frameCount);
            ui->standardCutout->setEnabled(true);
            ui->extraCutout->setEnabled(true);
        }
    }
    else{
        ui->chosenVideoLE->setReadOnly(true);
    }
}

void SingleVideoET::on_chosenVideoLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenVideoETSingle[0]+"/"+arg1+"."+chosenVideoETSingle[2];
    QFile file(fullPath);
    if (!file.exists()){
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        videoETScorrect = false;
        ui->standardCutout->setEnabled(false);
        ui->extraCutout->setEnabled(false);
    }
    else
    {
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        ui->chosenVideoLE->setStyleSheet("color: #339900");
        videoETScorrect = true;
        ui->standardCutout->setEnabled(false);
        ui->extraCutout->setEnabled(false);
        chosenVideoETSingle[1] = arg1;
        QVector<double> videoETthresholds;
        if (checkAndLoadData("thresholds",arg1,videoETthresholds)){
            fillMap(arg1,videoETthresholds,mapDouble["thresholds"]);
            //mapDouble["thresholds"][arg1] = videoETthresholds;
            ui->previousThresholdsCB->setEnabled(true);
            ETthresholdsFound.insert(arg1,true);
        }
        else{
            ETthresholdsFound.insert(arg1,false);
            ui->previousThresholdsCB->setEnabled(false);
        }
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
            videoNamesList.append(chosenVideoETSingle[1]);
            initMaps(videoNamesList);
            canProceed = true;
            First[1] = new qThreadFirstPart(analysedVideos,
                                            ui->extraCutout->isChecked(),
                                            SharedVariables::getSharedVariables()->getFrangiParameters(),
                                            SharedVariables::getSharedVariables()->getFrangiMargins(),
                                            SharedVariables::getSharedVariables()->getFrangiRatios(),
                                            mapDouble["thresholds"],
                                            ETthresholdsFound,
                                            ui->previousThresholdsCB->isChecked());
            QObject::connect(First[1],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
            QObject::connect(First[1],SIGNAL(done(int)),this,SLOT(onDone(int)));
            QObject::connect(First[1],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
            QObject::connect(First[1],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
            QObject::connect(First[1],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
            QObject::connect(First[1],SIGNAL(readyForFinish()),First[1],SLOT(deleteLater()));
            First[1]->start();
            emit calculationStarted();
            ui->calculateET->setText(tr("Cancel"));
            runStatus = false;
        }
        else{
            localErrorDialogHandling[ui->calculateET]->evaluate("left","hardError",6);
            localErrorDialogHandling[ui->calculateET]->show(false);
            canProceed = false;
            cap.release();
        }
    }
    else {
        cancelAllCalculations(videoNamesList);
        runStatus = true;
        canProceed = false;
        ui->calculateET->setText(tr("Analyze video"));
        ui->computationProgress->setValue(0);
        ui->actualAlgorithmPart_label->setText("");
        ui->analysedVideo_label->setText("");
    }
}

void SingleVideoET::on_showGraphET_clicked()
{
    GraphET_parent* graph = new GraphET_parent(analysedVideos,
                                               mapDouble["entropy"],
                                               mapDouble["tennengrad"],
                                               mapDouble["thresholds"],
                                               mapInt["firstEvalEntropy"],
                                               mapInt["firstEvalTennengrad"],
                                               mapInt["firstEval"],
                                               mapInt["secondEval"],
                                               mapInt["evaluation"]);
    connect(graph,SIGNAL(saveCalculatedData(QString,QJsonObject)),this,SLOT(onSaveFromGraphET(QString,QJsonObject)));
    graph->setModal(true);
    graph->show();
}

void SingleVideoET::on_savePB_clicked()
{   
    /*QJsonDocument document;
    QJsonObject object;
    QString actualName = chosenVideoETSingle[1];
    QString path = SharedVariables::getSharedVariables()->getPath("saveDatFilesPath")+"/"+actualName+".dat";
    //for (int indexVideo=0; indexVideo<mapDouble[actualName]["entropy"].length(); indexVideo++){
    foreach (QString name,videoNamesList){
        if (badVideos.indexOf(name) == -1){
            for (int parameter = 0; parameter < videoParameters.count(); parameter++){
                if (parameter < 8){
                    QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][name];
                    QJsonArray pomArray = vector2array(pomDouble);
                    object[videoParameters.at(parameter)] = pomArray;
                }
                else if (parameter >= 8 && parameter <= 12){
                    QVector<int> pomInt = mapInt[videoParameters.at(parameter)][name];
                    if (videoParameters.at(parameter) == "evaluation"){
                        pomInt[framesReferencial[name]]=2;
                    }
                    else{
                        QJsonArray pomArray = vector2array(pomInt);
                        object[videoParameters.at(parameter)] = pomArray;
                    }
                }
                else{*/
                    /*if (videoParameters.at(parameter) == "VerticalAnomaly")
                    object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().y);
                else
                    object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().x);*/
                /*}
            }
            document.setObject(object);
            QString documentString = document.toJson();
            QFile writer;
            writer.setFileName(path);
            writer.open(QIODevice::WriteOnly);
            writer.write(documentString.toLocal8Bit());
            writer.close();
        }
    }*/
    saveVideoAnalysisResults();
}

void SingleVideoET::onSaveFromGraphET(QString i_videoName, QJsonObject i_object){
    /*QJsonDocument document;
    QJsonObject object;
    //QString actualName = chosenVideoETSingle[1];
    QString path = SharedVariables::getSharedVariables()->getPath("saveDatFilesPath")+"/"+i_videoName+".dat";
    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 8){
            QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][i_videoName];
            QJsonArray pomArray = vector2array(pomDouble);
            object[videoParameters.at(parameter)] = pomArray;
        }
        else if (parameter >= 8 && parameter <= 12){
            QVector<int> pomInt = mapInt[videoParameters.at(parameter)][i_videoName];
            if (videoParameters.at(parameter) == "evaluation")
                pomInt[framesReferencial[i_videoName]]=2;

            QJsonArray pomArray = vector2array(pomInt);
            object[videoParameters.at(parameter)] = pomArray;
        }
    }
    document.setObject(object);
    QString documentString = document.toJson();
    QFile writer;
    writer.setFileName(path);
    writer.open(QIODevice::WriteOnly);
    writer.write(documentString.toLocal8Bit());
    writer.close();*/
    saveVideoAnalysisResultsFromGraphET(i_videoName, i_object);
}

void SingleVideoET::onDone(int thread){
    done(thread);

    if (thread == 1 && canProceed){
        qDebug()<<"First done, starting second...";

        Second[2] = new qThreadSecondPart(analysedVideos,
                                          badVideos,
                                          mapAnomalies["standard"],
                                          mapAnomalies["extra"],
                                          badFramesComplete,
                                          framesReferencial,
                                          ui->extraCutout->isChecked(),
                                          areaMaximum);
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
                                        mapInt["evaluation"],
                                        framesReferencial,
                                        averageCCcomplete,
                                        averageFWHMcomplete,
                                        mapAnomalies["standard"],
                                        mapAnomalies["extra"],
                                        false,
                                        areaMaximum);
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
                                          mapInt["firstEval"],
                                          mapInt["evaluation"],
                                          CC_problematicFrames,
                                          FWHM_problematicFrames,
                                          mapDouble["POCX"],
                                          mapDouble["POCY"],
                                          mapDouble["angle"],
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
                                        mapAnomalies["standard"],
                                        mapAnomalies["extra"],
                                        mapDouble["POCX"],
                                        mapDouble["POCY"],
                                        mapDouble["angle"],
                                        mapDouble["FrangiX"],
                                        mapDouble["FrangiY"],
                                        mapDouble["FrangiEuklid"],
                                        false,
                                        mapInt["evaluation"],
                                        mapInt["secondEval"],
                                        framesReferencial,
                                        SharedVariables::getSharedVariables()->getFrangiParameters(),
                                        int(iterationCount),
                                        areaMaximum,
                                        rotationAngle,
                                        SharedVariables::getSharedVariables()->getFrangiMargins());
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
        ui->calculateET->setText(tr("Analyze video"));
        emit calculationStopped();
        runStatus = true;
        canProceed = false;
        First.clear();
        Second.clear();
        Third.clear();
        Fourth.clear();
        Fifth.clear();
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
    Q_UNUSED(videoIndex)
    localErrorDialogHandling[ui->calculateET]->evaluate("left",errorType,"Video could not be analysed.");
    localErrorDialogHandling[ui->calculateET]->show(false);
    if (errorType == "hardError"){
        cancelAllCalculations(videoNamesList);
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
        ui->standardCutout->setEnabled(true);
        ui->extraCutout->setEnabled(true);
    }
    else{
        ui->calculateET->setEnabled(false);
        ui->standardCutout->setEnabled(false);
        ui->extraCutout->setEnabled(false);
    }
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

void SingleVideoET::showDialog(){
    if (ui->standardCutout->isChecked())
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,cutoutType::STANDARD);
        markAnomaly->setModal(true);
        markAnomaly->show();

    }
    if (ui->extraCutout->isChecked())
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,cutoutType::EXTRA);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
}

void SingleVideoET::disableWidgets(){
    ui->chooseVideoPB->setEnabled(false);
    ui->chosenVideoLE->setEnabled(false);
    ui->standardCutout->setEnabled(false);
    ui->extraCutout->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->showGraphET->setEnabled(false);
}

void SingleVideoET::enableWidgets(){
    ui->chooseVideoPB->setEnabled(true);
    ui->chosenVideoLE->setEnabled(true);
    ui->standardCutout->setEnabled(true);
    ui->extraCutout->setEnabled(true);
    ui->savePB->setEnabled(true);
    ui->showGraphET->setEnabled(true);
}
