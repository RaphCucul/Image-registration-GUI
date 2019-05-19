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

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    ui->saveParameters->setText(tr("Save Frangi parameters"));
    ui->fileToAnalyse->setText(tr("Choose file"));
    ui->frameNumber->setPlaceholderText(tr("Frame"));
    ui->chosenFile->setPlaceholderText(tr("Chosen file"));
    ui->RB_standard->setText(tr("Standard Frangi mode"));
    ui->RB_reverz->setText(tr("Reverz Frangi mode"));

    ui->frameNumber->setEnabled(false);
    ui->Frangi_filtr->setEnabled(true);

    initWidgetHashes();

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
    QObject::connect(ui->leftMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->rightMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->topMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->bottomMargin,SIGNAL(valueChanged(int)),this,SLOT(processMargins(int)));
    QObject::connect(ui->leftRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    QObject::connect(ui->rightRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    QObject::connect(ui->topRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
    QObject::connect(ui->bottomRatio,SIGNAL(valueChanged(double)),this,SLOT(processRatio(double)));
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

    frangiMargins = SharedVariables::getSharedVariables()->getFrangiMargins();
    cutoutRatios = SharedVariables::getSharedVariables()->getFrangiRatios();

    for (int hashIndex = 0; hashIndex < MarginsRatios.count(); hashIndex++) {
        if (hashIndex < 4)
            marginSpinBoxes[MarginsRatios.at(hashIndex)]->setValue(frangiMargins[MarginsRatios.at(hashIndex)]);
        else
            ratioSpinBoxes[MarginsRatios.at(hashIndex)]->setValue(cutoutRatios[MarginsRatios.at(hashIndex)]);
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
    loading = false;
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
    if (readyToCalculate){
        if (SharedVariables::getSharedVariables()->getPath("parametryFrangiFiltr") == ""){// || vectorSum(FrangiParametersVector)==0.0)
            localErrorDialogHandling[ui->Frangi_filtr]->evaluate("left","hardError",5);
            localErrorDialogHandling[ui->Frangi_filtr]->show();
        }
        else{
            emit calculationStarted();
            double beta_two = ui->beta_one_DSB->value();
            double beta_one = ui->beta_one_DSB->value();
            double sigma_start = ui->sigma_start_DSB->value();
            double sigma_end = ui->sigma_end_DSB->value();
            double sigma_step = ui->sigma_step_DSB->value();
            int standardInt=0;
            if(ui->RB_standard->isChecked())
                standardInt = 1;
            QVector<double> parametersForFrangi = {sigma_start,sigma_step,sigma_end,beta_one,beta_two,double(standardInt)};
            qDebug()<<"Frangi parameters: "<<parametersForFrangi;
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
            transformMatTypeTo8C3(chosenFrame);
            Point3d pt_temp(0.0,0.0,0.0);
            cv::Point3d detectedFrangi = frangi_analysis(chosenFrame,
                                                         processingType,1,1,tr("Frangi of chosen frame"),1,pt_temp,
                                                         parametersForFrangi,
                                                         SharedVariables::getSharedVariables()->getFrangiMargins());
            qDebug()<<"Frangi maximum detected "<<detectedFrangi.x<<" "<<detectedFrangi.y;
            SharedVariables::getSharedVariables()->setFrangiMaximum(detectedFrangi);
            showStandardCutout(chosenFrame);
            calculationStopped();
        }
    }
    else{
        localErrorDialogHandling[ui->Frangi_filtr]->evaluate("left","info",2);
        localErrorDialogHandling[ui->Frangi_filtr]->show();
    }
}

void Frangi_detektor::showStandardCutout(Mat &i_chosenFrame){
    cv::Point3d frangi_point = SharedVariables::getSharedVariables()->getFrangiMaximum();
    QMap<QString,double> frangiRatios = SharedVariables::getSharedVariables()->getFrangiRatios();
    cv::Rect cutoutStandard;
    cutoutStandard.x = int(round(frangi_point.x-frangiRatios["left_r"]*(frangi_point.x)));
    cutoutStandard.y = int(round(frangi_point.y-frangiRatios["top_r"]*frangi_point.y));
    int rowTo = int(round(frangi_point.y+frangiRatios["bottom_r"]*(i_chosenFrame.rows - frangi_point.y)));
    int columnTo = int(round(frangi_point.x+frangiRatios["right_r"]*(i_chosenFrame.cols - frangi_point.x)));
    cutoutStandard.width = columnTo-cutoutStandard.x;
    cutoutStandard.height = rowTo - cutoutStandard.y;

    cv::rectangle(i_chosenFrame, cutoutStandard, Scalar(255), 1, 8, 0);
    cv::imshow("Chosen frame with standard cutout area",i_chosenFrame);
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
    int frameNumber = ui->frameNumber->text().toInt(&checkInput)-1;
    if (arg1.toInt() <= 0 || arg1.toInt() > frameCount || !checkInput){
        ui->frameNumber->setStyleSheet("color: #FF0000");
        readyToCalculate = false;
    }
    else
    {
        ui->frameNumber->setStyleSheet("color: #339900");
        analyseFrame = frameNumber-1;
        readyToCalculate = true;
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

        SharedVariables::getSharedVariables()->setFrangiMargins(frangiMargins);
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

        SharedVariables::getSharedVariables()->setFrangiRatios(cutoutRatios);
    }
}

void Frangi_detektor::initWidgetHashes(){
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

    marginSpinBoxes["top_m"] = ui->topMargin;
    marginSpinBoxes["bottom_m"] = ui->bottomMargin;
    marginSpinBoxes["left_m"] = ui->leftMargin;
    marginSpinBoxes["right_m"] = ui->rightMargin;

    ratioSpinBoxes["top_r"] = ui->topRatio;
    ratioSpinBoxes["bottom_r"] = ui->bottomRatio;
    ratioSpinBoxes["left_r"] = ui->leftRatio;
    ratioSpinBoxes["right_r"] = ui->rightRatio;
}
