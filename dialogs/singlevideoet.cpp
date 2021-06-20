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
    ui->areaMaximum->setPlaceholderText("1 - 20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationCount->setPlaceholderText("1 - Inf; -1");

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
       ui->chosenVideoLE->setText(chosenVideoETSingle["filename"]);
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

        chosenVideoETSingle["folder"] = folder;
        chosenVideoETSingle["filename"] = filename;
        chosenVideoETSingle["suffix"] = suffix;
        ui->chosenVideoLE->setText(chosenVideoETSingle["filename"]);
        evaluateVideoImageInput(referencialImagePath);        
    }
}

void SingleVideoET::on_chosenVideoLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenVideoETSingle["folder"]+"/"+arg1+"."+chosenVideoETSingle["suffix"];
    QFile file(fullPath);
    if (!file.exists()){
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        videoETScorrect = false;
        ui->standardCutout->setEnabled(false);
        ui->extraCutout->setEnabled(false);
    }
    else
    {
        videoETScorrect = true;
        chosenVideoETSingle["filename"] = arg1;
        evaluateVideoImageInput(fullPath);
    }
}

bool SingleVideoET::evaluateVideoImageInput(QString i_path){
    cv::VideoCapture cap = cv::VideoCapture(i_path.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        ui->areaMaximum->setEnabled(false);
        ui->rotationAngle->setEnabled(false);
        ui->iterationCount->setEnabled(false);
        cap.release();
        return false;
    }
    else
    {
        ui->chosenVideoLE->setStyleSheet("color: #339900");

        QVector<int> standardCutout,extraCutout;
        if (checkAndLoadData("standard",chosenVideoETSingle["filename"],standardCutout)) {
            if (standardCutout.length() == 4 && standardCutout[2] > 0 && standardCutout[3] > 0) {
                cv::Rect _p0 = convertVector2Rect(standardCutout);
                QRect _p1 = convertRectToQRect(_p0);
                SharedVariables::getSharedVariables()->setVideoInformation(chosenVideoETSingle["filename"],"standard",_p1);
                standardLoaded = true;
                ui->standardCutout->setChecked(true);
                selectedCutout = cutoutType::STANDARD;
            }
            else {
                standardLoaded = false;
                ui->standardCutout->setChecked(false);
            }
        }
        else {
            standardLoaded = false;
            ui->standardCutout->setChecked(false);
        }

        if (checkAndLoadData("extra",chosenVideoETSingle["filename"],extraCutout)) {
            if (extraCutout.length() == 4 && extraCutout[2] > 0 && extraCutout[3] > 0) {
                cv::Rect _p0 = convertVector2Rect(extraCutout);
                QRect _p1 = convertRectToQRect(_p0);
                SharedVariables::getSharedVariables()->setVideoInformation(chosenVideoETSingle["filename"],"extra",_p1);
                extraLoaded = true;
                ui->extraCutout->setChecked(true);
                selectedCutout = cutoutType::EXTRA;
            }
            else {
                extraLoaded = false;
                ui->extraCutout->setChecked(false);
            }
        }
        else {
            extraLoaded = false;
            ui->extraCutout->setChecked(false);
        }

        QVector<double> videoETthresholds;
        if (checkAndLoadData("thresholds",chosenVideoETSingle["filename"],videoETthresholds)){
            temporalySavedThresholds.insert(chosenVideoETSingle["filename"],videoETthresholds);
            ui->previousThresholdsCB->setEnabled(true);
            ETthresholdsFound.insert(chosenVideoETSingle["filename"],true);
        }
        else{
            ETthresholdsFound.insert(chosenVideoETSingle["filename"],false);
            ui->previousThresholdsCB->setEnabled(false);
        }

        return true;
    }

}

void SingleVideoET::on_calculateET_clicked()
{
    if (runStatus){
        QString fullPath = chosenVideoETSingle["folder"]+"/"+chosenVideoETSingle["filename"]+"."+chosenVideoETSingle["suffix"];
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        if (cap.isOpened()){
            analysedVideos.clear();
            analysedVideos.append(fullPath);
            videoNamesList.append(chosenVideoETSingle["filename"]);
            initMaps();
            canProceed = true;
            if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoETSingle["filename"])){
                qDebug()<<"Success";
            }
            First[1] = new qThreadFirstPart(analysedVideos,
                                            selectedCutout,
                                            temporalySavedThresholds,
                                            ETthresholdsFound,
                                            ui->previousThresholdsCB->isChecked());
            QObject::connect(First[1],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
            QObject::connect(First[1],SIGNAL(done(int)),this,SLOT(onDone(int)));
            QObject::connect(First[1],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
            QObject::connect(First[1],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
            QObject::connect(First[1],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
            QObject::connect(this,SIGNAL(dataObtained_first()),First[1],SLOT(onDataObtained()));
            QObject::connect(First[1],&qThreadFirstPart::readyForFinish,[=](){
                if (!First.isEmpty()) {
                    if (First[1]->isRunning()){
                        First[1]->terminate();
                        First[1]->wait(200);
                        First[1]->deleteLater();
                    }
                    else
                        First[1]->deleteLater();
                }
            });
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
        cancelAllCalculations();
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
                                               mapInt["evaluation"],
                                               framesReferencial);
    connect(graph,SIGNAL(saveCalculatedData(QString,QJsonObject)),this,SLOT(onSaveFromGraphET(QString,QJsonObject)));
    graph->setModal(true);
    graph->show();
}

void SingleVideoET::on_savePB_clicked()
{
    saveVideoAnalysisResults();
}

void SingleVideoET::onSaveFromGraphET(QString i_videoName, QJsonObject i_object){
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
                                          selectedCutout,
                                          areaMaximum);
        QObject::connect(Second[2],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Second[2],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Second[2],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Second[2],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Second[2],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_second()),Second[2],SLOT(onDataObtained()));
        QObject::connect(Second[2],&qThreadSecondPart::readyForFinish,[=](){
            if (Second[2]->isRunning()){
                Second[2]->terminate();
                Second[2]->wait(200);
                Second[2]->deleteLater();
            }
            else
                Second[2]->deleteLater();
        });

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
                                        selectedCutout,
                                        areaMaximum);
        QObject::connect(Third[3],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Third[3],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Third[3],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Third[3],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Third[3],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_third()),Third[3],SLOT(onDataObtained()));
        QObject::connect(Third[3],&qThreadThirdPart::readyForFinish,[=](){
            if (Third[3]->isRunning()){
                Third[3]->terminate();
                Third[3]->wait(200);
                Third[3]->deleteLater();
            }
            else
                Third[3]->deleteLater();
        });

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
        QObject::connect(Fourth[4],&qThreadFourthPart::readyForFinish,[=](){
            if (Fourth[4]->isRunning()){
                Fourth[4]->terminate();
                Fourth[4]->wait(200);
                Fourth[4]->deleteLater();
            }
            else
                Fourth[4]->deleteLater();
        });

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
                                        selectedCutout,
                                        mapInt["evaluation"],
                                        mapInt["secondEval"],
                                        framesReferencial,
                                        int(iterationCount),
                                        areaMaximum,
                                        rotationAngle);
        QObject::connect(Fifth[5],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Fifth[5],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Fifth[5],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Fifth[5],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Fifth[5],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_fifth()),Fifth[5],SLOT(onDataObtained()));
        QObject::connect(Fifth[5],&qThreadFifthPart::readyForFinish,[=](){
            if (Fifth[5]->isRunning()){
                Fifth[5]->terminate();
                Fifth[5]->wait(200);
                Fifth[5]->deleteLater();
            }
            else
                Fifth[5]->deleteLater();
        });

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
        cancelAllCalculations();
        emit calculationStopped();
    }
}

void SingleVideoET::on_areaMaximum_editingFinished()
{
    bool ok;
    double input = ui->areaMaximum->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,1.0,20.0,ui->areaMaximum,areaMaximum,areaMaximumCorrect);
        emit checkValuesPass();
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
        emit checkValuesPass();
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
        emit checkValuesPass();
    }
    else
        ui->iterationCount->setText("");
}

void SingleVideoET::showDialog(){
    if (QObject::sender() == ui->standardCutout && !standardLoaded) {
        if (ui->standardCutout->isChecked())
        {
            QString fullPath = chosenVideoETSingle["folder"]+"/"+chosenVideoETSingle["filename"]+"."+chosenVideoETSingle["suffix"];
            ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,cutoutType::STANDARD);
            markAnomaly->setModal(true);
            markAnomaly->show();
            connect(markAnomaly,&QDialog::finished,[=](){
                checkSelectedCutout();
            });
        }
    }
    else if (QObject::sender() == ui->standardCutout && !ui->standardCutout->isChecked())
        standardLoaded = false;
    else if (QObject::sender() == ui->extraCutout && !extraLoaded) {
        if (ui->extraCutout->isChecked())
        {
            QString fullPath = chosenVideoETSingle["folder"]+"/"+chosenVideoETSingle["filename"]+"."+chosenVideoETSingle["suffix"];
            ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,cutoutType::EXTRA);
            markAnomaly->setModal(true);
            markAnomaly->show();
            connect(markAnomaly,&QDialog::finished,[=](){
                checkSelectedCutout();
            });
        }
    }
    else if (QObject::sender() == ui->extraCutout && !ui->extraCutout->isChecked()) {
        extraLoaded = false;
        if (ui->standardCutout->isChecked()) {
            ui->standardCutout->setChecked(false);
        }
    }
}

void SingleVideoET::checkSelectedCutout() {

    if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoETSingle["filename"])) {
        QRect __extra = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoETSingle["filename"],"extra").toRect();
        if (__extra.isNull() && __extra.width() <= 0 && __extra.height() <= 0) {
            ui->extraCutout->setChecked(false);
            ui->standardCutout->setChecked(false);
            selectedCutout = cutoutType::NO_CUTOUT;
        }
        else {
            selectedCutout = cutoutType::EXTRA;
            standardLoaded = true;
            extraLoaded = true;
            ui->standardCutout->setChecked(true);
        }
    }
    else {
        ui->extraCutout->setChecked(false);
        ui->standardCutout->setChecked(false);
        selectedCutout = cutoutType::NO_CUTOUT;
    }

    if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoETSingle["filename"])) {
        QRect __standard = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoETSingle["filename"],"standard").toRect();
        if (__standard.isNull() && __standard.width() <= 0 && __standard.height() <= 0) {
            ui->standardCutout->setChecked(false);
            selectedCutout = cutoutType::NO_CUTOUT;
        }
        else {
            if (!extraLoaded)
                selectedCutout = cutoutType::STANDARD;
            standardLoaded = true;
        }
    }
    else {
        ui->standardCutout->setChecked(false);
        selectedCutout = cutoutType::NO_CUTOUT;
    }

}

void SingleVideoET::disableWidgets(){
    ui->chooseVideoPB->setEnabled(false);
    ui->chosenVideoLE->setEnabled(false);
    ui->standardCutout->setEnabled(false);
    ui->extraCutout->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->showGraphET->setEnabled(false);
    ui->areaMaximum->setEnabled(false);
    ui->iterationCount->setEnabled(false);
    ui->rotationAngle->setEnabled(false);
}

void SingleVideoET::enableWidgets(){
    ui->chooseVideoPB->setEnabled(true);
    ui->chosenVideoLE->setEnabled(true);
    ui->standardCutout->setEnabled(true);
    ui->extraCutout->setEnabled(true);
    ui->savePB->setEnabled(true);
    ui->showGraphET->setEnabled(true);
    ui->areaMaximum->setEnabled(true);
    ui->iterationCount->setEnabled(true);
    ui->rotationAngle->setEnabled(true);
}
