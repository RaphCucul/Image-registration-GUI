#include "main_program/frangi_detektor.h"
#include "ui_frangi_detektor.h"
#include "image_analysis/image_processing.h"
#include "image_analysis/frangi_utilization.h"
#include "util/files_folders_operations.h"
#include "util/vector_operations.h"
#include "dialogs/errordialog.h"
#include "dialogs/matviewer.h"
#include "dialogs/clickimageevent.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QString>
#include <QVector>
#include <QLineEdit>
#include <QFileDialog>
#include <QDebug>
#include <QJsonDocument>
#include <QTimer>

using cv::Point3d;
using cv::VideoCapture;
using cv::Mat;
using namespace cv;

Frangi_detektor::Frangi_detektor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Frangi_detektor)
{
    ui->setupUi(this);

    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    ui->fileToAnalyse->setText(tr("Choose file"));
    ui->frameNumber->setPlaceholderText(tr("Frame"));
    ui->chosenFile->setPlaceholderText(tr("Chosen file"));
    ui->RB_standard->setText(tr("Standard Frangi mode"));
    ui->RB_reverz->setText(tr("Revers Frangi mode"));

    ui->frameNumber->setEnabled(false);
    ui->Frangi_filtr->setEnabled(false);

    initWidgetHashes();

    localErrorDialogHandling[ui->saveParamGlob] = new ErrorDialog(ui->saveParamGlob);
    localErrorDialogHandling[ui->Frangi_filtr] = new ErrorDialog(ui->Frangi_filtr);
    localErrorDialogHandling[ui->chosenFile] = new ErrorDialog(ui->chosenFile);
    for (int index = 0; index < frangiParametersList.count()-1; index++){
        localErrorDialogHandling[spinBoxes[frangiParametersList.at(index)]] = new ErrorDialog(spinBoxes[frangiParametersList.at(index)]);
    }

    QObject::connect(ui->leftMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->rightMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->topMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->bottomMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->leftRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    QObject::connect(ui->rightRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    QObject::connect(ui->topRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    QObject::connect(ui->bottomRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    connect(ui->externalCutout,SIGNAL(stateChanged(int)),this,SLOT(openClickImageDialogue(int)));

    connect(ui->saveParamGlob,SIGNAL(clicked()),this,SLOT(onSaveFrangiData()));
    connect(ui->saveParamLocal,SIGNAL(clicked()),this,SLOT(onSaveFrangiData()));
    connect(ui->saveCutouts,SIGNAL(clicked()),this,SLOT(saveSelectedCutout()));
    connect(ui->saveReferential,SIGNAL(clicked()),this,SLOT(saveChosenReferentialFrame()));

    ui->globalParams->setEnabled(false);
    ui->videoParams->setEnabled(false);

    changeSaveButtonsStatus(false);
}

Frangi_detektor::~Frangi_detektor()
{
    delete ui;
}

void Frangi_detektor::setParametersToUI(){
    foreach (QString parameter,frangiParametersList) {
        setParameter(parameter,parameter);
    }

    if (SharedVariables::getSharedVariables()->getSpecificFrangiParameterWrapper(frangiType::VIDEO_SPECIFIC,
                                                                                 analyseChosenFile["filename"],
                                                                                 "mode") == 1.0){
        ui->RB_standard->setChecked(1);
        ui->RB_reverz->setChecked(0);}
    else{
        ui->RB_standard->setChecked(0);
        ui->RB_reverz->setChecked(1);
    }

    frangiMargins = SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(chosenFrangiType,
                                                                                   analyseChosenFile["filename"]);
    cutoutRatios = SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(chosenFrangiType,
                                                                                 analyseChosenFile["filename"]);
    int _counter = 0;
    foreach (QString parameter,MarginsRatiosList) {
        if (_counter < 4)
            marginSpinBoxes[parameter]->setValue(frangiMargins[parameter]);
        else
            ratioSpinBoxes[parameter]->setValue(cutoutRatios[parameter]);
        _counter++;
    }

    ui->chosenFile->setEnabled(true);
}

void Frangi_detektor::checkPaths()
{
    if (SharedVariables::getSharedVariables()->getPath("videosPath") == "")
        ui->chosenFile->setPlaceholderText(tr("Chosen video"));
    else
    {
        QString folder,filename,suffix;
        QStringList foundFiles;
        int foundCount;
        QString pathToVideos = SharedVariables::getSharedVariables()->getPath("videosPath");
        analyseFileNames(pathToVideos,foundFiles,foundCount,"avi");
        if (foundCount != 0)
        {
            QString completePath = pathToVideos+"/"+foundFiles.at(0);
            processFilePath(completePath,folder,filename,suffix);
            analyseChosenFile["folder"] = folder;
            analyseChosenFile["filename"] = filename;
            analyseChosenFile["suffix"] = suffix;
            ui->chosenFile->setText(filename);
        }
        else{
            localErrorDialogHandling[ui->chosenFile]->evaluate("left","hardError",3);
            localErrorDialogHandling[ui->chosenFile]->show(false);
        }
    }
    setParametersToUI();
    loading = false;
    connect(ui->globalParams,SIGNAL(toggled(bool)),this,SLOT(onFrangiSourceChosen()));
    connect(ui->videoParams,SIGNAL(toggled(bool)),this,SLOT(onFrangiSourceChosen()));
}

void Frangi_detektor::checkStartEndValues(){
    if (ui->sigma_start_DSB->value() >= ui->sigma_end_DSB->value()){
        if ((ui->sigma_start_DSB->value()+1) <= ui->sigma_end_DSB->maximum()){
            ui->sigma_end_DSB->setValue(ui->sigma_start_DSB->value()+1);
        }
        else{
            ui->sigma_end_DSB->setValue(ui->sigma_end_DSB->maximum()-1);
            ui->sigma_end_DSB->setValue(ui->sigma_end_DSB->maximum());
        }
    }
}

void Frangi_detektor::on_Frangi_filtr_clicked()
{
    if (SharedVariables::getSharedVariables()->getPath("parametersFrangiFiltr") == "") {
        localErrorDialogHandling[ui->Frangi_filtr]->evaluate("left","hardError",5);
        localErrorDialogHandling[ui->Frangi_filtr]->show(false);
    }
    else {
        QMap<QString,double> parametersForFrangi;
        foreach (QString parameter,frangiParametersList)
            parametersForFrangi[parameter] = (spinBoxes[parameter]->value());
        qDebug()<<"Frangi parameters: "<<parametersForFrangi;
        bool standard = ui->RB_standard->isChecked();
        int processingType;
        if (standard)
            processingType = 1;
        else
            processingType = 2;

        Mat chosenFrame_temp,chosenFrame;
        if (analyseChosenFile["suffix"] == "avi")
        {
            QString chosenFile = analyseChosenFile["folder"]+"/"+analyseChosenFile["filename"]+"."+analyseChosenFile["suffix"];
            QFile file(chosenFile);
            if (!file.exists()){
                localErrorDialogHandling[ui->Frangi_filtr]->evaluate("left","hardError",6);
                return;
            }
            VideoCapture cap = VideoCapture(chosenFile.toLocal8Bit().constData());
            cap.set(CV_CAP_PROP_POS_FRAMES,analyseFrame);
            cap.read(chosenFrame_temp);
        }
        if (previousExtraCutoutFound && ui->externalCutout->isChecked()) {
            chosenFrame_temp(extraCutout).copyTo(chosenFrame);
            chosenFrame_temp.release();
        }
        else if (ui->externalCutout->isChecked() && extraCutout.width == 0 && extraCutout.height == 0) {
            if (checkPresenceOfExtraCutout(analyseChosenFile["filename"],true)) {
                chosenFrame_temp(extraCutout).copyTo(chosenFrame);
                chosenFrame_temp.release();
            }
            else {
                chosenFrame_temp.copyTo(chosenFrame);
                chosenFrame_temp.release();
            }
        }
        else if (extraCutout.width != 0 && extraCutout.height != 0) {
            chosenFrame_temp(extraCutout).copyTo(chosenFrame);
            chosenFrame_temp.release();
        }
        else {
            chosenFrame_temp.copyTo(chosenFrame);
            chosenFrame_temp.release();
        }
        transformMatTypeTo8C3(chosenFrame);
        Point3d pt_temp(0.0,0.0,0.0);
        emit calculationStarted();
        cv::Point3d detectedFrangi = frangi_analysis(chosenFrame,
                                                     processingType,1,1,tr("Frangi of chosen"),1,pt_temp,
                                                     parametersForFrangi,
                                                     SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(chosenFrangiType,
                                                                                                                    analyseChosenFile["filename"]));
        SharedVariables::getSharedVariables()->setFrangiMaximumWrapper(chosenFrangiType,analyseChosenFile["filename"],detectedFrangi);
        showStandardCutout(chosenFrame);
        ui->saveReferential->setEnabled(true);
        emit calculationStopped();
    }
}

void Frangi_detektor::showStandardCutout(Mat &i_chosenFrame){
    cv::Point3d frangi_point = SharedVariables::getSharedVariables()->getFrangiMaximumWrapper(chosenFrangiType,
                                                                                              analyseChosenFile["filename"]);
    QMap<QString,double> frangiRatios = SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(chosenFrangiType,
                                                                                                      analyseChosenFile["filename"]);
    cv::Rect cutoutStandard;
    cutoutStandard.x = int(round(frangi_point.x-frangiRatios["left_r"]*(frangi_point.x)));
    cutoutStandard.y = int(round(frangi_point.y-frangiRatios["top_r"]*frangi_point.y));
    int rowTo = int(round(frangi_point.y+frangiRatios["bottom_r"]*(i_chosenFrame.rows - frangi_point.y)));
    int columnTo = int(round(frangi_point.x+frangiRatios["right_r"]*(i_chosenFrame.cols - frangi_point.x)));
    cutoutStandard.width = columnTo-cutoutStandard.x;
    cutoutStandard.height = rowTo - cutoutStandard.y;

    cv::rectangle(i_chosenFrame, cutoutStandard, Scalar(255), 1, 8, 0);
    MatViewer *viewer = new MatViewer(i_chosenFrame,"Chosen frame with standard cutout area");
    viewer->open();
}

void Frangi_detektor::on_frameNumber_textChanged(const QString &arg1)
{
    QString fullPath = analyseChosenFile["folder"]+"/"+analyseChosenFile["filename"]+"."+analyseChosenFile["suffix"];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    bool checkInput;
    int frameNumber = ui->frameNumber->text().toInt(&checkInput)-1;
    if (arg1.toInt() <= 0 || arg1.toInt() > frameCount || !checkInput){
        ui->frameNumber->setStyleSheet("color: #FF0000");
        //readyToCalculate = false;
        changeStatus(false);
    }
    else
    {
        ui->frameNumber->setStyleSheet("color: #339933");
        analyseFrame = frameNumber-1;
        //readyToCalculate = true;
        changeStatus(true);
    }
}

bool Frangi_detektor::checkPresenceOfExtraCutout(QString i_filename, bool videoInformationOnly) {
    previousExtraCutoutFound = false;
    QVector<int> unusedVariable;
    // first option - video was analysed previously -> the information can be found in the dat file
    if (checkAndLoadData("extra",i_filename,unusedVariable) && !videoInformationOnly) {
        if (unusedVariable.length() > 2) {
            if (unusedVariable[2] > 0 && unusedVariable[3] > 0) {
                previousExtraCutoutFound = true;
                extraCutout = convertVector2Rect(unusedVariable);
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else {
        // it is still possible that the requested data will be in the video information
        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(i_filename)) {
            QRect _pom = SharedVariables::getSharedVariables()->getVideoInformation(i_filename,"extra").toRect();
            extraCutout = convertQRectToRect(_pom);
            if (extraCutout.width > 0 && extraCutout.height > 0) {
                previousExtraCutoutFound = true;
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
}

void Frangi_detektor::on_fileToAnalyse_clicked()
{
    QString videoProFrangiFiltr = QFileDialog::getOpenFileName(this,
                                                               tr("Choose frame for Frangi filter analysis"), SharedVariables::getSharedVariables()->getPath("videosPath"),"(*.avi);;All files (*)");
    QString folder,filename,suffix;
    processFilePath(videoProFrangiFiltr,folder,filename,suffix);
    if (suffix == "avi"){
        analyseChosenFile["folder"] = folder;
        analyseChosenFile["filename"] = filename;
        analyseChosenFile["suffix"] = suffix;

        this->ui->frameNumber->setEnabled(true);
        ui->chosenFile->setText(analyseChosenFile["filename"]);
        ui->chosenFile->setStyleSheet("color: #339933");
        bool videoFrangiParametersLoaded = loadFrangiParametersForVideo(chosenFrangiType);
        if (!videoFrangiParametersLoaded) {
            setParametersToUI();
            ui->globalParams->setEnabled(false);
            ui->videoParams->setEnabled(false);
            ui->globalParams->setChecked(true);
        }
        else {
            ui->globalParams->setEnabled(true);
            ui->videoParams->setEnabled(true);
            ui->videoParams->setChecked(true);
        }
        if (checkPresenceOfExtraCutout(filename,false)) {
            ui->externalCutout->setCheckState(Qt::CheckState::Checked);
        }
        else
            ui->externalCutout->setCheckState(Qt::CheckState::Unchecked);
    }
    else {
        // no other video format is supported
        changeStatus(false);
    }
}

void Frangi_detektor::on_chosenFile_textChanged(const QString &arg1)
{
    QString fullPath = analyseChosenFile["folder"]+"/"+arg1+"."+analyseChosenFile["suffix"];
    QFile file(fullPath);
    if (!file.exists()){
        ui->chosenFile->setStyleSheet("color: #FF0000");
        ui->frameNumber->setText("");
        ui->frameNumber->setEnabled(false);
        changeStatus(false);
    }
    else {
        actualVideo = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        ui->chosenFile->setStyleSheet("color: #339933");
        analyseChosenFile["filename"] = arg1;
        ui->frameNumber->setEnabled(true);
        bool videoFrangiParametersLoaded = loadFrangiParametersForVideo(chosenFrangiType);
        if (!videoFrangiParametersLoaded) {
            setParametersToUI();
            ui->globalParams->setEnabled(false);
            ui->videoParams->setEnabled(false);
            ui->globalParams->setChecked(true);
        }
        else {
            ui->globalParams->setEnabled(true);
            ui->videoParams->setEnabled(true);
            ui->videoParams->setChecked(true);
        }
        if (checkPresenceOfExtraCutout(arg1,false))
            ui->externalCutout->setCheckState(Qt::CheckState::Checked);
        else
            ui->externalCutout->setCheckState(Qt::CheckState::Unchecked);
    }

}

bool Frangi_detektor::loadFrangiParametersForVideo(frangiType i_type) {
    QMap<QString,QVariant> receivedParameters;
    if (SharedVariables::getSharedVariables()->processVideoFrangiParameters(ui->chosenFile->text(),receivedParameters)) {
        if (i_type == frangiType::VIDEO_SPECIFIC) {
            foreach (QString name, MarginsRatiosList) {
                if (name.contains("_m"))
                    marginSpinBoxes[name]->setValue(receivedParameters[name].toInt());
                else
                    ratioSpinBoxes[name]->setValue(receivedParameters[name].toDouble());
            }
            foreach (QString name,frangiParametersList) {
                if (name != "mode")
                    spinBoxes[name]->setValue(receivedParameters[name].toDouble());
                else {
                    if (receivedParameters[name].toDouble() == 1.0)
                        ui->RB_standard->setChecked(true);
                    else
                        ui->RB_reverz->setChecked(true);
                }
            }
        }
        return true;
    }
    else
        return false;
}

void Frangi_detektor::onFrangiSourceChosen() {
    if (QObject::sender() == ui->globalParams) {
        chosenFrangiType = frangiType::GLOBAL;
        setParametersToUI();        
    }
    else {
        chosenFrangiType = frangiType::VIDEO_SPECIFIC;
        loadFrangiParametersForVideo(chosenFrangiType);
    }
}

void Frangi_detektor::openClickImageDialogue(int checked) {
    if (checked == Qt::CheckState::Checked && !previousExtraCutoutFound) {
        QString fullPath = analyseChosenFile["folder"]+"/"+analyseChosenFile["filename"]+"."+analyseChosenFile["suffix"];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,ui->frameNumber->text().toInt(),cutoutType::EXTRA,nullptr);
        markAnomaly->setModal(true);
        markAnomaly->show();
        connect(markAnomaly,SIGNAL(finished(int)),this,SLOT(checkSelectedCutout()));
    }
    else
        previousExtraCutoutFound = false;
}

void Frangi_detektor::checkSelectedCutout() {
    if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(analyseChosenFile["filename"])) {
        QRect __extra = SharedVariables::getSharedVariables()->getVideoInformation(analyseChosenFile["filename"],"extra").toRect();
        if (!__extra.isNull())
            ui->saveCutouts->setEnabled(true);
        else
            ui->externalCutout->setCheckState(Qt::CheckState::Unchecked);
    }
    else {
        ui->externalCutout->setCheckState(Qt::CheckState::Unchecked);
    }
}

void Frangi_detektor::setParameter(QString i_doubleSpinboxName, QString parameter) {
    spinBoxes[i_doubleSpinboxName]->setValue(SharedVariables::getSharedVariables()->getSpecificFrangiParameterWrapper(chosenFrangiType,
                                                                                                                      analyseChosenFile["filename"],
                                                                                                                      parameter));
}

void Frangi_detektor::setParameter(QString i_doubleSpinboxName, double i_value) {
    spinBoxes[i_doubleSpinboxName]->setValue(i_value);
}

void Frangi_detektor::getParameter(frangiType i_type, QString i_parameterName) {
    if (spinBoxes[i_parameterName]->value() != 0.0){
        SharedVariables::getSharedVariables()->setSpecificFrangiParameterWrapper(i_type,
                                                                                 spinBoxes[i_parameterName]->value(),
                                                                                 analyseChosenFile["filename"],
                                                                                 i_parameterName);
    }
    else{
        localErrorDialogHandling[ui->chosenFile]->evaluate("left","hardError",5);
        localErrorDialogHandling[ui->chosenFile]->show(false);
    }
}

void Frangi_detektor::setMargin(QString i_marginName, int i_value) {
    marginSpinBoxes[i_marginName]->setValue(i_value);
}

void Frangi_detektor::getMargin(frangiType i_type, QString i_marginName) {
    SharedVariables::getSharedVariables()->setSpecificFrangiMarginWrapper(i_type,
                                                                          i_marginName,
                                                                          analyseChosenFile["filename"],
                                                                          marginSpinBoxes[i_marginName]->value());
}

void Frangi_detektor::setRatio(QString i_ratioName, double i_value) {
    ratioSpinBoxes[i_ratioName]->setValue(i_value);
}

void Frangi_detektor::getRatio(frangiType i_type, QString i_ratioName) {
    SharedVariables::getSharedVariables()->setSpecificFrangiRatioWrapper(i_type,
                                                                         i_ratioName,
                                                                         analyseChosenFile["filename"],
                                                                         ratioSpinBoxes[i_ratioName]->value());
}

void Frangi_detektor::saveSelectedCutout() {
    QJsonDocument document;
    QJsonObject object;
    QString path = SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+analyseChosenFile["filename"]+".dat";
    QFile datFile(path);
    if (datFile.exists()) {
        object = readJson(datFile);
    }
    QRect _p = SharedVariables::getSharedVariables()->getVideoInformation(analyseChosenFile["filename"],"extra").toRect();
    cv::Rect _pCV = convertQRectToRect(_p);
    QVector<int> _pV = convertRect2Vector(_pCV);
    QJsonArray _pA = vector2array(_pV);
    object["extra"] = _pA;
    document.setObject(object);
    QString documentString = document.toJson();
    QFile writer;
    writer.setFileName(path);
    writer.open(QIODevice::WriteOnly);
    writer.write(documentString.toLocal8Bit());
    writer.close();
}

void Frangi_detektor::saveChosenReferentialFrame() {
    QJsonDocument document;
    QJsonObject object;
    QString path = SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+analyseChosenFile["filename"]+".dat";
    QFile datFile(path);
    if (datFile.exists()) {
        object = readJson(datFile);
    }

    if (object.contains("evaluation")) {
        QJsonArray existingEvaluation = object["evaluation"].toArray();
        QVector<int> existingEvaluationV = array2vector<int>(existingEvaluation);
        int existingReference = findReferenceFrame(existingEvaluationV);
        if (existingReference != analyseFrame) {
            existingEvaluationV[existingReference] = 0;
            existingEvaluationV[analyseFrame] = 2;
        }
        object["evaluation"] = vector2array(existingEvaluationV);
    }
    else {
        QVector<int> newEvaluation(actualVideo.get(CV_CAP_PROP_FRAME_COUNT),0);
        newEvaluation[analyseFrame] = 2;
        object["evaluation"] = vector2array(newEvaluation);
    }

    QStringList coordinates = {"FrangiX","FrangiY"};
    cv::Point3d frangiCoordinates = SharedVariables::getSharedVariables()->getFrangiMaximumWrapper(chosenFrangiType,analyseChosenFile["filename"]);
    foreach (QString coordinate,coordinates) {
        if (object.contains(coordinate)) {
            QJsonArray existingFrangi = object[coordinate].toArray();
            QVector<double> existingFrangiV = array2vector<double>(existingFrangi);
            existingFrangiV[analyseFrame] = coordinate=="FrangiX" ? frangiCoordinates.x : frangiCoordinates.y;
            object[coordinate] = vector2array(existingFrangiV);
        }
        else {
            QVector<double> newDataVector(actualVideo.get(CV_CAP_PROP_FRAME_COUNT),0.0);
            newDataVector[analyseFrame] = coordinate=="FrangiX" ? frangiCoordinates.x : frangiCoordinates.y;
            object[coordinate] = vector2array(newDataVector);
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

void Frangi_detektor::onSaveFrangiData() {
    frangiType saveType;
    int _glPar = -1;
    if (QObject::sender() == ui->saveParamGlob) {
        saveType = frangiType::GLOBAL;
        _glPar = 5;
    }
    else
        saveType = frangiType::VIDEO_SPECIFIC;

    int type = ui->RB_standard->isChecked() ? 1.0 : 0.0;
    SharedVariables::getSharedVariables()->setSpecificFrangiParameterWrapper(saveType,
                                                                             type,
                                                                             analyseChosenFile["filename"],
                                                                             "mode",_glPar);
    foreach (QString parameter,frangiParametersList) {
        if (parameter != "mode") {
            getParameter(saveType,parameter);
        }
    }
    foreach (QString parameter,MarginsRatiosList) {
        if (parameter.contains("_m"))
            getMargin(saveType,parameter);
        else
            getRatio(saveType,parameter);
    }
    SharedVariables::getSharedVariables()->saveFrangiParametersWrapper(saveType,analyseChosenFile["filename"]);

    if (saveType == frangiType::VIDEO_SPECIFIC) {
        ui->videoParams->setChecked(true);
        ui->globalParams->setEnabled(true);
        ui->videoParams->setEnabled(true);
    }

}

void Frangi_detektor::processMargins(int i_margin){
    if (!loading){
        if (QObject::sender() == ui->leftMargin)
            frangiMargins["left_m"] = i_margin;
        else if (QObject::sender() == ui->rightMargin)
            frangiMargins["right_m"] = i_margin;
        else if (QObject::sender() == ui->topMargin)
            frangiMargins["top_m"] = i_margin;
        else if (QObject::sender() == ui->bottomMargin)
            frangiMargins["bottom_m"] = i_margin;

        SharedVariables::getSharedVariables()->setFrangiMarginsWrapper(chosenFrangiType,frangiMargins,analyseChosenFile["filename"]);
    }
}

void Frangi_detektor::processRatio(double i_ratio){
    if (!loading){
        if (QObject::sender() == ui->leftRatio)
            cutoutRatios["left_r"] = i_ratio;
        else if (QObject::sender() == ui->rightRatio)
            cutoutRatios["right_r"] = i_ratio;
        else if (QObject::sender() == ui->topRatio)
            cutoutRatios["top_r"] = i_ratio;
        else if (QObject::sender() == ui->bottomRatio)
            cutoutRatios["bottom_r"] = i_ratio;

        SharedVariables::getSharedVariables()->setFrangiRatiosWrapper(chosenFrangiType,cutoutRatios,analyseChosenFile["filename"]);
    }
}

void Frangi_detektor::initWidgetHashes(){
    spinBoxes["sigma_start"] = ui->sigma_start_DSB;
    spinBoxes["sigma_end"] = ui->sigma_end_DSB;
    spinBoxes["sigma_step"] = ui->sigma_step_DSB;
    spinBoxes["beta_one"] = ui->beta_one_DSB;
    spinBoxes["beta_two"] = ui->beta_two_DSB;

    marginSpinBoxes["top_m"] = ui->topMargin;
    marginSpinBoxes["bottom_m"] = ui->bottomMargin;
    marginSpinBoxes["left_m"] = ui->leftMargin;
    marginSpinBoxes["right_m"] = ui->rightMargin;

    ratioSpinBoxes["top_r"] = ui->topRatio;
    ratioSpinBoxes["bottom_r"] = ui->bottomRatio;
    ratioSpinBoxes["left_r"] = ui->leftRatio;
    ratioSpinBoxes["right_r"] = ui->rightRatio;
}

void Frangi_detektor::changeStatus(bool status) {
    ui->saveParamGlob->setEnabled(status);
    ui->saveParamLocal->setEnabled(status);
    ui->Frangi_filtr->setEnabled(status);
    ui->externalCutout->setEnabled(status);
}

void Frangi_detektor::changeSaveButtonsStatus(bool status) {
    ui->saveCutouts->setEnabled(status);
    ui->saveReferential->setEnabled(status);
}
