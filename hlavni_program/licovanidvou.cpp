#include "licovanidvou.h"
#include "ui_licovanidvou.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/entropie.h"
#include "licovani/multiPOC_Ai1.h"
#include "licovani/korekce_zapis.h"
#include "licovani/fazova_korelace_funkce.h"
#include "dialogy/errordialog.h"
#include "dialogy/clickimageevent.h"
#include "hlavni_program/frangi_detektor.h"
#include "hlavni_program/vysledeklicovani.h"
#include "fancy_staff/sharedvariables.h"
#include "util/souborove_operace.h"
#include "util/prace_s_vektory.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

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
#include <QSignalMapper>
#include <typeinfo>

static cv::Mat src1;
static cv::Mat src2;

LicovaniDvou::LicovaniDvou(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LicovaniDvou)
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
    ui->horizontalAnomaly->setText(tr("Top/bottom anomaly"));
    ui->verticalAnomaly->setText(tr("Left/right anomaly"));
    ui->registrateTwo->setText(tr("Registrate"));
    ui->registrateTwo->setEnabled(false);

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    ui->areaMaximum->setPlaceholderText("0 - 20");
    ui->areaMaximum->setEnabled(false);
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->rotationAngle->setEnabled(false);
    ui->iterationCount->setPlaceholderText("1 - Inf; -1~automatic settings");
    ui->iterationCount->setEnabled(false);
    ui->horizontalAnomaly->setChecked(false);
    ui->verticalAnomaly->setChecked(false);
    ui->horizontalAnomaly->setEnabled(false);
    ui->verticalAnomaly->setEnabled(false);

    frangiMaximumCoords = SharedVariables::getSharedVariables()->getFrangiMaximum();
    connect(ui->verticalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    connect(ui->horizontalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));

    initChoiceOneInnerWidgets();
    placeChoiceOneWidgets();
    formerIndex=0;

    QWidget* widgetVideoPB = ui->nabidkaAnalyzy->itemAt(1)->widget();
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
                     this,SLOT(TranslatedLE_textChanged(const QString&)));
    referenceNoLE->setEnabled(false);
    translatedNoLE->setEnabled(false);

    QObject::connect(this,SIGNAL(checkRegistrationPass()),this,SLOT(evaluateCorrectValues()));
    localErrorDialogHandling[ui->registrateTwo] = new ErrorDialog(ui->registrateTwo);
}

LicovaniDvou::~LicovaniDvou()
{
    delete ui;
}

void LicovaniDvou::initChoiceOneInnerWidgets(){
    chosenVideoLE = new QLineEdit();
    chosenVideoPB = new QPushButton();
    referenceNoLE = new QLineEdit();
    translatedNoLE = new QLineEdit();
    horizontalSpacer1 = new QSpacerItem(20,20);
}

void LicovaniDvou::initChoiceTwoInnerWidgets(){
    horizontalSpacer2 = new QSpacerItem(5,20);
    referenceImgLE = new QLineEdit();
    translatedImgLE = new QLineEdit();
    chooseReferencialImagePB = new QPushButton();
    chooseTranslatedImagePB = new QPushButton();
}

void LicovaniDvou::placeChoiceOneWidgets(){
    chosenVideoLE->setPlaceholderText(tr("Chosen video"));
    chosenVideoLE->setMinimumWidth(110);
    chosenVideoLE->setMinimumHeight(20);
    chosenVideoPB->setText(tr("Choose video"));
    chosenVideoPB->setMinimumWidth(71);
    chosenVideoPB->setMinimumHeight(23);
    referenceNoLE->setPlaceholderText("Ref");
    referenceNoLE->setMinimumWidth(31);
    referenceNoLE->setMinimumHeight(20);
    translatedNoLE->setPlaceholderText(tr("Moved"));
    translatedNoLE->setMinimumWidth(41);
    translatedNoLE->setMinimumHeight(20);

    ui->nabidkaAnalyzy->addWidget(chosenVideoLE,0,0);
    ui->nabidkaAnalyzy->addWidget(chosenVideoPB,0,1);
    ui->nabidkaAnalyzy->addWidget(referenceNoLE,0,2);
    ui->nabidkaAnalyzy->addWidget(translatedNoLE,0,3);
    ui->nabidkaAnalyzy->addItem(horizontalSpacer1,0,4);
}

void LicovaniDvou::placeChoiceTwoWidgets(){
    referenceImgLE->setMinimumWidth(20);
    referenceImgLE->setMinimumHeight(20);
    referenceImgLE->setPlaceholderText("Ref");
    translatedImgLE->setMinimumWidth(20);
    translatedImgLE->setMinimumHeight(20);
    translatedImgLE->setPlaceholderText(tr("Moved"));
    chooseReferencialImagePB->setText(tr("Chooce referrence"));
    chooseReferencialImagePB->setMinimumWidth(90);
    chooseReferencialImagePB->setMinimumHeight(23);
    chooseTranslatedImagePB->setText(tr("Choose moved"));
    chooseTranslatedImagePB->setMinimumWidth(90);
    chooseTranslatedImagePB->setMinimumHeight(23);
    ui->nabidkaAnalyzy->addWidget(referenceImgLE,0,0);
    ui->nabidkaAnalyzy->addWidget(chooseReferencialImagePB,0,1);
    ui->nabidkaAnalyzy->addWidget(translatedImgLE,0,2);
    ui->nabidkaAnalyzy->addWidget(chooseTranslatedImagePB,0,3);
    ui->nabidkaAnalyzy->addItem(horizontalSpacer2,0,4);
}

void LicovaniDvou::analyseAndSave(QString analysedFolder, QVector<QString> &whereToSave){
    QString folder,filename,suffix;
    processFilePath(analysedFolder,folder,filename,suffix);
    if (whereToSave.length() == 0)
    {
        whereToSave.push_back(folder);
        whereToSave.push_back(filename);
        whereToSave.push_back(suffix);
    }
    else
    {
        whereToSave.clear();
        whereToSave.push_back(folder);
        whereToSave.push_back(filename);
        whereToSave.push_back(suffix);
    }
}

void LicovaniDvou::evaluateVideoImageInput(QString path, QString method){
    if (method == "video"){
        cap = cv::VideoCapture(path.toLocal8Bit().constData());
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
        }
        else
        {
            chosenVideoLE->setStyleSheet("color: #339900");
            videoCorrect = true;
            referenceNoLE->setEnabled(true);
            translatedNoLE->setEnabled(true);
        }
    }
    else if (method == "referencialImage"){
        cv::Mat referencialImg;
        referencialImg = cv::imread(path.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
        if (referencialImg.empty())
        {
            referenceImgLE->setStyleSheet("color: #FF0000");
            referencialImgCorrect = false;
            ui->areaMaximum->setEnabled(false);
            ui->rotationAngle->setEnabled(false);
            ui->iterationCount->setEnabled(false);
        }
        else
        {
            referenceImgLE->setStyleSheet("color: #339900");
            referencialImgCorrect = true;
        }
        referencialImg.release();
    }
    else if (method == "translatedImage"){
        cv::Mat translatedImg;
        translatedImg = cv::imread(path.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
        if (translatedImg.empty())
        {
            translatedImgLE->setStyleSheet("color: #FF0000");
            translatedImgCorrect = false;
        }
        else
        {
            translatedImgLE->setStyleSheet("color: #339900");
            translatedImgCorrect = true;
        }
        translatedImg.release();
    }
}

void LicovaniDvou::evaluateCorrectValues(){
    if (areaMaximumCorrect && angleCorrect && iterationCorrect){
        ui->registrateTwo->setEnabled(true);
        ui->horizontalAnomaly->setEnabled(true);
        ui->verticalAnomaly->setEnabled(true);
    }
    else
        ui->registrateTwo->setEnabled(false);
}

void LicovaniDvou::checkPaths(){
    if (formerIndex == 0){

        if (SharedVariables::getSharedVariables()->getPath("cestaKvideim") == "")
            chosenVideoLE->setPlaceholderText(tr("Chosen video"));
        else{
            analyseAndSaveFirst(SharedVariables::getSharedVariables()->getPath("cestaKvideim"),chosenVideoAnalysis);
            chosenVideoLE->setText(chosenVideoAnalysis[1]);
        }
    }
}

void LicovaniDvou::on_comboBox_activated(int index)
{
    if (index == 2)
    {
        if (formerIndex != 2)
            clearLayout(ui->nabidkaAnalyzy);

        formerIndex = 2;
    }
    if (index == 0)
    {
        if (formerIndex != 2)
            clearLayout(ui->nabidkaAnalyzy);
        if (formerIndex != 0)
            initChoiceOneInnerWidgets();

        formerIndex = 0;
        placeChoiceOneWidgets();

        QWidget* widgetVideoPB = ui->nabidkaAnalyzy->itemAt(1)->widget();
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
                         this,SLOT(TranslatedLE_textChanged(const QString&)));
}
    else if (index == 1)
    {
        if (formerIndex != 2)
            clearLayout(ui->nabidkaAnalyzy);
        if (formerIndex != 1)
            initChoiceTwoInnerWidgets();

        formerIndex = 1;
        placeChoiceTwoWidgets();

        QWidget* widgetReferencePB = ui->nabidkaAnalyzy->itemAt(1)->widget();
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
                         this,SLOT(TranslatedImgLE_textChanged(const QString&)));

    }
}

void LicovaniDvou::clearLayout(QGridLayout *layout)
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

void LicovaniDvou::chosenVideoPBWrapper()
{
    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    chosenVideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    emit chosenVideoPB_clicked(chosenVideoLE);
}

void LicovaniDvou::chosenVideoPB_clicked(QWidget *W)
{
    QString pathToVideo = QFileDialog::getOpenFileName(this,
         tr("Choose video"), "","*.avi;;All files (*)");
    analyseAndSaveFirst(pathToVideo,chosenVideoAnalysis);
    chosenVideoLE = qobject_cast<QLineEdit*>(W);
    chosenVideoLE->setText(chosenVideoAnalysis[1]);
    evaluateVideoImageInput(pathToVideo,"video");
}

void LicovaniDvou::chosenReferenceImgPB_clicked(QWidget* W)
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         "Choose referencial image", "","*.bmp;;All files (*)");
    analyseAndSave(referencialImagePath,chosenReferencialImgAnalysis);
    referenceImgLE = qobject_cast<QLineEdit*>(W);
    referenceImgLE->setText(chosenReferencialImgAnalysis[1]);
    evaluateVideoImageInput(referencialImagePath,"referencialImage");
}

void LicovaniDvou::chosenTranslatedImgPB_clicked(QWidget* W)
{
    QString translatedImgPath = QFileDialog::getOpenFileName(this,
         "Choose translated image", "","*.bmp;;All files (*)");
    analyseAndSave(translatedImgPath,chosenTranslatedImgAnalysis);
    translatedImgLE = qobject_cast<QLineEdit*>(W);
    translatedImgLE->setText(chosenTranslatedImgAnalysis[1]);
    evaluateVideoImageInput(translatedImgPath,"translatedImage");
}

void LicovaniDvou::ReferenceImgLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenReferencialImgAnalysis[0]+"/"+arg1+"."+chosenReferencialImgAnalysis[2];
    evaluateVideoImageInput(fullPath,"referencialImage");
    if (referencialImgCorrect && translatedImgCorrect){
        ui->areaMaximum->setEnabled(true);
        ui->rotationAngle->setEnabled(true);
        ui->iterationCount->setEnabled(true);
    }
}

void LicovaniDvou::TranslatedImgLE_textChanged(const QString &arg1)
{
    QString fullPath = chosenTranslatedImgAnalysis[0]+"/"+arg1+"."+chosenTranslatedImgAnalysis[2];
    evaluateVideoImageInput(fullPath,"translatedImage");
    if (referencialImgCorrect && translatedImgCorrect){
        ui->areaMaximum->setEnabled(true);
        ui->rotationAngle->setEnabled(true);
        ui->iterationCount->setEnabled(true);
    }
}

void LicovaniDvou::Slot_VideoLE_textChanged(const QString &s)
{
    QString fullPath = chosenVideoAnalysis[0]+"/"+s+"."+chosenVideoAnalysis[2];
    evaluateVideoImageInput(fullPath,"video");
}

void LicovaniDvou::ReferenceLE_textChanged(const QString &arg1)
{
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int referenceFrameNo = arg1.toInt();
    if (referenceFrameNo < 0 || referenceFrameNo > frameCount)
    {
        referenceNoLE->setStyleSheet("color: #FF0000");
        referenceCorrect = false;
        referencialNumber = -1;
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
    }
}

void LicovaniDvou::TranslatedLE_textChanged(const QString &arg1)
{
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int translatedFrameNo = arg1.toInt();
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

void LicovaniDvou::checkInputNumber(double input, double lower, double upper, QLineEdit *editWidget, double &finalValue, bool &evaluation){

    if (input < lower || (input > upper && upper != 0.0)){
        editWidget->setStyleSheet("color: #FF0000");
        editWidget->setText("");
        finalValue = -99;
        evaluation = false;
    }
    else if (input < lower || (upper == 0.0 && input == 0.0)){
        editWidget->setStyleSheet("color: #FF0000");
        editWidget->setText("");
        finalValue = -99;
        evaluation = false;
    }
    else{
        editWidget->setStyleSheet("color: #33aa00");
        evaluation = true;
        finalValue = input;
        emit checkRegistrationPass();
    }

}

void LicovaniDvou::on_areaMaximum_editingFinished()
{
    bool ok;
    double inputNumber = ui->areaMaximum->text().toDouble(&ok);
    if (ok)
        checkInputNumber(inputNumber,1.0,20.0,ui->areaMaximum,areaMaximum,areaMaximumCorrect);
    else
        ui->areaMaximum->setText("");
}

void LicovaniDvou::on_rotationAngle_editingFinished()
{
    bool ok;
    double inputNumber = ui->rotationAngle->text().toDouble(&ok);
    if (ok)
        checkInputNumber(inputNumber,0.0,0.5,ui->rotationAngle,angle,angleCorrect);
    else
        ui->rotationAngle->setText("");
}

void LicovaniDvou::on_iterationCount_editingFinished()
{
    bool ok;
    double inputNumber = ui->iterationCount->text().toDouble(&ok);
    if (ok)
        checkInputNumber(inputNumber,-1.0,0.0,ui->iterationCount,iteration,iterationCorrect);
    else
        ui->iterationCount->setText("");
}

void LicovaniDvou::on_registrateTwo_clicked()
{
    QString filePath = chosenVideoAnalysis[0]+"/"+chosenVideoAnalysis[1]+"."+chosenVideoAnalysis[2];
    cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    cap.set(CV_CAP_PROP_POS_FRAMES,double(referencialNumber)-1);
    cv::Mat referencialImage,translatedImage;
    cap.read(referencialImage);
    kontrola_typu_snimku_8C3(referencialImage);
    cap.set(CV_CAP_PROP_POS_FRAMES,double(translatedNumber)-1);
    cap.read(translatedImage);
    kontrola_typu_snimku_8C3(translatedImage);
    double entropyTranslated,entropyReference;
    cv::Scalar tennengradTranslated,tennengradReference;
    if (!calculateParametersET(referencialImage,entropyReference,tennengradReference)){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",8);
        localErrorDialogHandling[ui->registrateTwo]->show();
        return;
    }
    if(!calculateParametersET(translatedImage,entropyTranslated,tennengradTranslated)){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",9);
        localErrorDialogHandling[ui->registrateTwo]->show();
        return;
    }
    ui->ER->setText(QString::number(entropyReference));
    ui->TR->setText(QString::number(tennengradReference[0]));
    ui->EM->setText(QString::number(entropyTranslated));
    ui->TM->setText(QString::number(tennengradTranslated[0]));

    cv::Rect cutoutExtra(0,0,0,0);
    cv::Rect cutoutStandard(0,0,0,0);
    cv::Rect cutoutAnomaly(0,0,0,0);
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Point2d horizontalAnomaly = SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords();
    cv::Point2d verticalAnomaly = SharedVariables::getSharedVariables()->getVerticalAnomalyCoords();
    cv::Mat image;
    //qDebug()<<referencialImage.rows<<" "<<referencialImage.cols;
    if (!preprocessingCompleteRegistration(referencialImage,
                                           image,
                                           SharedVariables::getSharedVariables()->getFrangiParameters(),
                                           verticalAnomaly,
                                           horizontalAnomaly,
                                           cutoutAnomaly,
                                           cutoutExtra,
                                           cutoutStandard,
                                           cap,
                                           scaleChanged)){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",10);
        localErrorDialogHandling[ui->registrateTwo]->show();
        return;
    }
    int rows = referencialImage.rows;
    int cols = referencialImage.cols;
    qDebug()<<"preprocessing completed."<<image.rows<<" "<<image.cols;
    cv::Point3d maximum_frangi_reverse = frangi_analysis(image,2,2,0,"",1,pt_temp,
                                                         SharedVariables::getSharedVariables()->getFrangiParameters());
    qDebug()<<"Maximum frangi reverse "<<maximum_frangi_reverse.x<<" "<<maximum_frangi_reverse.y;
    /// Beginning
    cv::Mat intermediate_result = cv::Mat::zeros(image.size(), CV_32FC3);
    cv::Point3d pt3(0.0,0.0,0.0);
    double l_angle = 0.0;
    QVector<double> l_angleSum;
    QVector<double> fr_x;
    QVector<double> fr_y;
    QVector<double> fr_eukl;
    QVector<double> _pocX;
    QVector<double> _pocY;
    QVector<double> pomD(static_cast<int>(cap.get(CV_CAP_PROP_FRAME_COUNT)),0.0);
    fr_x = pomD;fr_y = pomD;fr_eukl = pomD;_pocX = pomD;_pocY = pomD;l_angleSum = pomD;
    bool registrationSuccessfull = licovani_nejvhodnejsich_snimku(cap,
                                                                  referencialImage,
                                                                  maximum_frangi_reverse,
                                                                  translatedNumber,
                                                                  iteration,
                                                                  areaMaximum,
                                                                  l_angle,
                                                                  cutoutExtra,
                                                                  cutoutStandard,
                                                                  scaleChanged,
                                                                  SharedVariables::getSharedVariables()->getFrangiParameters(),
                                                                  _pocX,_pocY,fr_x,fr_y,fr_eukl,
                                                                  l_angleSum);
    /// Konec
    if (!registrationSuccessfull){
        localErrorDialogHandling[ui->registrateTwo]->evaluate("left","hardError",11);
        localErrorDialogHandling[ui->registrateTwo]->show();
        return;
    }
    else
    {
        qDebug()<<"translation after multiPOC "<<_pocX[translatedNumber]<<" "<<_pocY[translatedNumber];
        cv::Mat plneSlicovany = cv::Mat::zeros(cv::Size(cols,rows), CV_32FC3);
        cv::Mat posunuty;
        cv::Point3d translation(_pocX[translatedNumber],_pocY[translatedNumber],0.0);
        cap.set(CV_CAP_PROP_POS_FRAMES,translatedNumber);
        if(!cap.read(posunuty))
        {
            return;
        }
        kontrola_typu_snimku_8C3(posunuty);
        plneSlicovany = translace_snimku(posunuty,translation,rows,cols);
        plneSlicovany = rotace_snimku(plneSlicovany,l_angleSum[translatedNumber]);

        VysledekLicovani *vysledekLicovani = new VysledekLicovani(referencialImage,plneSlicovany);
        vysledekLicovani->start(1);
        vysledekLicovani->setModal(true);
        vysledekLicovani->show();
    }
}

void LicovaniDvou::showDialog()
{
    if (ui->verticalAnomaly->isChecked())
    {
        QString fullPath = chosenVideoAnalysis[0]+"/"+chosenVideoAnalysis[1]+"."+chosenVideoAnalysis[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,1);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
    if (ui->horizontalAnomaly->isChecked())
    {
        QString fullPath = chosenVideoAnalysis[0]+"/"+chosenVideoAnalysis[1]+"."+chosenVideoAnalysis[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,2);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
}
