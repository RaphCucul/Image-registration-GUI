#include "hlavni_program/frangi_detektor.h"
#include "fancy_staff/sharedvariables.h"
#include "ui_frangi_detektor.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "util/souborove_operace.h"
#include "util/prace_s_vektory.h"
#include "dialogy/errordialog.h"

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
#include <QSlider>
#include <QVector>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QDebug>
#include <QJsonDocument>

using cv::Point3d;
using cv::VideoCapture;
using cv::Mat;
using namespace cv;

Frangi_detektor::Frangi_detektor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Frangi_detektor)
{
    ui->setupUi(this);

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    ui->saveParameters->setText(tr("Save Frangi parameters"));
    ui->fileToAnalyse->setText(tr("Choose file"));
    ui->frameNumber->setPlaceholderText(tr("Choose frame number"));
    ui->chosenFile->setPlaceholderText(tr("Chosen file"));
    ui->RB_standard->setText(tr("Standard Frangi mode"));
    ui->RB_reverz->setText(tr("Reverz Frangi mode"));

    ui->frameNumber->setEnabled(false);
    ui->Frangi_filtr->setEnabled(false);

    spinBoxes["sigma_start"] = ui->sigma_start_DSB;
    spinBoxes["sigma_end"] = ui->sigma_end_DSB;
    spinBoxes["sigma_step"] = ui->sigma_step_DSB;
    spinBoxes["beta_one"] = ui->beta_one_DSB;
    spinBoxes["beta_two"] = ui->beta_two_DSB;

    sliders["sigma_start"] = ui->sigma_start;
    sliders["sigma_end"] = ui->sigma_end;
    sliders["sigma_step"] = ui->sigma_step;
    sliders["beta_one"] = ui->beta_one;
    sliders["beta_two"] = ui->beta_two;

    localErrorDialogHandling[ui->saveParameters] = new ErrorDialog(ui->saveParameters);
    localErrorDialogHandling[ui->Frangi_filtr] = new ErrorDialog(ui->Frangi_filtr);
    localErrorDialogHandling[ui->chosenFile] = new ErrorDialog(ui->chosenFile);
    for (int index = 0; index < frangiParametersList.count(); index++){
        localErrorDialogHandling[spinBoxes[frangiParametersList.at(index)]] = new ErrorDialog(spinBoxes[frangiParametersList.at(index)]);
    }

    connect(ui->sigma_start_DSB, SIGNAL(editingFinished()),this,
            SLOT(changeValue_slider_start()));
    connect(ui->sigma_end_DSB, SIGNAL(editingFinished()),this,
            SLOT(changeValue_slider_end()));
    connect(ui->sigma_step_DSB, SIGNAL(editingFinished()),this,
            SLOT(changeValue_slider_step()));
    connect(ui->beta_one_DSB, SIGNAL(editingFinished()),this,
            SLOT(changeValue_slider_one()));
    connect(ui->beta_two_DSB, SIGNAL(editingFinished()),this,
            SLOT(changeValue_slider_two()));
}

Frangi_detektor::~Frangi_detektor()
{
    delete ui;
}

void Frangi_detektor::setDSBInput(QDoubleSpinBox * _spinbox,QString parameter){
    _spinbox->setValue(SharedVariables::getSharedVariables()->getSpecificFrangiParameter(parameter));
}

void Frangi_detektor::analyseSliderInput(QSlider *_slider, QString parameter){
    _slider->setValue(int(spinBoxes[parameter]->value()/10*50));
}

void Frangi_detektor::setParametersToUI(){
    for (int index = 0; index < (frangiParametersList.count()-1); index++){
        setDSBInput(spinBoxes[frangiParametersList.at(index)],frangiParametersList.at(index));
    }

    if (SharedVariables::getSharedVariables()->getSpecificFrangiParameter(5) == 1.0){
        ui->RB_standard->setChecked(1);
        ui->RB_reverz->setChecked(0);}
    else{
        ui->RB_standard->setChecked(0);
        ui->RB_reverz->setChecked(1);
    }

    for (int index = 0; index < (frangiParametersList.count()-1); index++){
        analyseSliderInput(sliders[frangiParametersList.at(index)],frangiParametersList.at(index));
    }
    ui->chosenFile->setEnabled(true);
}

void Frangi_detektor::checkPaths()
{
    if (SharedVariables::getSharedVariables()->getPath("cestaKvideim") == "")
        ui->chosenFile->setPlaceholderText(tr("Chosen video"));
    else
    {
        QString folder,filename,suffix;
        QStringList foundFiles;
        int foundCount;
        QString pathToVideos = SharedVariables::getSharedVariables()->getPath("cestaKvideim");
        analyseFileNames(pathToVideos,foundFiles,foundCount,"avi");
        if (foundCount != 0)
        {
            QString celefilename = pathToVideos+"/"+foundFiles.at(0);
            processFilePath(celefilename,folder,filename,suffix);
            if (analyseChosenFile.length() == 0)
            {
                analyseChosenFile.push_back(folder);
                analyseChosenFile.push_back(filename);
                analyseChosenFile.push_back(suffix);
            }
            else
            {
                analyseChosenFile.clear();
                analyseChosenFile.push_back(folder);
                analyseChosenFile.push_back(filename);
                analyseChosenFile.push_back(suffix);
            }
            ui->chosenFile->setText(filename);
        }
        else{
            localErrorDialogHandling[ui->chosenFile]->evaluate("left","hardError",3);
            localErrorDialogHandling[ui->chosenFile]->show();
        }
    }
    setParametersToUI();
}

void Frangi_detektor::on_sigma_start_sliderMoved(int value)
{
    double valueD = (value/50.0)*10.0;
    ui->sigma_start_DSB->setValue(valueD);
}

void Frangi_detektor::on_sigma_end_sliderMoved(int value)
{
    double valueD = (value/50.0)*10.0;
    ui->sigma_end_DSB->setValue(valueD);
}

void Frangi_detektor::on_sigma_step_sliderMoved(int value)
{
    double valueD = (value/50.0)*10.0;
    ui->sigma_step_DSB->setValue(valueD);
}

void Frangi_detektor::on_beta_one_sliderMoved(int value)
{
    double valueD = (value/50.0)*10.0;
    ui->beta_one_DSB->setValue(valueD);
}

void Frangi_detektor::on_beta_two_sliderMoved(int value)
{
    double valueD = (value/50.0)*10.0;
    ui->beta_two_DSB->setValue(valueD);
}

void Frangi_detektor::setSliderValue(QDoubleSpinBox *_spinbox, QSlider* _slider){
    double value = _spinbox->value();
    int recalculated = int((value/10)*50);
    _slider->setValue(recalculated);
}

void Frangi_detektor::changeValue_slider_start(){
    setSliderValue(ui->sigma_start_DSB,ui->sigma_start);
}
void Frangi_detektor::changeValue_slider_end(){
    setSliderValue(ui->sigma_end_DSB,ui->sigma_end);
}
void Frangi_detektor::changeValue_slider_step(){
    setSliderValue(ui->sigma_step_DSB,ui->sigma_step);
}
void Frangi_detektor::changeValue_slider_one(){
    setSliderValue(ui->beta_one_DSB,ui->beta_one);
}
void Frangi_detektor::changeValue_slider_two(){
    setSliderValue(ui->beta_two_DSB,ui->beta_two);
}

void Frangi_detektor::on_Frangi_filtr_clicked()
{
    if (SharedVariables::getSharedVariables()->getPath("parametryFrangiFiltr") == ""){// || vectorSum(FrangiParametersVector)==0.0)
        localErrorDialogHandling[ui->Frangi_filtr]->evaluate("left","hardError",5);
        localErrorDialogHandling[ui->Frangi_filtr]->show();
    }
    else{
        double beta_two = ui->beta_one_DSB->value();
        double beta_one = ui->beta_one_DSB->value();
        double sigma_start = ui->sigma_start_DSB->value();
        double sigma_end = ui->sigma_end_DSB->value();
        double sigma_step = ui->sigma_step_DSB->value();
        int standardInt=0;
        if(ui->RB_standard->isChecked())
            standardInt = 1;
        QVector<double> parametersForFrangi = {sigma_start,sigma_step,sigma_end,beta_one,beta_two,double(standardInt)};
        qDebug()<<parametersForFrangi;
        //qDebug()<<FrangiParametersVector.length();
        bool standard = ui->RB_standard->isChecked();
        int processingType;
        if (standard)
            processingType = 1;
        else
            processingType = 2;

        Mat chosenFrame;
        if (analyseChosenFile[2] == "avi")
        {
            QString chosenFile = analyseChosenFile[0]+"/"+analyseChosenFile[1]+"."+analyseChosenFile[2];
            VideoCapture cap = VideoCapture(chosenFile.toLocal8Bit().constData()); // konverze z QString do string
            if (!cap.isOpened()){
                localErrorDialogHandling[ui->Frangi_filtr]->evaluate("left","hardError",6);
                return;
            }            
            cap.set(CV_CAP_PROP_POS_FRAMES,analyseFrame);
            cap.read(chosenFrame);
        }
        kontrola_typu_snimku_8C3(chosenFrame);
        Point3d pt_temp(0.0,0.0,0.0);
        cv::Point3d detectedFrangi = frangi_analysis(chosenFrame,
                                                     processingType,1,1,tr("Frangi of chosen frame"),1,pt_temp,
                                                     parametersForFrangi);
        qDebug()<<"Frangi maximum detected "<<detectedFrangi.x<<" "<<detectedFrangi.y;
        SharedVariables::getSharedVariables()->setFrangiMaximum(detectedFrangi);
        /*cv::Mat src, J, Scale, Directions;
   frangi2d_opts_t opts;
   frangi2d_createopts(&opts);
   if (processingType == 2)
   {
       opts.BlackWhite = false;
   }
   else if(processingType == 1)
   {
       opts.BlackWhite = true;
   }
   opts.sigma_start = int(parametersForFrangi[0]);
   opts.sigma_step = int(parametersForFrangi[1]);
   opts.sigma_end = int(parametersForFrangi[2]);
   opts.BetaOne = parametersForFrangi[3];
   opts.BetaTwo = parametersForFrangi[4];
   kontrola_typu_snimku_32(chosenFrame);
   frangi2d(chosenFrame,J,Scale,Directions,opts);
   imshow("Frangi",J);*/
    }
}

void Frangi_detektor::on_fileToAnalyse_clicked()
{
    QString videoProFrangiFiltr = QFileDialog::getOpenFileName(this,
       tr("Choose frame for Frangi filter analysis"), SharedVariables::getSharedVariables()->getPath("cestaKvideim"),"(*.avi);;All files (*)");
    QString folder,filename,suffix;
    processFilePath(videoProFrangiFiltr,folder,filename,suffix);
    if (analyseChosenFile.length() == 0)
    {
        analyseChosenFile.push_back(folder);
        analyseChosenFile.push_back(filename);
        analyseChosenFile.push_back(suffix);
    }
    else
    {
        analyseChosenFile.clear();
        analyseChosenFile.push_back(folder);
        analyseChosenFile.push_back(filename);
        analyseChosenFile.push_back(suffix);
    }
    if (suffix == "avi"){
        this->ui->frameNumber->setEnabled(true);
        ui->chosenFile->setText(analyseChosenFile[1]);
    }

}

void Frangi_detektor::on_frameNumber_textChanged(const QString &arg1)
{
    QString fullPath = analyseChosenFile[0]+"/"+analyseChosenFile[1]+"."+analyseChosenFile[2];
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    bool checkInput;
    int frameNumber = ui->frameNumber->text().toInt(&checkInput);
    if (arg1.toInt() <= 0 || arg1.toInt() > frameCount || !checkInput){
        ui->frameNumber->setStyleSheet("color: #FF0000");
        ui->Frangi_filtr->setEnabled(false);
    }
    else
    {
        ui->frameNumber->setStyleSheet("color: #339900");
        analyseFrame = frameNumber-1;
        ui->Frangi_filtr->setEnabled(true);
    }
}

void Frangi_detektor::on_chosenFile_textChanged(const QString &arg1)
{
    QString fullPath = analyseChosenFile[0]+"/"+arg1+"."+analyseChosenFile[2];
    actualVideo = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    if (!actualVideo.isOpened()){
        ui->chosenFile->setStyleSheet("color: #FF0000");
        ui->frameNumber->setText("");
        ui->frameNumber->setEnabled(false);
    }
    else
    {
        ui->chosenFile->setStyleSheet("color: #339933");
        analyseChosenFile[1] = arg1;
        ui->frameNumber->setEnabled(true);
    }
}

void Frangi_detektor::getDSBInput(QString parameter){
    if (spinBoxes[parameter]->value() != 0.0){
        SharedVariables::getSharedVariables()->setSpecificFrangiParameter(frangiParametersList.indexOf(parameter),
                                                                          spinBoxes[parameter]->value());
    }
    else{
        localErrorDialogHandling[ui->saveParameters]->evaluate("left","hardError",4);
        localErrorDialogHandling[ui->saveParameters]->show();
    }
}

void Frangi_detektor::on_saveParameters_clicked()
{
    for (int index = 0; index < (frangiParametersList.count()-1); index++){
        getDSBInput(frangiParametersList.at(index));
    }
    if (ui->RB_standard->isChecked())
        SharedVariables::getSharedVariables()->setSpecificFrangiParameter(5,1.0);
    else
        SharedVariables::getSharedVariables()->setSpecificFrangiParameter(5,0.0);
    SharedVariables::getSharedVariables()->saveFrangiParameters();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Návod pro práci s OpenCV v Qt - je třeba neustále kontrolovat channels a type Mat objektů
// podle požadavků jednotlivých funkcí!!
// Například načtení snímku z videa místo 3 channel 8U plive 8UC1
/*cv::Mat Frangi_detektor::filtrace_obrazu(cv::Mat vstupni_snimek)
{
    cv::Mat vystupni_obraz,medianFilter,RF;
    cv::medianBlur(vstupni_snimek,medianFilter,5);
    qDebug()<<medianFilter.channels()<<" "<<medianFilter.type();
    kontrola_typu_snimku_8C3(medianFilter);
    qDebug()<<medianFilter.channels()<<" "<<medianFilter.type();
    cv::edgePreservingFilter(medianFilter,RF);
    ukaz_Mat("MedianFilter",medianFilter);
    imshow("RF",RF);*/
/*if (RF.channels() == 3)
    {
        std::vector<cv::Mat> channels(3);
        cv::split(RF,channels);
        vystupni_obraz = channels[1];
        vystupni_obraz.convertTo(vystupni_obraz,CV_32FC1);
    }
    else
    {
        vystupni_obraz = RF;
    }
    return vystupni_obraz;*/
/*return RF;
}*/




