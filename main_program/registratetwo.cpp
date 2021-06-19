#include "main_program/registratetwo.h"
#include "ui_registratetwo.h"
#include "image_analysis/image_processing.h"
#include "image_analysis/frangi_utilization.h"
#include "image_analysis/entropy.h"
#include "registration/multiPOC_Ai1.h"
#include "registration/registration_correction.h"
#include "registration/phase_correlation_function.h"
#include "dialogs/errordialog.h"
#include "dialogs/clickimageevent.h"
#include "main_program/frangi_detektor.h"
#include "main_program/registrationresult.h"
#include "shared_staff/sharedvariables.h"
#include "util/files_folders_operations.h"
#include "util/vector_operations.h"

#include <opencv2/opencv.hpp>

#include <QFileDialog>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QtGui>
#include <QPixmap>
#include <QEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QSpacerItem>
#include <array>
#include <typeinfo>

RegistrateTwo::RegistrateTwo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegistrateTwo)
{
    ui->setupUi(this);
    ui->selectInput->addItem(tr("Choose two videoframes"));
    ui->selectInput->addItem(tr("Choose two images"));
    ui->areaMaximumLabel->setText(tr("Size of calculation area"));
    ui->rotationAngleLabel->setText(tr("Maximal tolerated rotation angle"));
    ui->iterationCountLabel->setText(tr("Number of iteration of algorithm"));
    ui->entropy_referential->setText(tr("Reference entropy"));
    ui->entropie_translated->setText(tr("Translated entropy"));
    ui->tennengrad_referential->setText(tr("Tennengrad reference"));
    ui->tennengrad_translated->setText(tr("Tennengrad translated"));
    ui->standardCutout->setText(tr("Modify standard cutout"));
    ui->extraCutout->setText(tr("Modify extra cutout"));
    ui->registrateTwo->setText(tr("Registrate"));
    ui->registrateTwo->setEnabled(false);

    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    ui->areaMaximum->setPlaceholderText("0 - 20");
    ui->areaMaximum->setEnabled(false);
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->rotationAngle->setEnabled(false);
    ui->iterationCount->setPlaceholderText("1 - Inf; -1~automatic settings");
    ui->iterationCount->setEnabled(false);
    ui->standardCutout->setChecked(false);
    ui->extraCutout->setChecked(false);
    ui->standardCutout->setEnabled(false);
    ui->extraCutout->setEnabled(false);

    //frangiMaximumCoords = SharedVariables::getSharedVariables()->getFrangiMaximum();
    connect(ui->extraCutout,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    connect(ui->standardCutout,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));

    initChoiceOneInnerWidgets();
    placeChoiceOneWidgets();
    whatIsAnalysed=chosenSource::VIDEO;
    firstChoiceInitialised = true;

    referenceNoLE->setEnabled(false);
    translatedNoLE->setEnabled(false);

    QObject::connect(this,SIGNAL(checkRegistrationPass()),this,SLOT(evaluateCorrectValues()));
    localErrorDialogHandling[ui->registrateTwo] = new ErrorDialog(ui->registrateTwo);
    localErrorDialogHandling[ui->standardCutout] = new ErrorDialog(ui->standardCutout);
    localErrorDialogHandling[ui->extraCutout] = new ErrorDialog(ui->extraCutout);
}

RegistrateTwo::~RegistrateTwo()
{
    delete ui;
}

void RegistrateTwo::initChoiceOneInnerWidgets(){
    chosenVideoLE = new QLineEdit();
    chosenVideoPB = new QPushButton();
    referenceNoLE = new QLineEdit();
    translatedNoLE = new QLineEdit();
    horizontalSpacer1 = new QSpacerItem(20,20);

    chosenVideoLE->setPlaceholderText(tr("Chosen video"));
    chosenVideoLE->setMinimumWidth(150);
    chosenVideoLE->setMaximumWidth(170);
    chosenVideoLE->setMinimumHeight(20);

    chosenVideoPB->setText(tr("Choose video"));
    chosenVideoPB->setMinimumWidth(70);
    chosenVideoPB->setMinimumHeight(23);
    chosenVideoPB->setMaximumWidth(75);

    referenceNoLE->setPlaceholderText("Ref");
    referenceNoLE->setMinimumWidth(20);
    referenceNoLE->setMinimumHeight(20);
    referenceNoLE->setMaximumWidth(35);
    referenceNoLE->setMaximumHeight(20);

    translatedNoLE->setPlaceholderText(tr("Shifted"));
    translatedNoLE->setMinimumWidth(20);
    translatedNoLE->setMinimumHeight(20);
    translatedNoLE->setMaximumWidth(50);
    translatedNoLE->setMaximumHeight(20);
}

void RegistrateTwo::initChoiceTwoInnerWidgets(){
    horizontalSpacer2 = new QSpacerItem(5,20);
    referenceImgLE = new QLineEdit();
    translatedImgLE = new QLineEdit();
    chooseReferentialImagePB = new QPushButton();
    chooseTranslatedImagePB = new QPushButton();

    referenceImgLE->setMinimumWidth(90);
    referenceImgLE->setMinimumHeight(20);
    referenceImgLE->setPlaceholderText("Ref");
    referenceImgLE->setMaximumWidth(200);
    referenceImgLE->setMaximumHeight(20);

    translatedImgLE->setMinimumWidth(90);
    translatedImgLE->setMinimumHeight(20);
    translatedImgLE->setPlaceholderText(tr("Shifted"));
    translatedImgLE->setMaximumWidth(200);
    translatedImgLE->setMaximumHeight(20);

    chooseReferentialImagePB->setText(tr("Chooce referential"));
    chooseReferentialImagePB->setMinimumWidth(90);
    chooseReferentialImagePB->setMinimumHeight(20);
    chooseReferentialImagePB->setMaximumWidth(90);
    chooseReferentialImagePB->setMaximumHeight(20);
    chooseTranslatedImagePB->setText(tr("Choose shifted"));
    chooseTranslatedImagePB->setMinimumWidth(90);
    chooseTranslatedImagePB->setMinimumHeight(20);
    chooseTranslatedImagePB->setMaximumWidth(90);
    chooseTranslatedImagePB->setMaximumHeight(20);
}

void RegistrateTwo::placeChoiceOneWidgets(){    
    ui->analysisOption->addWidget(chosenVideoLE,0,0);
    ui->analysisOption->addWidget(chosenVideoPB,0,1);
    ui->analysisOption->addWidget(referenceNoLE,0,2);
    ui->analysisOption->addWidget(translatedNoLE,0,3);
    ui->analysisOption->addItem(horizontalSpacer1,0,4);

    localErrorDialogHandling[referenceNoLE] = new ErrorDialog(referenceNoLE);

    connect(chosenVideoPB,SIGNAL(clicked()),this,SLOT(chosenVideoPB_clicked()));
    connect(chosenVideoLE,SIGNAL(textChanged(const QString &)),this,SLOT(Slot_VideoLE_textChanged(const QString &)));
    connect(referenceNoLE,SIGNAL(textChanged(const QString &)),this,SLOT(ReferenceLE_textChanged(const QString &)));
    connect(translatedNoLE,SIGNAL(textChanged(const QString &)),this,SLOT(TranslatedLE_textChanged(const QString &)));
}

void RegistrateTwo::placeChoiceTwoWidgets(){
    ui->analysisOption->addWidget(referenceImgLE,0,0);
    ui->analysisOption->addWidget(chooseReferentialImagePB,0,1);
    ui->analysisOption->addWidget(translatedImgLE,0,2);
    ui->analysisOption->addWidget(chooseTranslatedImagePB,0,3);
    ui->analysisOption->addItem(horizontalSpacer2,0,4);

    connect(chooseReferentialImagePB,SIGNAL(clicked()),this,SLOT(chosenReferenceImgPB_clicked()));
    connect(chooseTranslatedImagePB,SIGNAL(clicked()),this,SLOT(chosenTranslatedImgPB_clicked()));
    connect(referenceImgLE,SIGNAL(textChanged(const QString &)),this,SLOT(ReferenceImgLE_textChanged(const QString &)));
    connect(translatedImgLE,SIGNAL(textChanged(const QString &)),this,SLOT(TranslatedImgLE_textChanged(const QString &)));
}

void RegistrateTwo::analyseAndSave(QString i_analysedFolder, QMap<QString,QString> &i_whereToSave){
    QString folder,filename,suffix;
    processFilePath(i_analysedFolder,folder,filename,suffix);
    i_whereToSave["folder"] = folder;
    i_whereToSave["filename"] = filename;
    i_whereToSave["suffix"] = suffix;
}

bool RegistrateTwo::evaluateVideoImageInput(QString i_path, QString i_method){
    if (i_method == "video"){
        if (!i_path.isEmpty()) {
            cap = cv::VideoCapture(i_path.toLocal8Bit().constData());
            if (!cap.isOpened())
            {
                chosenVideoLE->setStyleSheet("color: #FF0000");
                videoCorrect = false;
                referenceNoLE->setText("");
                translatedNoLE->setText("");
                referenceNoLE->setEnabled(false);
                translatedNoLE->setEnabled(false);
                ui->areaMaximum->setEnabled(false);
                ui->rotationAngle->setEnabled(false);
                ui->iterationCount->setEnabled(false);
                cap.release();
                return false;
            }
        }
        if (cap.isOpened()) {
            chosenVideoLE->setStyleSheet("color: #339900");
            videoCorrect = true;
            referenceNoLE->setEnabled(true);
            translatedNoLE->setEnabled(true);
            analyseAndSave(i_path,chosenVideoAnalysis);
            QMap<QString,QVariant> _pom; // unused, just
            if (SharedVariables::getSharedVariables()->processVideoFrangiParameters(chosenVideoAnalysis["filename"],_pom))
                typeOfFrangi = frangiType::VIDEO_SPECIFIC;
            else
                typeOfFrangi = frangiType::GLOBAL;

            QVector<int> standardCutout,extraCutout;
            if (checkAndLoadData("standard",chosenVideoAnalysis["filename"],standardCutout)) {
                if (standardCutout.length() == 4 && standardCutout[2] > 0 && standardCutout[3] > 0) {
                    cv::Rect _p0 = convertVector2Rect(standardCutout);
                    QRect _p1 = convertRectToQRect(_p0);
                    SharedVariables::getSharedVariables()->setVideoInformation(chosenVideoAnalysis["filename"],"standard",_p1);
                    standardLoaded = true;
                    ui->standardCutout->setChecked(true);
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

            if (checkAndLoadData("extra",chosenVideoAnalysis["filename"],extraCutout)) {
                if (extraCutout.length() == 4 && extraCutout[2] > 0 && extraCutout[3] > 0) {
                    cv::Rect _p0 = convertVector2Rect(extraCutout);
                    QRect _p1 = convertRectToQRect(_p0);
                    SharedVariables::getSharedVariables()->setVideoInformation(chosenVideoAnalysis["filename"],"extra",_p1);
                    extraLoaded = true;
                    ui->extraCutout->setChecked(true);
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

            return true;
        }
        else {
            chosenVideoLE->setStyleSheet("color: #FF0000");
            videoCorrect = false;
            referenceNoLE->setText("");
            translatedNoLE->setText("");
            referenceNoLE->setEnabled(false);
            translatedNoLE->setEnabled(false);
            ui->areaMaximum->setEnabled(false);
            ui->rotationAngle->setEnabled(false);
            ui->iterationCount->setEnabled(false);
            cap.release();
            return false;
        }
    }
    else if (i_method == "referentialImage"){
        referentialImg = cv::imread(i_path.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
        if (referentialImg.empty())
        {
            referenceImgLE->setStyleSheet("color: #FF0000");
            referentialImgCorrect = false;
            ui->areaMaximum->setEnabled(false);
            ui->rotationAngle->setEnabled(false);
            ui->iterationCount->setEnabled(false);
            referentialImg.release();
            return false;
        }
        else
        {
            referenceImgLE->setStyleSheet("color: #339900");
            referentialImgCorrect = true;
            return true;
        }        
    }
    else if (i_method == "translatedImage"){
        translatedImg = cv::imread(i_path.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
        if (translatedImg.empty())
        {
            translatedImgLE->setStyleSheet("color: #FF0000");
            translatedImgCorrect = false;
            translatedImg.release();
            return false;
        }
        else
        {
            translatedImgLE->setStyleSheet("color: #339900");
            translatedImgCorrect = true;
            return true;
        }        
    }
    else
        return false;
}

void RegistrateTwo::evaluateCorrectValues(){
    if (areaMaximumCorrect && angleCorrect && iterationCorrect){
        ui->registrateTwo->setEnabled(true);
        ui->standardCutout->setEnabled(true);
        ui->extraCutout->setEnabled(true);
    }
    else {
        ui->registrateTwo->setEnabled(false);
        ui->registrateTwo->setEnabled(false);
        ui->standardCutout->setEnabled(false);
    }
}

void RegistrateTwo::checkPaths(){
    if (whatIsAnalysed == chosenSource::VIDEO){

        if (SharedVariables::getSharedVariables()->getPath("videosPath") == "")
            chosenVideoLE->setPlaceholderText(tr("Chosen video"));
        else{
            analyseAndSaveFirst(SharedVariables::getSharedVariables()->getPath("videosPath"),chosenVideoAnalysis);
            chosenVideoLE->setText(chosenVideoAnalysis["filename"]);
        }
    }
}

void RegistrateTwo::on_selectInput_activated(int index)
{
    if (index == 2)
    {
        if (whatIsAnalysed != chosenSource::NOTHING)
            clearLayout(ui->analysisOption);

        whatIsAnalysed = chosenSource::NOTHING;
    }
    if (index == 0)
    {
        if (whatIsAnalysed != chosenSource::NOTHING)
            clearLayout(ui->analysisOption);
        if (whatIsAnalysed != chosenSource::VIDEO)
            initChoiceOneInnerWidgets();

        referentialImg.release();
        translatedImg.release();
        whatIsAnalysed = chosenSource::VIDEO;
        placeChoiceOneWidgets();
    }
    else if (index == 1)
    {
        if (whatIsAnalysed != chosenSource::NOTHING)
            clearLayout(ui->analysisOption);
        if (whatIsAnalysed != chosenSource::IMAGE)
            initChoiceTwoInnerWidgets();

        referentialImg.release();
        translatedImg.release();
        whatIsAnalysed = chosenSource::IMAGE;
        placeChoiceTwoWidgets();
        secondChoiceInitialised = true;
        typeOfFrangi = frangiType::GLOBAL;
        referentialNumber = -1;
        cap.release();
    }
}

void RegistrateTwo::clearLayout(QGridLayout *layout)
{
    int columnCount = layout->columnCount();
    int rowCount = layout->rowCount();
    for (int a = 1; a <= rowCount; a++)
    {
        for (int b = 1; b <= columnCount; b++)
        {
            QWidget* widget = layout->itemAtPosition(a-1,b-1)->widget();
            layout->removeWidget(widget);
            delete widget;
        }
    }
}

void RegistrateTwo::chosenVideoPB_clicked()
{
    QString pathToVideo = QFileDialog::getOpenFileName(this,tr("Choose video"), SharedVariables::getSharedVariables()->getPath("videosPath"),"*.avi;;;");
    if (pathToVideo != "") {
        analyseAndSave(pathToVideo,chosenVideoAnalysis);
        chosenVideoLE->setText(chosenVideoAnalysis["filename"]);
        evaluateVideoImageInput(pathToVideo,"video");
    }
}

void RegistrateTwo::chosenReferenceImgPB_clicked()
{
    QString referentialImagePath = QFileDialog::getOpenFileName(this,tr("Choose referential image"), "",tr("Images (*.bmp *.png *.jpg)"));
    if (referentialImagePath != "") {
        analyseAndSave(referentialImagePath,chosenReferentialImgAnalysis);
        referenceImgLE->setText(chosenReferentialImgAnalysis["filename"]);
        evaluateVideoImageInput(referentialImagePath,"referentialImage");
    }
}

void RegistrateTwo::chosenTranslatedImgPB_clicked()
{
    QString translatedImgPath = QFileDialog::getOpenFileName(this,tr("Choose translated image"), "",tr("Images (*.bmp *.png *.jpg)"));
    if (translatedImgPath != "") {
        analyseAndSave(translatedImgPath,chosenTranslatedImgAnalysis);
        translatedImgLE->setText(chosenTranslatedImgAnalysis["filename"]);
        evaluateVideoImageInput(translatedImgPath,"translatedImage");
    }
}

void RegistrateTwo::ReferenceImgLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenReferentialImgAnalysis["folder"]+"/"+arg1+"."+chosenReferentialImgAnalysis["suffix"];
    evaluateVideoImageInput("","referentialImage");
    if (referentialImgCorrect && translatedImgCorrect){
        ui->areaMaximum->setEnabled(true);
        ui->rotationAngle->setEnabled(true);
        ui->iterationCount->setEnabled(true);
    }
}

void RegistrateTwo::TranslatedImgLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenTranslatedImgAnalysis["folder"]+"/"+arg1+"."+chosenTranslatedImgAnalysis["suffix"];
    evaluateVideoImageInput("","translatedImage");
    if (referentialImgCorrect && translatedImgCorrect){
        ui->areaMaximum->setEnabled(true);
        ui->rotationAngle->setEnabled(true);
        ui->iterationCount->setEnabled(true);
    }
}

void RegistrateTwo::Slot_VideoLE_textChanged(const QString &s)
{
    QString fullPath = chosenVideoAnalysis["folder"]+"/"+s+"."+chosenVideoAnalysis["suffix"];
    if (evaluateVideoImageInput(fullPath,"video")){
        if (localErrorDialogHandling[referenceNoLE]->isEvaluated())
            localErrorDialogHandling[referenceNoLE]->hide();
    }
}

void RegistrateTwo::ReferenceLE_textChanged(const QString &arg1)
{
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int referenceFrameNo = arg1.toInt()-1;
    if (referenceFrameNo < 0 || referenceFrameNo > frameCount)
    {
        referenceNoLE->setStyleSheet("color: #FF0000");
        referenceCorrect = false;
        referentialNumber = -1;
        if (localErrorDialogHandling[referenceNoLE]->isEvaluated())
            localErrorDialogHandling[referenceNoLE]->hide();
    }
    else
    {
        referenceNoLE->setStyleSheet("color: #33aa00");
        referenceCorrect = true;
        referentialNumber = referenceFrameNo;
        if (referenceCorrect && translatedCorrect){
            ui->areaMaximum->setEnabled(true);
            ui->rotationAngle->setEnabled(true);
            ui->iterationCount->setEnabled(true);
        }
        if (!checkReferentialFrameExistence(SharedVariables::getSharedVariables()->getPath("datFilesPath"),
                                            chosenVideoAnalysis["filename"],referentialNumber)){
            if (!localErrorDialogHandling[referenceNoLE]->isEvaluated()){
                localErrorDialogHandling[referenceNoLE]->evaluate("left","info",4);
                localErrorDialogHandling[referenceNoLE]->show(false);
            }
        }
        else{
            if (localErrorDialogHandling[referenceNoLE]->isEvaluated())
                localErrorDialogHandling[referenceNoLE]->hide();
        }
    }
}

void RegistrateTwo::TranslatedLE_textChanged(const QString &arg1)
{
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int translatedFrameNo = arg1.toInt()-1;
    if (translatedFrameNo < 0 || translatedFrameNo > frameCount)
    {
        translatedNoLE->setStyleSheet("color: #FF0000");
        translatedCorrect = false;
        translatedNumber = -1;
    }
    else
    {
        translatedNoLE->setStyleSheet("color: #33aa00");
        translatedCorrect = true;
        translatedNumber = translatedFrameNo;
        if (referenceCorrect && translatedCorrect){
            ui->areaMaximum->setEnabled(true);
            ui->rotationAngle->setEnabled(true);
            ui->iterationCount->setEnabled(true);
        }
    }
}

void RegistrateTwo::checkInputNumber(double i_input, double i_lower, double i_upper, QLineEdit *i_editWidget, double &i_finalValue, bool &i_evaluation){

    if (i_input < i_lower || (i_input > i_upper && i_upper != 0.0)){
        i_editWidget->setStyleSheet("color: #FF0000");
        i_editWidget->setText("");
        i_finalValue = -99;
        i_evaluation = false;
    }
    else if (i_input < i_lower || (i_upper == 0.0 && i_input == 0.0)){
        i_editWidget->setStyleSheet("color: #FF0000");
        i_editWidget->setText("");
        i_finalValue = -99;
        i_evaluation = false;
    }
    else{
        i_editWidget->setStyleSheet("color: #33aa00");
        i_evaluation = true;
        i_finalValue = i_input;
        emit checkRegistrationPass();
    }

}

void RegistrateTwo::on_areaMaximum_editingFinished()
{
    bool ok;
    double inputNumber = ui->areaMaximum->text().toDouble(&ok);
    if (ok)
        checkInputNumber(inputNumber,1.0,20.0,ui->areaMaximum,areaMaximum,areaMaximumCorrect);
    else
        ui->areaMaximum->setText("");
}

void RegistrateTwo::on_rotationAngle_editingFinished()
{
    bool ok;
    double inputNumber = ui->rotationAngle->text().toDouble(&ok);
    if (ok)
        checkInputNumber(inputNumber,0.0,0.5,ui->rotationAngle,angle,angleCorrect);
    else
        ui->rotationAngle->setText("");
}

void RegistrateTwo::on_iterationCount_editingFinished()
{
    bool ok;
    double inputNumber = ui->iterationCount->text().toDouble(&ok);
    if (ok)
        checkInputNumber(inputNumber,-1.0,0.0,ui->iterationCount,iteration,iterationCorrect);
    else
        ui->iterationCount->setText("");
}

void RegistrateTwo::on_registrateTwo_clicked()
{
    emit calculationStarted();
    cv::Mat referentialImage_temp,referentialImage,translatedImage_temp,translatedImage;
    if (whatIsAnalysed == chosenSource::VIDEO) {
        if (!cap.isOpened())
            return;
        cap.set(CV_CAP_PROP_POS_FRAMES,1.0*referentialNumber);
        if (!cap.read(referentialImage_temp))
            return;
        cap.set(CV_CAP_PROP_POS_FRAMES,1.0*translatedNumber);
        if (!cap.read(translatedImage_temp))
            return;
    }
    else if (whatIsAnalysed == chosenSource::IMAGE) {
        referentialImg.copyTo(referentialImage_temp);
        translatedImg.copyTo(translatedImage_temp);
    }
    transformMatTypeTo8C3(referentialImage_temp);
    transformMatTypeTo8C3(translatedImage_temp);
    qDebug()<<"Analysing frames "<<referentialNumber<<" "<<translatedNumber;
    cv::Rect cutoutExtra(0,0,0,0);
    cv::Rect cutoutStandard(0,0,0,0);
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Point3d maximum_frangi(0.0,0.0,0.0);
    QPoint _MFR(0,0);

    bool standardReady = false, frangiReady = false;
    bool readyToObtainData = SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoAnalysis["filename"]);

    // before starting the registration process, it is necessary to check several thinks:
    // I. Is modified standard cutout present?
    if ((ui->standardCutout->isChecked() || ui->extraCutout->isChecked()) && readyToObtainData) {
        // if checked, videoInformation has to contain corresponding information
        // when extra cutout is selected, usually standard cutout is also available
        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoAnalysis["filename"])) {
            QRect _standardCutout = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"standard").toRect();
            if (!_standardCutout.isNull() && _standardCutout.width() > 0 && _standardCutout.height() > 0) {
                cutoutStandard = transform_QRect_to_CV_RECT(_standardCutout);
                standardReady = true;
            }
        }
    }
    // II. Is modified extra cutout present?
    if (ui->extraCutout->isChecked() && readyToObtainData) {
        QRect _extraCutout = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"extra").toRect();
        if (!_extraCutout.isNull() && _extraCutout.width() > 0 && _extraCutout.height() > 0) {
            cutoutExtra = transform_QRect_to_CV_RECT(_extraCutout);
            scaleChanged = true;
        }
        else
            scaleChanged = false;
    }
    else
        scaleChanged = false;
    // if scale has to be changed (<-extra cutout must be applied), it has to be done now
    if (scaleChanged) {
        referentialImage_temp(cutoutExtra).copyTo(referentialImage);
        referentialImage_temp.release();
        translatedImage_temp(cutoutExtra).copyTo(translatedImage);
        translatedImage_temp.release();
    }
    else {
        referentialImage_temp.copyTo(referentialImage);
        referentialImage_temp.release();
        translatedImage_temp.copyTo(translatedImage);
        translatedImage_temp.release();
    }
    referentialImg = referentialImage;
    // III. Was frangi maximum already estimated for the referential frame?
    if (readyToObtainData) {
        if (SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"frame").toInt() == referentialNumber) {
            QPoint _frangiCoordinates = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"frangi").toPoint();
            if (!_frangiCoordinates.isNull()) {
                maximum_frangi = transform_QPoint_to_CV_Point3d(_frangiCoordinates);
                frangiReady = true;
            }
        }
    }
    // IV. It may happen, that frangi is not in the videoInformation, but it can be still present in the dat file
    if (!frangiReady) {
        if (findReferentialFrameData(chosenVideoAnalysis["filename"],referentialNumber,_MFR)){
            maximum_frangi = transform_QPoint_to_CV_Point3d(_MFR);
            if (!standardReady) {
                cutoutStandard = calculateStandardCutout(maximum_frangi,
                                                         SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                                         referentialImage.rows,
                                                         referentialImage.cols);
                standardReady = true;
            }
        }
    }
    // V. Final check - if nothing was calculated previously, it is necessary to calculat it now
    if (!standardReady && !frangiReady) {
        // the referential frame was not preprocessed -> no anomaly is present in the frame
        // video was not analysed in the past -> no data about referential frame and frangi coordinates
        // ->
        // extraCutout is not expected, only standard cutout must be calculated from the newly calculated
        // frangi coordinates -> preprocessing the full registration
        if (!preprocessingCompleteRegistration(referentialImage,
                                               maximum_frangi,
                                               cutoutStandard,
                                               SharedVariables::getSharedVariables()->getFrangiParameterWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                               SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                               SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(typeOfFrangi,chosenVideoAnalysis["filename"])
                                               )){
            localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",10);
            localErrorDialogHandling[ui->registrateTwo]->show(false);
            return;
        }
    }

    double entropyTranslated,entropyReference;
    cv::Scalar tennengradTranslated,tennengradReference;
    if (!calculateParametersET(referentialImage,entropyReference,tennengradReference)){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",8);
        localErrorDialogHandling[ui->registrateTwo]->show(false);
        return;
    }
    if(!calculateParametersET(translatedImage,entropyTranslated,tennengradTranslated)){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",9);
        localErrorDialogHandling[ui->registrateTwo]->show(false);
        return;
    }
    ui->ER->setText(QString::number(entropyReference));
    ui->TR->setText(QString::number(tennengradReference[0]));
    ui->EM->setText(QString::number(entropyTranslated));
    ui->TM->setText(QString::number(tennengradTranslated[0]));

    //int rows = referentialImage.rows;
    //int cols = referentialImage.cols;

    qDebug()<<"Maximum frangi "<<maximum_frangi.x<<" "<<maximum_frangi.y;
    qDebug()<<"Standard cutout "<<cutoutStandard.width<<" "<<cutoutStandard.height;
    qDebug()<<"Extra cutout "<<cutoutExtra.width<<" "<<cutoutExtra.height;
    /// Beginning
    cv::Point3d pt3(0.0,0.0,0.0);
    double l_angle = 0.0;
    QVector<double> l_angleSum;
    QVector<double> fr_x;
    QVector<double> fr_y;
    QVector<double> fr_eukl;
    QVector<double> _pocX;
    QVector<double> _pocY;

    bool registrationSuccessfull = registrateBestFrames(cap,
                                                        referentialImage,
                                                        translatedImage,
                                                        maximum_frangi,
                                                        translatedNumber,
                                                        iteration,
                                                        areaMaximum,
                                                        l_angle,
                                                        cutoutExtra,
                                                        cutoutStandard,
                                                        scaleChanged,
                                                        SharedVariables::getSharedVariables()->getFrangiParameterWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                                        _pocX,_pocY,fr_x,fr_y,fr_eukl,
                                                        l_angleSum,
                                                        SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                                        whatIsAnalysed != chosenSource::VIDEO ? false : true);
    /// Konec
    if (!registrationSuccessfull){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",11);
        localErrorDialogHandling[ui->registrateTwo]->show(false);
        return;
    }
    else
    {
        qDebug()<<"translation after multiPOC "<<_pocX[0]<<" "<<_pocY[0];
        cv::Mat registratedFrame = cv::Mat::zeros(cv::Size(referentialImage.cols,referentialImage.rows),
                                                  CV_32FC3);
        cv::Mat translated_temp,translated;
        cv::Point3d translation(_pocX[0],_pocY[0],0.0);
        if (whatIsAnalysed == chosenSource::VIDEO) {
            cap.set(CV_CAP_PROP_POS_FRAMES,translatedNumber);
            if(!cap.read(translated_temp))
            {
                cap.release();
                return;
            }
        }

        if (scaleChanged) {
            translated_temp(cutoutExtra).copyTo(translated);
            translated_temp.release();
        }
        else {
            translated_temp.copyTo(translated);
            translated_temp.release();
        }
        translatedImg = translated;

        transformMatTypeTo8C3(translated);
        registratedFrame = frameTranslation(translated,translation,
                                            referentialImage.rows,referentialImage.cols);
        registratedFrame = frameRotation(registratedFrame,l_angleSum[0]);
        qDebug()<<"Size check before showing results";
        qDebug()<<referentialImage.rows<<" "<<referentialImage.cols;
        qDebug()<<registratedFrame.rows<<" "<<registratedFrame.cols;
        RegistrationResult *showResults = new RegistrationResult(referentialImage,registratedFrame);
        showResults->callTwo();
        showResults->setModal(true);
        showResults->show();
    }
    emit calculationStopped();
}

void RegistrateTwo::showDialog()
{
    // before any operation starts, it is necessary to make sure the frangi analysis function will have
    // all requested inputs
    QString fullPath;
    if (whatIsAnalysed == chosenSource::VIDEO) {
        fullPath = chosenVideoAnalysis["folder"]+"/"+chosenVideoAnalysis["filename"]+"."+chosenVideoAnalysis["suffix"];
    }
    else if (whatIsAnalysed == chosenSource::IMAGE) {
        fullPath = chosenReferentialImgAnalysis["folder"]+"/"+chosenReferentialImgAnalysis["filename"]+"."+chosenReferentialImgAnalysis["suffix"];
    }

    if (QObject::sender() == ui->standardCutout && !standardLoaded){
        if (ui->standardCutout->isChecked())
        {
            ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referentialNumber,cutoutType::STANDARD,false,whatIsAnalysed);
            markAnomaly->setModal(true);
            markAnomaly->show();
            connect(markAnomaly,&QDialog::finished,[=](){
                checkSelectedCutout(cutoutType::STANDARD);
            });
        }
    }
    else if (QObject::sender() == ui->standardCutout && !ui->standardCutout->isChecked())
        standardLoaded = false;
    else if (QObject::sender() == ui->extraCutout && !extraLoaded){
        if (ui->extraCutout->isChecked())
        {
            ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referentialNumber,cutoutType::EXTRA,false,whatIsAnalysed);
            markAnomaly->setModal(true);
            markAnomaly->show();
            connect(markAnomaly,&QDialog::finished,[=](){
                checkSelectedCutout(cutoutType::EXTRA);
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

void RegistrateTwo::checkSelectedCutout(cutoutType i_type) {
    if (i_type == cutoutType::EXTRA) {
        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoAnalysis["filename"])) {
            QRect __extra = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"extra").toRect();
            if (__extra.isNull() || (__extra.width() <= 0 && __extra.height() <= 0)) {
                ui->extraCutout->setChecked(false);
                if (!standardLoaded)
                    ui->standardCutout->setChecked(false);
            }
        }
        else {
            ui->extraCutout->setChecked(false);
            if (!standardLoaded)
                ui->standardCutout->setChecked(false);
        }
    }
    else {
        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoAnalysis["filename"])) {
            QRect __standard = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"standard").toRect();
            if (__standard.isNull() || (__standard.width() <= 0 && __standard.height() <= 0)) {
                ui->standardCutout->setChecked(false);
                standardLoaded = false;
            }
        }
        else {
            ui->standardCutout->setChecked(false);
            standardLoaded = false;
        }
    }
}
