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
//#include "hlavni_program/vysledeklicovani.h"
//#include "hlavni_program/t_b_ho.h"
#include "util/souborove_operace.h"

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

static cv::Mat src1;
static cv::Mat src2;

extern QString videaKanalyzeAktual;
extern QString ulozeniVideiAktual;
extern QString TXTnacteniAktual;
extern QString TXTulozeniAktual;
extern QString paramFrangi;

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
    areaMaximum = 10.0;
    ui->areaMaximum->setEnabled(false);
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    angle = 0.1;
    ui->rotationAngle->setEnabled(false);
    ui->iterationCount->setPlaceholderText("1 - Inf; -1~automatic settings");
    iteration = -1;
    ui->iterationCount->setEnabled(false);
    ui->horizontalAnomaly->setChecked(false);
    ui->verticalAnomaly->setChecked(false);

    frangiMaximumCoords = detekovane_frangiho_maximum;
    connect(ui->verticalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    //velikost_frangi_opt(6,FrangiParametersVector);

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
    //ui->nabidkaAnalyzy->itemAt(2)->widget()->setEnabled(false);
    //ui->nabidkaAnalyzy->itemAt(3)->widget()->setEnabled(false);
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

void LicovaniDvou::analyseAndSaveFirst(QString analysedFolder,QVector<QString> &whereToSave){
    QString folder,filename,suffix;
    QStringList filesFound;
    int foundCount = 0;
    analyzuj_jmena_souboru_avi(analysedFolder,filesFound,foundCount,"avi");
    if (foundCount != 0){
        QString fullName = analysedFolder+"/"+filesFound.at(0);
        zpracujJmeno(fullName,folder,filename,suffix);
        if (whereToSave.length() == 0){
            whereToSave.push_back(folder);
            whereToSave.push_back(filename);
            whereToSave.push_back(suffix);
        }
        else{
            whereToSave.clear();
            whereToSave.push_back(folder);
            whereToSave.push_back(filename);
            whereToSave.push_back(suffix);
        }
    }
}

void LicovaniDvou::analyseAndSave(QString analysedFolder, QVector<QString> &whereToSave){
    QString folder,filename,suffix;
    zpracujJmeno(analysedFolder,folder,filename,suffix);
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
            //qDebug()<<"video nelze otevrit pro potreby zpracovani";
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
            //qDebug()<<"Obrazek zvoleny jako referenční nelze otevřít.";
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
            qDebug()<<"Obrazek zvoleny jako posunutý nelze otevřít.";
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
    }
}

void LicovaniDvou::checkPaths(){
    if (formerIndex == 0){

        if (videaKanalyzeAktual == "")
            chosenVideoLE->setPlaceholderText(tr("Chosen video"));
        else{
            analyseAndSaveFirst(videaKanalyzeAktual,chosenVideoAnalysis);
            chosenVideoLE->setText(chosenVideoAnalysis[1]);
        }
    }
    /*if (paramFrangi != ""){
        QFile soubor;
        soubor.setFileName(paramFrangi+"/frangiParameters.json");
        FrangiParametersFile = readJson(soubor);
        QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
        for (int a = 0; a < 6; a++)
            inicializace_frangi_opt(FrangiParametersFile,parametry.at(a),FrangiParametersVector,a);
    }*/
}

void LicovaniDvou::on_comboBox_activated(int index)
{
    qDebug()<<"Index Comboboxu: "<<index;
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
        QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
        translatedNoLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);
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
        QWidget* widgetPosunutyPB = ui->nabidkaAnalyzy->itemAt(3)->widget();
        QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
        chooseReferencialImagePB = qobject_cast<QPushButton*>(widgetReferencePB);
        chooseTranslatedImagePB = qobject_cast<QPushButton*>(widgetPosunutyPB);
        referenceImgLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
        translatedImgLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);

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
    int pocetSLoupcu = layout->columnCount();
    int pocetRadku = layout->rowCount();
    qDebug()<<"pocetSLoupcu: "<<pocetSLoupcu<<" pocetRadku: "<<pocetRadku;
    for (int a = 1; a <= pocetRadku; a++)
    {
        for (int b = 1; b <= pocetSLoupcu; b++)
        {
            QWidget* widget = layout->itemAtPosition(a-1,b-1)->widget();
            layout->removeWidget(widget);
            delete widget;
        }
    }
}

void LicovaniDvou::chosenVideoPBWrapper()
{
    //vyberVideaPB_clicked(dynamic_cast<QLineEdit*>(sender()));
    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    chosenVideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    emit chosenVideoPB_clicked(chosenVideoLE);
}

void LicovaniDvou::chosenVideoPB_clicked(QWidget *W)
{
    QString pathToVideo = QFileDialog::getOpenFileName(this,
         "Vyberte snímek pro Frangiho filtr", "","*.avi;;Všechny soubory (*)");
    //qDebug()<<"vybrane video: "<<videoProDvojiciSnimku;
    analyseAndSaveFirst(pathToVideo,chosenVideoAnalysis);
    chosenVideoLE = qobject_cast<QLineEdit*>(W);
    //qDebug()<<rozborVybranehoSouboru[0];
    //qDebug()<<rozborVybranehoSouboru[1];
    //qDebug()<<rozborVybranehoSouboru[2];
    chosenVideoLE->setText(chosenVideoAnalysis[1]);
    evaluateVideoImageInput(pathToVideo,"video");
}

void LicovaniDvou::chosenReferenceImgPB_clicked(QWidget* W)
{
    QString referencialImagePath = QFileDialog::getOpenFileName(this,
         "Vyberte referenční obrázek", "","*.bmp;;Všechny soubory (*)");
    analyseAndSave(referencialImagePath,chosenReferencialImgAnalysis);
    referenceImgLE = qobject_cast<QLineEdit*>(W);
    referenceImgLE->setText(chosenReferencialImgAnalysis[1]);
    evaluateVideoImageInput(referencialImagePath,"referencialImage");
}

void LicovaniDvou::chosenTranslatedImgPB_clicked(QWidget* W)
{
    QString translatedImgPath = QFileDialog::getOpenFileName(this,
         "Vyberte posunutý obrázek", "","*.bmp;;Všechny soubory (*)");
    analyseAndSave(translatedImgPath,chosenTranslatedImgAnalysis);
    translatedImgLE = qobject_cast<QLineEdit*>(W);
    translatedImgLE->setText(chosenTranslatedImgAnalysis[1]);
    evaluateVideoImageInput(translatedImgPath,"translatedImage");
}

void LicovaniDvou::ReferenceImgLE_textChanged(const QString &arg1)
{
    //QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    //QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
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
    //QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
    //QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);
    QString fullPath = chosenTranslatedImgAnalysis[0]+"/"+arg1+"."+chosenTranslatedImgAnalysis[2];
    evaluateVideoImageInput(fullPath,"translatedImage");
    if (referencialImgCorrect && translatedImgCorrect){
        ui->areaMaximum->setEnabled(true);
        ui->rotationAngle->setEnabled(true);
        ui->iterationCount->setEnabled(true);
    }
}

/*void LicovaniDvou::VideoLE_textChanged(QLineEdit *LE, QString &s)
{
    QString kompletni_cesta = rozborVybranehoSouboru[0]+"/"+s+"."+rozborVybranehoSouboru[2];
    cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        LE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
        spravnostVidea = false;
    }
    else
    {
        LE->setStyleSheet("color: #339900");
        spravnostVidea = true;
        rozborVybranehoSouboru[1] = s;
    }
}*/

void LicovaniDvou::Slot_VideoLE_textChanged(const QString &s)
{
    //QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    //chosenVideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    QString fullPath = chosenVideoAnalysis[0]+"/"+s+"."+chosenVideoAnalysis[2];
    evaluateVideoImageInput(fullPath,"video");
}

void LicovaniDvou::ReferenceLE_textChanged(const QString &arg1)
{
    //QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
    //QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int referenceFrameNo = arg1.toInt();
    if (referenceFrameNo < 0 || referenceFrameNo > frameCount)
    {
        referenceNoLE->setStyleSheet("color: #FF0000");
        //qDebug()<<"Referencni snimek nelze ve videu dohledat";
        referenceCorrect = false;
        referenceNumber = -1;
    }
    else
    {
        referenceNoLE->setStyleSheet("color: #339900");
        referenceCorrect = true;
        referenceNumber = referenceFrameNo;
        if (referenceCorrect && translatedCorrect){
            ui->areaMaximum->setEnabled(true);
            ui->rotationAngle->setEnabled(true);
            ui->iterationCount->setEnabled(true);
        }
    }
}

void LicovaniDvou::TranslatedLE_textChanged(const QString &arg1)
{
    //QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
    //QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int translatedFrameNo = arg1.toInt();
    if (translatedFrameNo < 0 || translatedFrameNo > frameCount)
    {
        translatedNoLE->setStyleSheet("color: #FF0000");
        qDebug()<<"Snimek posunuty nelze ve videu dohledat";
        translatedCorrect = false;
        translatedNumber = -1;
    }
    else
    {
        translatedNoLE->setStyleSheet("color: #339900");
        translatedCorrect = true;
        translatedNumber = translatedFrameNo;
        if (referenceCorrect && translatedCorrect){
            ui->areaMaximum->setEnabled(true);
            ui->rotationAngle->setEnabled(true);
            ui->iterationCount->setEnabled(true);
        }
    }
}

void LicovaniDvou::on_areaMaximum_textChanged(const QString &arg1)
{
    double inputNumber = arg1.toDouble();
    if (inputNumber < 1.0 || inputNumber > 20.0)
    {
        ui->areaMaximum->setStyleSheet("color: #FF0000");
        ui->areaMaximum->setText("");
        areaMaximumCorrect = false;
        areaMaximum = -1;
    }
    else
    {
        ui->areaMaximum->setStyleSheet("color: #339900");
        areaMaximumCorrect = true;
        areaMaximum = inputNumber;
        emit checkRegistrationPass();
    }
}

void LicovaniDvou::on_rotationAngle_textChanged(const QString &arg1)
{
    double inputNumber = arg1.toDouble();
    if (inputNumber < 0.0 || inputNumber > 0.5)
    {
        ui->rotationAngle->setStyleSheet("color: #FF0000");
        angleCorrect = false;
        angle = 0.1;
    }
    else
    {
        ui->rotationAngle->setStyleSheet("color: #339900");
        angleCorrect = true;
        angle = inputNumber;
        emit checkRegistrationPass();
    }
}

void LicovaniDvou::on_iterationCount_textChanged(const QString &arg1)
{
    int inputNumber = arg1.toInt();
    if (inputNumber < -1.0 && inputNumber == 0.0)
    {
        ui->iterationCount->setStyleSheet("color: #FF0000");
        iterationCorrect = false;
        iteration = -1;
    }
    if (inputNumber == -1 || inputNumber > 1)
    {
        ui->iterationCount->setStyleSheet("QLineEdit#pocetIteraci{color: #339900}");
        iterationCorrect = true;
        if (inputNumber == -1)
            iteration = -1;
        else        
            iteration = inputNumber;
        emit checkRegistrationPass();
    }
}

void LicovaniDvou::on_registrateTwo_clicked()
{
    //this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    //MainWindow* mojeplikace = new MainWindow();
    //mojeplikace->showMinimized();
    //qDebug()<<parametry_frangi;
    //qDebug()<<parametry_frangi.length();
    QString filePath = chosenVideoAnalysis[0]+"/"+chosenVideoAnalysis[1]+"."+chosenVideoAnalysis[2];
    cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    cap.set(CV_CAP_PROP_POS_FRAMES,double(referenceNumber));
    cv::Mat referencni_snimek,posunuty;
    cap.read(referencni_snimek);
    kontrola_typu_snimku_8C3(referencni_snimek);
    qDebug()<<"reference má "<<referencni_snimek.channels()<<" kanálů a typu "<<referencni_snimek.type();
    cap.set(CV_CAP_PROP_POS_FRAMES,double(translatedNumber));
    cap.read(posunuty);
    kontrola_typu_snimku_8C3(posunuty);
    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    double entropie_posunuteho,entropie_reference;
    cv::Scalar tennengrad_posunuteho,tennengrad_reference;
    vypocet_entropie(referencni_snimek,entropie_reference,tennengrad_reference);
    vypocet_entropie(posunuty,entropie_posunuteho,tennengrad_posunuteho);
    //cout << "E: " << entropie_reference <<" T: " << tennengrad_reference[0] << endl;
    //cout << "E: " << entropie_posunuteho << " T: " << tennengrad_posunuteho[0] << endl;
    ui->ER->setText(QString::number(entropie_reference));
    ui->TR->setText(QString::number(tennengrad_reference[0]));
    ui->EM->setText(QString::number(entropie_posunuteho));
    ui->TM->setText(QString::number(tennengrad_posunuteho[0]));

    cv::Rect vyrez_korelace_extra(0,0,0,0);
    cv::Rect vyrez_korelace_standard(0,0,0,0);
    cv::Rect vyrez_anomalie(0,0,0,0);
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Mat obraz;
    if (oznacena_hranice_svetelne_anomalie.x >0.0f && oznacena_hranice_svetelne_anomalie.x < float(sirka))
    {
        verticalAnomalyCoords.x = oznacena_hranice_svetelne_anomalie.x;
        verticalAnomalyCoords.y = oznacena_hranice_svetelne_anomalie.y;
    }
    else
    {
        verticalAnomalyCoords.x = 0.0f;
        verticalAnomalyCoords.y = 0.0f;
    }
    if (oznacena_hranice_casove_znacky.y > 0.0f && oznacena_hranice_casove_znacky.y < float(vyska))
    {
        horizontalAnomalyCoords.y = oznacena_hranice_casove_znacky.y;
        horizontalAnomalyCoords.x = oznacena_hranice_casove_znacky.x;
    }
    else
    {
        horizontalAnomalyCoords.y = 0.0f;
        horizontalAnomalyCoords.x = 0.0f;
    }
    predzpracovaniKompletnihoLicovani(referencni_snimek,
                                      obraz,
                                      FrangiParametersVector,
                                      verticalAnomalyCoords,
                                      horizontalAnomalyCoords,
                                      frangiMaximumCoords,
                                      vyrez_anomalie,
                                      vyrez_korelace_extra,
                                      vyrez_korelace_standard,
                                      cap,
                                      verticalAnomalyPresent,
                                      horizontalAnomalyPresent,
                                      scaleChanged);

    int rows = referencni_snimek.rows;
    int cols = referencni_snimek.cols;
    cv::Mat obraz_32f,obraz_vyrez;
    obraz.copyTo(obraz_32f);
    //posunuty.copyTo(posunuty_32f);
    kontrola_typu_snimku_32C1(obraz_32f);
    //kontrola_typu_snimku_32C1(posunuty_32f);
    //qDebug()<<"Obraz "<<obraz.channels();
    //qDebug()<<"Posunuty"<<posunuty.channels();
    //qDebug()<<"Obraz 32"<<obraz_32f.channels();
    //qDebug()<<"Posunuty 320"<<posunuty_32f.channels();
    obraz_32f(vyrez_korelace_standard).copyTo(obraz_vyrez);
    cv::Point3d maximum_frangi_reverse = frangi_analyza(obraz,2,2,0,"",1,false,pt_temp,FrangiParametersVector);
    qDebug()<<"Maximum frangi reverse "<<maximum_frangi_reverse.x<<" "<<maximum_frangi_reverse.y;
    /// Začátek
    cv::Mat mezivysledek = cv::Mat::zeros(obraz.size(), CV_32FC3);
    cv::Point3d pt3;
    double uhel = 0;
    double celkovy_uhel = 0;
    int uspech_slicovani = kompletni_slicovani(cap,
                                               referencni_snimek,
                                               translatedNumber,
                                               iteration,
                                               areaMaximum,
                                               uhel,
                                               vyrez_korelace_extra,
                                               vyrez_korelace_standard,
                                               scaleChanged,
                                               mezivysledek,
                                               pt3,
                                               celkovy_uhel);
    /// Konec
    if (uspech_slicovani==0)
        qDebug()<<"Licovani skoncilo chybou";
    else
    {
        qDebug()<<"PT3 - posunutí po multiPOC "<<pt3.x<<" "<<pt3.y;
        cv::Mat korekce = eventualni_korekce_translace(mezivysledek,obraz,vyrez_korelace_standard,pt3,areaMaximum);
        qDebug()<<"Mezivýsledek "<<mezivysledek.channels()<<" "<<mezivysledek.type();
        cv::Point3d pt5 = fk_translace_hann(obraz,korekce);
        qDebug()<<"PT5 obraz vs korekce"<<pt5.x<<" "<<pt5.y;
        double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt5.z);
        double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
        qDebug()<<"FWHM: "<<FWHM;
        cv::Point3d pt6 = fk_translace(obraz,korekce);
        qDebug()<<"Pt6 "<<pt6.x<<" "<<pt6.y;
        cv::Point3d souradnice_slicovany_frangi_reverse = frangi_analyza(mezivysledek,2,2,0,"",2,false,pt3,FrangiParametersVector);
        //cv::Point3d souradnice_slicovany_frangi = frangi_analyza(mezivysledek,1,1,0,"",2,false,pt3,parametry_frangi);
        double yydef = maximum_frangi_reverse.x - souradnice_slicovany_frangi_reverse.x;
        double xxdef = maximum_frangi_reverse.y - souradnice_slicovany_frangi_reverse.y;
        cv::Point3d vysledne_posunuti;
        vysledne_posunuti.y = pt3.y - yydef;
        vysledne_posunuti.x = pt3.x - xxdef;
        vysledne_posunuti.z = 0;
        cv::Mat posunuty_temp2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
        cv::Mat finalni_licovani = rotace_snimku(posunuty_temp2,uhel);
        cv::Mat finalni_licovani_32f,finalni_licovani_32f_vyrez;
        finalni_licovani.copyTo(finalni_licovani_32f);
        kontrola_typu_snimku_32C1(finalni_licovani_32f);
        finalni_licovani_32f(vyrez_korelace_standard).copyTo(finalni_licovani_32f_vyrez);
        qDebug()<<"Licovani dokonceno";
        src1 = obraz;
        src2 = mezivysledek;
        kontrola_typu_snimku_8C3(src1);
        kontrola_typu_snimku_8C3(src2);

        /// originální verze z DP, která však tvoří okno ovladatelné až po ukončení samotné aplikace
        /*alpha_slider = 0;
            cv::namedWindow("Kontrola_licovani", CV_WINDOW_AUTOSIZE); // Create Window
            cv::createTrackbar("Vybrany snimek","Kontrola_licovani",&slider_position,1,on_trackbar);
            on_trackbar( alpha_slider, 0 );*/
        /// nová verze prohlížeče, která umožní ovládání bez vypnutí hlavního programu
        /*VysledekLicovani *vysledekLicovani = new VysledekLicovani();
            vysledekLicovani->setModal(true);
            vysledekLicovani->show();*/
    }
}

void LicovaniDvou::showDialog()
{
    if (ui->verticalAnomaly->isChecked())
    {
        QString fullPath = chosenVideoAnalysis[0]+"/"+chosenVideoAnalysis[1]+"."+chosenVideoAnalysis[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referenceNumber,1);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
    if (ui->horizontalAnomaly->isChecked())
    {
        QString fullPath = chosenVideoAnalysis[0]+"/"+chosenVideoAnalysis[1]+"."+chosenVideoAnalysis[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referenceNumber,2);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }

        /*QObject::connect(vyznac_anomalii,SIGNAL(SendClickCoordinates(QPointF)),
                         this,SLOT(GetClickCoordinates(QPointF)));*/
        //connect(vyznac_anomalii, vyznac_anomalii->SendClickCoordinates, (=)[const auto &myString] {ui->label->setText(myString);});
}
