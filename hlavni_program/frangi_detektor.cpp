#include "hlavni_program/frangi_detektor.h"
#include "ui_frangi_detektor.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/pouzij_frangiho.h"

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

using cv::Point3d;
using cv::VideoCapture;
using cv::Mat;
using namespace cv;

cv::Point3d detekovane_frangiho_maximum;

Frangi_detektor::Frangi_detektor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Frangi_detektor)
{
    ui->setupUi(this);
    velikost_frangi_opt(6);
    QFile soubor(QDir::currentPath()+"/"+"frangi_parametry.txt");
    bool soubor_pritomen = false;
    if (soubor.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        soubor_pritomen = true;
        QTextStream in(&soubor);
        int pocitadlo = 0;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            inicializace_frangi_opt(line,pocitadlo);
            pocitadlo+=1;
        }
    }
    if (soubor_pritomen == true){
        ui->sigma_start_DSB->setValue(data_z_frangi_opt(0));
        ui->sigma_end_DSB->setValue(data_z_frangi_opt(1));
        ui->sigma_step_DSB->setValue(data_z_frangi_opt(2));
        ui->beta_one_DSB->setValue(data_z_frangi_opt(3));
        ui->beta_two_DSB->setValue(data_z_frangi_opt(4));
        if (data_z_frangi_opt(5) == 1.0){
            ui->RB_standard->setChecked(1);
            ui->RB_reverz->setChecked(0);}
        else{
            ui->RB_standard->setChecked(0);
            ui->RB_reverz->setChecked(1);
        }
        ui->sigma_start->setValue(int(ui->sigma_start_DSB->value()/10*50));
        ui->sigma_end->setValue(int(ui->sigma_end_DSB->value()/10*50));
        ui->sigma_step->setValue(int(ui->sigma_step_DSB->value()/10*50));
        ui->beta_one->setValue(int(ui->beta_one_DSB->value()/10*50));
        ui->beta_two->setValue(int(ui->beta_two_DSB->value()/10*50));
    }
    if (soubor_pritomen == false)
    {
        ui->sigma_start_DSB->setValue(0);
        ui->sigma_end_DSB->setValue(0);
        ui->sigma_step_DSB->setValue(0);
        ui->beta_one_DSB->setValue(0);
        ui->beta_two_DSB->setValue(0);
        ui->RB_standard->setChecked(0);
        ui->RB_reverz->setChecked(0);
    }
    connect(ui->sigma_start_DSB, SIGNAL(editingFinished()),this,
            SLOT(zmena_hodnoty_slider_start()));
    connect(ui->sigma_end_DSB, SIGNAL(editingFinished()),this,
            SLOT(zmena_hodnoty_slider_end()));
    connect(ui->sigma_step_DSB, SIGNAL(editingFinished()),this,
            SLOT(zmena_hodnoty_slider_step()));
    connect(ui->beta_one_DSB, SIGNAL(editingFinished()),this,
            SLOT(zmena_hodnoty_slider_one()));
    connect(ui->beta_two_DSB, SIGNAL(editingFinished()),this,
            SLOT(zmena_hodnoty_slider_two()));
    ui->cisloSnimku->setEnabled(false);
    ui->Frangi_filtr->setEnabled(false);
}

Frangi_detektor::~Frangi_detektor()
{
    delete ui;
}

void Frangi_detektor::inicializace_frangi_opt(QString &hodnota, int &pozice)
{
    FrangiParametrySouboru[pozice] = hodnota.toDouble();
}

double Frangi_detektor::data_z_frangi_opt(int pozice)
{
    return FrangiParametrySouboru[pozice];
}
void Frangi_detektor::velikost_frangi_opt(int velikost){
    FrangiParametrySouboru = (QVector<double>(velikost));
}

void Frangi_detektor::on_sigma_start_sliderMoved(int value)
{
    double hodnota = (value/50.0)*10.0;
    ui->sigma_start_DSB->setValue(hodnota);
}

void Frangi_detektor::on_sigma_end_sliderMoved(int value)
{
    double hodnota = (value/50.0)*10.0;
    ui->sigma_end_DSB->setValue(hodnota);
}

void Frangi_detektor::on_sigma_step_sliderMoved(int value)
{
    double hodnota = (value/50.0)*10.0;
    ui->sigma_step_DSB->setValue(hodnota);
}

void Frangi_detektor::on_beta_one_sliderMoved(int value)
{
    double hodnota = (value/50.0)*10.0;
    ui->beta_one_DSB->setValue(hodnota);
}

void Frangi_detektor::on_beta_two_sliderMoved(int value)
{
    double hodnota = (value/50.0)*10.0;
    ui->beta_two_DSB->setValue(hodnota);
}

void Frangi_detektor::zmena_hodnoty_slider_start(){
    double hodnota = ui->sigma_start_DSB->value();
    int hodnota_prepocitana = int((hodnota/10)*50);
    ui->sigma_start->setValue(hodnota_prepocitana);
}
void Frangi_detektor::zmena_hodnoty_slider_end(){
    double hodnota = ui->sigma_end_DSB->value();
    int hodnota_prepocitana = int((hodnota/10.0)*50.0);
    ui->sigma_end->setValue(hodnota_prepocitana);
}
void Frangi_detektor::zmena_hodnoty_slider_step(){
    double hodnota = ui->sigma_step_DSB->value();
    int hodnota_prepocitana = int((hodnota/10.0)*50.0);
    ui->sigma_step->setValue(hodnota_prepocitana);
}
void Frangi_detektor::zmena_hodnoty_slider_one(){
    double hodnota = ui->beta_one_DSB->value();
    int hodnota_prepocitana = int((hodnota/10.0)*50.0);
    ui->beta_one->setValue(hodnota_prepocitana);
}
void Frangi_detektor::zmena_hodnoty_slider_two(){
    double hodnota = ui->beta_two_DSB->value();
    int hodnota_prepocitana = int((hodnota/10.0)*50.0);
    ui->beta_two->setValue(hodnota_prepocitana);
}

void Frangi_detektor::on_Frangi_filtr_clicked()
{
    double beta_two = ui->beta_one_DSB->value();
    double beta_one = ui->beta_one_DSB->value();
    double sigma_start = ui->sigma_start_DSB->value();
    double sigma_end = ui->sigma_end_DSB->value();
    double sigma_step = ui->sigma_step_DSB->value();
    int standardInt=0;
    if(ui->RB_standard->isChecked()){standardInt = 1;};
    QVector<double> parametry_pro_frangiFiltr = {sigma_start,sigma_step,sigma_end,beta_one,beta_two,double(standardInt)};
    qDebug()<<parametry_pro_frangiFiltr;
    qDebug()<<FrangiParametrySouboru.length();
    bool standard = ui->RB_standard->isChecked();
    int typ_zpracovani;
    if (standard == true)
    {
        typ_zpracovani = 1;
    }
    else
    {
        typ_zpracovani = 2;
    }
    Mat vybrany_snimekFrangi;
    if (rozborVybranehoSouboru[2] == "avi")
    {
        QString vybrany_soubor = rozborVybranehoSouboru[0]+"/"+rozborVybranehoSouboru[1]+"."+rozborVybranehoSouboru[2];
        VideoCapture cap = VideoCapture(vybrany_soubor.toLocal8Bit().constData()); // konverze z QString do string
        if (!cap.isOpened())
        {
            qDebug()<<"video nelze otevrit pro potreby zpracovani"<<endl;
        }
        bool kontrola_zadani_cisla;
        int cislo_snimku = ui->cisloSnimku->text().toInt(&kontrola_zadani_cisla);
        cap.set(CV_CAP_PROP_POS_FRAMES,cislo_snimku);
        if(!cap.read(vybrany_snimekFrangi))
        {
            qDebug()<<"Zvolený snímek nebyl ve videu nalezen";
        }
    }
    /***********************************************************************************************/
    /***********************************************************************************************/
    //ukaz_Mat("Vybrany snimek pro Frangiho filtru",vybrany_snimekFrangi);
    //qDebug()<<vybrany_snimekFrangi.channels()<<" "<<vybrany_snimekFrangi.type();
    /* v tuto chvíli je po načtení channels 1 a typ 0 */

    kontrola_typu_snimku_8C3(vybrany_snimekFrangi);

    //qDebug()<<vybrany_snimekFrangi.channels()<<" "<<vybrany_snimekFrangi.type();
    /* nyní je všechno v pořádku, channels 3 a typ 16 */
    /***********************************************************************************************/
    /***********************************************************************************************/

   Point3d pt_temp(0,0,0);
   detekovane_frangiho_maximum = frangi_analyza(vybrany_snimekFrangi,
                                               typ_zpracovani,1,1,"Frangi zvoleneho snimku",1,false,
                                               pt_temp,parametry_pro_frangiFiltr);
   qDebug()<<"Detekovane maximum Frangiho filtru je "<<detekovane_frangiho_maximum.x<<" "<<detekovane_frangiho_maximum.y;

    /*
   Mat filtrovany;
   medianBlur(vybrany_snimekFrangi,filtrovany,5);
   imshow("Filtrovany",filtrovany);
    */


    /*cv::Mat src, J, Scale, Directions;
   frangi2d_opts_t opts;
   frangi2d_createopts(&opts);
   if (typ_zpracovani == 2)
   {
       opts.BlackWhite = false;
   }
   else if(typ_zpracovani == 1)
   {
       opts.BlackWhite = true;
   }
   opts.sigma_start = int(parametry_pro_frangiFiltr[0]);
   opts.sigma_step = int(parametry_pro_frangiFiltr[1]);
   opts.sigma_end = int(parametry_pro_frangiFiltr[2]);
   opts.BetaOne = parametry_pro_frangiFiltr[3];
   opts.BetaTwo = parametry_pro_frangiFiltr[4];
   kontrola_typu_snimku_32(vybrany_snimekFrangi);
   frangi2d(vybrany_snimekFrangi,J,Scale,Directions,opts);
   imshow("Frangi",J);*/
}

void Frangi_detektor::on_souborKAnalyzePB_clicked()
{
    QString videoProFrangiFiltr = QFileDialog::getOpenFileName(this,
       "Vyberte snímek pro Frangiho filtr", "","(*.avi);;Všechny soubory (*)");
    int lastindexSlash = videoProFrangiFiltr.lastIndexOf("/");
    int lastIndexComma = videoProFrangiFiltr.length() - videoProFrangiFiltr.lastIndexOf(".");
    QString cesta_k_souborum = videoProFrangiFiltr.left(lastindexSlash);
    QString vybrany_soubor = videoProFrangiFiltr.mid(lastindexSlash+1,
                                                     (videoProFrangiFiltr.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = videoProFrangiFiltr.right(lastIndexComma-1);
    rozborVybranehoSouboru.push_back(cesta_k_souborum);
    rozborVybranehoSouboru.push_back(vybrany_soubor);
    rozborVybranehoSouboru.push_back(koncovka);
    QLineEdit cislo_snimku;
    if (koncovka == "avi"){
        this->ui->cisloSnimku->setEnabled(true);
        ui->vybranySoubor->setText(rozborVybranehoSouboru[1]);
        ui->Frangi_filtr->setEnabled(true);
    }

}

/******************************************************************************************/
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
