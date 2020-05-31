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

static cv::Mat src1;
static cv::Mat src2;

RegistrateTwo::RegistrateTwo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegistrateTwo)
{
    ui->setupUi(this);
    ui->comboBox->addItem(tr("Choose two videoframes"));
    ui->comboBox->addItem(tr("Choose two images"));
    ui->areaMaximumLabel->setText(tr("Size of calculation area"));
    ui->rotationAngleLabel->setText(tr("Maximal tolerated rotation angle"));
    ui->iterationCountLabel->setText(tr("Number of iteration of algorithm"));
    ui->entropie_referencial->setText(tr("Reference entropy"));
    ui->entropie_translated->setText(tr("Translated entropy"));
    ui->tennengrad_referencial->setText(tr("Tennengrad reference"));
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

    /*QWidget* widgetVideoPB = ui->nabidkaAnalyzy->itemAt(1)->widget();
    chosenVideoPB = qobject_cast<QPushButton*>(widgetVideoPB);
    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    chosenVideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);

    QSignalMapper* m_sigmapper = new QSignalMapper(this);
    connect(chosenVideoPB,SIGNAL(clicked()),m_sigmapper,SLOT(map()));
    m_sigmapper->setMapping(chosenVideoPB,chosenVideoLE);
    QObject::connect(m_sigmapper, SIGNAL(mapped(QWidget *)),this, SLOT(chosenVideoPB_clicked(QWidget *)));
    QObject::connect(widgetVideoLE,SIGNAL(textChanged(const QString &)),this,
                     SLOT(Slot_VideoLE_textChanged(const QString&)));

    QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(2)->widget();    
    referenceNoLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
    QWidget* widgetPosunutehoLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
    translatedNoLE = qobject_cast<QLineEdit*>(widgetPosunutehoLE);
    QObject::connect(referenceNoLE,SIGNAL(textChanged(const QString &)),
                     this,SLOT(ReferenceLE_textChanged(const QString&)));
    QObject::connect(translatedNoLE,SIGNAL(textChanged(const QString &)),
                     this,SLOT(TranslatedLE_textChanged(const QString&)));*/
    referenceNoLE->setEnabled(false);
    translatedNoLE->setEnabled(false);

    QObject::connect(this,SIGNAL(checkRegistrationPass()),this,SLOT(evaluateCorrectValues()));
    localErrorDialogHandling[ui->registrateTwo] = new ErrorDialog(ui->registrateTwo);
    localErrorDialogHandling[ui->standardCutout] = new ErrorDialog(ui->standardCutout);
    localErrorDialogHandling[ui->extraCutout] = new ErrorDialog(ui->extraCutout);
    localErrorDialogHandling[referenceNoLE] = new ErrorDialog(referenceNoLE);
}

RegistrateTwo::~RegistrateTwo()
{
    delete ui;

    /*if (firstChoiceInitialised) {
        delete chosenVideoLE;
        delete chosenVideoPB;
        delete referenceNoLE;
        delete translatedNoLE;
        delete horizontalSpacer1;
    }

    if (secondChoiceInitialised) {
        delete horizontalSpacer2;
        delete chooseReferencialImagePB;
        delete chooseTranslatedImagePB;
        delete referenceImgLE;
        delete translatedImgLE;
    }*/
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
    chooseReferencialImagePB = new QPushButton();
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

    chooseReferencialImagePB->setText(tr("Chooce referencial"));
    chooseReferencialImagePB->setMinimumWidth(90);
    chooseReferencialImagePB->setMinimumHeight(20);
    chooseReferencialImagePB->setMaximumWidth(90);
    chooseReferencialImagePB->setMaximumHeight(20);
    chooseTranslatedImagePB->setText(tr("Choose shifted"));
    chooseTranslatedImagePB->setMinimumWidth(90);
    chooseTranslatedImagePB->setMinimumHeight(20);
    chooseTranslatedImagePB->setMaximumWidth(90);
    chooseTranslatedImagePB->setMaximumHeight(20);
}

void RegistrateTwo::placeChoiceOneWidgets(){    
    ui->nabidkaAnalyzy->addWidget(chosenVideoLE,0,0);
    ui->nabidkaAnalyzy->addWidget(chosenVideoPB,0,1);
    ui->nabidkaAnalyzy->addWidget(referenceNoLE,0,2);
    ui->nabidkaAnalyzy->addWidget(translatedNoLE,0,3);
    ui->nabidkaAnalyzy->addItem(horizontalSpacer1,0,4);

    connect(chosenVideoPB,SIGNAL(clicked()),this,SLOT(chosenVideoPB_clicked()));
    connect(chosenVideoLE,SIGNAL(textChanged(const QString &)),this,SLOT(Slot_VideoLE_textChanged(const QString &)));
    connect(referenceNoLE,SIGNAL(textChanged(const QString &)),this,SLOT(ReferenceLE_textChanged(const QString &)));
    connect(translatedNoLE,SIGNAL(textChanged(const QString &)),this,SLOT(TranslatedLE_textChanged(const QString &)));
}

void RegistrateTwo::placeChoiceTwoWidgets(){
    ui->nabidkaAnalyzy->addWidget(referenceImgLE,0,0);
    ui->nabidkaAnalyzy->addWidget(chooseReferencialImagePB,0,1);
    ui->nabidkaAnalyzy->addWidget(translatedImgLE,0,2);
    ui->nabidkaAnalyzy->addWidget(chooseTranslatedImagePB,0,3);
    ui->nabidkaAnalyzy->addItem(horizontalSpacer2,0,4);

    connect(chooseReferencialImagePB,SIGNAL(clicked()),this,SLOT(chosenReferenceImgPB_clicked()));
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
        else
        {
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
            return true;
        }
    }
    else if (i_method == "referencialImage"){
        referencialImg = cv::imread(i_path.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
        if (referencialImg.empty())
        {
            referenceImgLE->setStyleSheet("color: #FF0000");
            referencialImgCorrect = false;
            ui->areaMaximum->setEnabled(false);
            ui->rotationAngle->setEnabled(false);
            ui->iterationCount->setEnabled(false);
            referencialImg.release();
            return false;
        }
        else
        {
            referenceImgLE->setStyleSheet("color: #339900");
            referencialImgCorrect = true;
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
    else
        ui->registrateTwo->setEnabled(false);
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

void RegistrateTwo::on_comboBox_activated(int index)
{
    if (index == 2)
    {
        if (whatIsAnalysed != chosenSource::NOTHING)
            clearLayout(ui->nabidkaAnalyzy);

        whatIsAnalysed = chosenSource::NOTHING;
    }
    if (index == 0)
    {
        if (whatIsAnalysed != chosenSource::NOTHING)
            clearLayout(ui->nabidkaAnalyzy);
        if (whatIsAnalysed != chosenSource::VIDEO)
            initChoiceOneInnerWidgets();

        referencialImg.release();
        translatedImg.release();
        whatIsAnalysed = chosenSource::VIDEO;
        placeChoiceOneWidgets();

        // signals and slots should be connected from the class constructor (?)

        /*QWidget* widgetVideoPB = ui->nabidkaAnalyzy->itemAt(1)->widget();
        chosenVideoPB = qobject_cast<QPushButton*>(widgetVideoPB);
        QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
        chosenVideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
        /// Řešení se QSignalMapper konečně funkční, jen je třeba uvnitř funkce castit QLineEdit
        QSignalMapper* m_sigmapper = new QSignalMapper(this);
        connect(chosenVideoPB,SIGNAL(clicked()),m_sigmapper,SLOT(map()));
        m_sigmapper->setMapping(chosenVideoPB,chosenVideoLE);
        QObject::connect(m_sigmapper, SIGNAL(mapped(QWidget *)),this, SLOT(chosenVideoPB_clicked(QWidget *)));
        QObject::connect(widgetVideoLE,SIGNAL(textChanged(const QString &)),this,
                         SLOT(Slot_VideoLE_textChanged(const QString&)));

        QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
        referenceNoLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
        QWidget* widgettranslatedImageLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
        translatedNoLE = qobject_cast<QLineEdit*>(widgettranslatedImageLE);
        QObject::connect(referenceNoLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(ReferenceLE_textChanged(const QString&)));
        QObject::connect(translatedNoLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(TranslatedLE_textChanged(const QString&)));*/
}
    else if (index == 1)
    {
        if (whatIsAnalysed != chosenSource::NOTHING)
            clearLayout(ui->nabidkaAnalyzy);
        if (whatIsAnalysed != chosenSource::IMAGE)
            initChoiceTwoInnerWidgets();

        whatIsAnalysed = chosenSource::IMAGE;
        placeChoiceTwoWidgets();
        secondChoiceInitialised = true;
        typeOfFrangi = frangiType::GLOBAL;
        referencialNumber = -1;
        cap.release();

        /*QWidget* widgetReferencePB = ui->nabidkaAnalyzy->itemAt(1)->widget();
        QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
        QWidget* widgettranslatedImagePB = ui->nabidkaAnalyzy->itemAt(3)->widget();
        QWidget* widgettranslatedImageLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
        chooseReferencialImagePB = qobject_cast<QPushButton*>(widgetReferencePB);
        chooseTranslatedImagePB = qobject_cast<QPushButton*>(widgettranslatedImagePB);
        referenceImgLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
        translatedImgLE = qobject_cast<QLineEdit*>(widgettranslatedImageLE);

        QSignalMapper* m_sigmapper1 = new QSignalMapper(this);

        connect(chooseReferencialImagePB,SIGNAL(clicked()),m_sigmapper1,SLOT(map()));
        m_sigmapper1->setMapping(chooseReferencialImagePB,referenceImgLE);
        QObject::connect(m_sigmapper1, SIGNAL(mapped(QWidget *)),this, SLOT(chosenReferenceImgPB_clicked(QWidget *)));

        QSignalMapper* m_sigmapper2 = new QSignalMapper(this);
        connect(chooseTranslatedImagePB,SIGNAL(clicked()),m_sigmapper2,SLOT(map()));
        m_sigmapper2->setMapping(chooseTranslatedImagePB,translatedImgLE);
        QObject::connect(m_sigmapper2, SIGNAL(mapped(QWidget *)),this, SLOT(chosenTranslatedImgPB_clicked(QWidget *)));

        QObject::connect(referenceImgLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(ReferenceImgLE_textChanged(const QString&)));
        QObject::connect(translatedImgLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(TranslatedImgLE_textChanged(const QString&)));*/

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

/*void RegistrateTwo::chosenVideoPBWrapper()
{
    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    chosenVideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    emit chosenVideoPB_clicked(chosenVideoLE);
}*/

void RegistrateTwo::chosenVideoPB_clicked()
{
    QString pathToVideo = QFileDialog::getOpenFileName(this,
         tr("Choose video"), "","*.avi;;;");
    analyseAndSave(pathToVideo,chosenVideoAnalysis);
    chosenVideoLE->setText(chosenVideoAnalysis["filename"]);
    //chosenVideoLE = qobject_cast<QLineEdit*>(W);
    //chosenVideoLE->setText(chosenVideoAnalysis[1]);
    evaluateVideoImageInput(pathToVideo,"video");
}

void RegistrateTwo::chosenReferenceImgPB_clicked()
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         tr("Choose referencial image"), "",tr("Images (*.bmp *.png *.jpg)"));
    analyseAndSave(referencialImagePath,chosenReferencialImgAnalysis);
    //referenceImgLE = qobject_cast<QLineEdit*>(W);
    referenceImgLE->setText(chosenReferencialImgAnalysis["filename"]);
    evaluateVideoImageInput(referencialImagePath,"referencialImage");
}

void RegistrateTwo::chosenTranslatedImgPB_clicked()
{
    QString translatedImgPath = QFileDialog::getOpenFileName(this,
         tr("Choose translated image"), "",tr("Images (*.bmp *.png *.jpg)"));
    analyseAndSave(translatedImgPath,chosenTranslatedImgAnalysis);
    //translatedImgLE = qobject_cast<QLineEdit*>(W);
    translatedImgLE->setText(chosenTranslatedImgAnalysis["filename"]);
    evaluateVideoImageInput(translatedImgPath,"translatedImage");
}

void RegistrateTwo::ReferenceImgLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenReferencialImgAnalysis["folder"]+"/"+arg1+"."+chosenReferencialImgAnalysis["suffix"];
    evaluateVideoImageInput(fullPath,"referencialImage");
    if (referencialImgCorrect && translatedImgCorrect){
        ui->areaMaximum->setEnabled(true);
        ui->rotationAngle->setEnabled(true);
        ui->iterationCount->setEnabled(true);
    }
}

void RegistrateTwo::TranslatedImgLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenTranslatedImgAnalysis["folder"]+"/"+arg1+"."+chosenTranslatedImgAnalysis["suffix"];
    evaluateVideoImageInput(fullPath,"translatedImage");
    if (referencialImgCorrect && translatedImgCorrect){
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
        referencialNumber = -1;
        if (localErrorDialogHandling[referenceNoLE]->isEvaluated())
            localErrorDialogHandling[referenceNoLE]->hide();
    }
    else
    {
        referenceNoLE->setStyleSheet("color: #33aa00");
        referenceCorrect = true;
        referencialNumber = referenceFrameNo;
        if (referenceCorrect && translatedCorrect){
            ui->areaMaximum->setEnabled(true);
            ui->rotationAngle->setEnabled(true);
            ui->iterationCount->setEnabled(true);
        }
        if (!checkReferentialFrameExistence(SharedVariables::getSharedVariables()->getPath("saveDatFilesPath"),
                                             chosenVideoAnalysis["filename"],referencialNumber)){
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
    cv::Mat referencialImage,translatedImage;
    if (whatIsAnalysed == chosenSource::VIDEO) {
        QString filePath = chosenVideoAnalysis["folder"]+"/"+chosenVideoAnalysis["filename"]+"."+chosenVideoAnalysis["suffix"];
        cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
        cap.set(CV_CAP_PROP_POS_FRAMES,double(referencialNumber));
        cap.read(referencialImage);
        cap.set(CV_CAP_PROP_POS_FRAMES,double(translatedNumber)-1);
        cap.read(translatedImage);
    }
    else if (whatIsAnalysed == chosenSource::IMAGE) {
        referencialImg.copyTo(referencialImage);
        translatedImg.copyTo(translatedImage);
    }
    transformMatTypeTo8C3(referencialImage);
    transformMatTypeTo8C3(translatedImage);
    double entropyTranslated,entropyReference;
    cv::Scalar tennengradTranslated,tennengradReference;
    if (!calculateParametersET(referencialImage,entropyReference,tennengradReference)){
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

    cv::Rect cutoutExtra(0,0,0,0);
    cv::Rect cutoutStandard(0,0,0,0);
    //cv::Rect anomalyArea(0,0,0,0);
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Point3d maximum_frangi_reverse(0.0,0.0,0.0);
    QPoint _MFR(0,0);
    //cv::Mat image;
    referencialImg = referencialImage;

    if (ui->standardCutout->isChecked() || ui->extraCutout->isChecked()){
        // if one of those checkboxes is checked, it is highly probable that videoInformation
        // will contain come info
        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(chosenVideoAnalysis["filename"]) &&
                (SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"frame").toInt() == referencialNumber)){
            // video was analysed
            QPoint _frangiCoordinates = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"frangi").toPoint();
            maximum_frangi_reverse = transform_QPoint_to_CV_Point3d(_frangiCoordinates);

            // standard cutout is can be modified in both cases
            QRect _standardCutout = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"standard").toRect();
            cutoutStandard = transform_QRect_to_CV_RECT(_standardCutout);

            if (ui->extraCutout->isChecked()) {
                QRect _extraCutout = SharedVariables::getSharedVariables()->getVideoInformation(chosenVideoAnalysis["filename"],"extra").toRect();
                cutoutExtra = transform_QRect_to_CV_RECT(_extraCutout);
                //image = referencialImage(cutoutExtra);
                scaleChanged = true;
            }
            else{
                //image = referencialImage;
                scaleChanged = false;
            }

        }
    }
    else if (findReferentialFrameData(chosenVideoAnalysis["filename"],referencialNumber,_MFR)){
        // video could be analysed in the past -> check it
        maximum_frangi_reverse = transform_QPoint_to_CV_Point3d(_MFR);
        cutoutStandard = calculateStandardCutout(maximum_frangi_reverse,
                                                 SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                                 referencialImage.rows,
                                                 referencialImage.cols);
        //image = referencialImage;
        scaleChanged = false;
    }
    else{
        // the referential frame was not preprocessed -> no anomaly is present in the frame
        // video was not analysed in the past -> no data about referential frame and frangi coordinates
        // ->
        // extraCutout is not expected, only standard cutout must be calculated from the newly calculated
        // frangi coordinates -> preprocessing complete registration
        /*ProcessingIndicator* _processingIndicator = new ProcessingIndicator(tr("Calculating Frangi coordinates"),
                                                                            ui->extraCutout);
        _processingIndicator->placeIndicator();
        _processingIndicator->showIndicator();*/

        if (!preprocessingCompleteRegistration(referencialImage,
                                               maximum_frangi_reverse,
                                               cutoutStandard,
                                               SharedVariables::getSharedVariables()->getFrangiParameterWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                               SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]),
                                               SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(typeOfFrangi,chosenVideoAnalysis["filename"])
                                               )){
            localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",10);
            localErrorDialogHandling[ui->registrateTwo]->show(false);
            return;
        }
        else{
            //image = referencialImage;
            scaleChanged = false;
        }

        //_processingIndicator->hideIndicator();
    }

    //int rows = referencialImage.rows;
    //int cols = referencialImage.cols;

    /*maximum_frangi_reverse = frangi_analysis(image,2,2,0,"",1,pt_temp,
                                                         SharedVariables::getSharedVariables()->getFrangiParameters(),
                                                         SharedVariables::getSharedVariables()->getFrangiMargins());
    */
    qDebug()<<"Maximum frangi reverse "<<maximum_frangi_reverse.x<<" "<<maximum_frangi_reverse.y;
    qDebug()<<"Standard cutout "<<cutoutStandard.width<<" "<<cutoutStandard.height;
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
                                                        referencialImage,
                                                        translatedImage,
                                                        maximum_frangi_reverse,
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
        cv::Mat registratedFrame = cv::Mat::zeros(cv::Size(referencialImage.cols,referencialImage.rows),
                                                  CV_32FC3);
        cv::Mat translated;
        cv::Point3d translation(_pocX[0],_pocY[0],0.0);
        if (whatIsAnalysed == chosenSource::VIDEO) {
            cap.set(CV_CAP_PROP_POS_FRAMES,translatedNumber);
            if(!cap.read(translated))
            {
                cap.release();
                return;
            }
        }
        else {
            translatedImg.copyTo(translated);
        }
        transformMatTypeTo8C3(translated);
        registratedFrame = frameTranslation(translated,translation,
                                            referencialImage.rows,referencialImage.cols);
        registratedFrame = frameRotation(registratedFrame,l_angleSum[0]);
        qDebug()<<"Size check before showing results";
        qDebug()<<referencialImage.rows<<" "<<referencialImage.cols;
        qDebug()<<registratedFrame.rows<<" "<<registratedFrame.cols;
        RegistrationResult *showResults = new RegistrationResult(referencialImage,registratedFrame);
        showResults->start(1);
        showResults->setModal(true);
        showResults->show();
    }
    emit calculationStopped();
}

void RegistrateTwo::showDialog()
{
    if (ui->standardCutout->isChecked() || ui->extraCutout->isChecked()){
        // before any operation starts, it is necessary to make sure the frangi analysis function will have
        // all requested inputs
        //QMap<QString,int> _storedMargins = SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]);
        //QMap<QString,double> _parameters = SharedVariables::getSharedVariables()->getFrangiParameterWrapper(typeOfFrangi,chosenVideoAnalysis["filename"]);
        //if (_storedMargins.count() == 4 && _parameters.count() == 6){
        QString fullPath;
        if (whatIsAnalysed == chosenSource::VIDEO) {
            fullPath = chosenVideoAnalysis["folder"]+"/"+chosenVideoAnalysis["filename"]+"."+chosenVideoAnalysis["suffix"];
        }
        else if (whatIsAnalysed == chosenSource::IMAGE) {
            fullPath = chosenReferencialImgAnalysis["folder"]+"/"+chosenReferencialImgAnalysis["filename"]+"."+chosenReferencialImgAnalysis["suffix"];
        }

        if (QObject::sender() == ui->standardCutout){
            if (ui->standardCutout->isChecked())
            {
                ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,cutoutType::STANDARD,false,whatIsAnalysed);
                markAnomaly->setModal(true);
                markAnomaly->show();
                //connect(markAnomaly,SIGNAL(),this,SLOT());
            }
        }
        else if (QObject::sender() == ui->extraCutout){
            if (ui->extraCutout->isChecked())
            {
                ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,cutoutType::EXTRA,false,whatIsAnalysed);
                markAnomaly->setModal(true);
                markAnomaly->show();
                //connect(markAnomaly,SIGNAL(),this,SLOT());
            }
        }
        //}
        /*else{
            if (QObject::sender() == ui->extraCutout){
                localErrorDialogHandling[ui->extraCutout]->evaluate("left","hardError",5);
                ui->extraCutout->setChecked(false);
            }
            else if (QObject::sender() == ui->standardCutout){
                localErrorDialogHandling[ui->standardCutout]->evaluate("left","hardError",5);
                ui->standardCutout->setChecked(false);
            }
        }*/
    }
}
