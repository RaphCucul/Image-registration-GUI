#include "analyza_obrazu/pouzij_frangiho.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "analyza_obrazu/upravy_obrazu.h"
#include <vector>
#include <math.h>
#include <string>
#include <QDebug>
#include <QVector>
using namespace cv;
cv::Mat predzpracovani_obrazu(const cv::Mat& obrazek_vstupni,float sigma_s,float sigma_r)
{
    Mat filtrovany = cv::Mat::zeros(obrazek_vstupni.rows,obrazek_vstupni.cols,CV_8UC3);
    Mat obrazek_vystupni;
    medianBlur(obrazek_vstupni,filtrovany,5);    
    Mat RF_obraz = cv::Mat::zeros(obrazek_vstupni.rows,obrazek_vstupni.cols,CV_8UC3);
    kontrola_typu_snimku_8C3(filtrovany);
    edgePreservingFilter(filtrovany,RF_obraz,1,sigma_s,sigma_r);
    //imshow("Filtrovany RF",RF_obraz);
    if (RF_obraz.channels() == 3)
    {
        Mat ch1, ch3;
        std::vector<Mat> channels(3);
        split(RF_obraz, channels);
        ch1 = channels[0];
        obrazek_vystupni = channels[1];
        ch3 = channels[2];
    }
    else{
        RF_obraz.copyTo(obrazek_vystupni);
    }
    //imshow("Vybrany snimek",obrazek_vystupni);
    //qDebug()<<"Pocet kanalu vystupniho filtrovaneho snimku je "<<obrazek_vystupni.channels();
    kontrola_typu_snimku_8C1(obrazek_vystupni);
    return obrazek_vystupni;
}

void osetreni_okraju_snimku(cv::Mat &vstupni_obraz,int typ_snimku, int velikost_okraje_r,int velikost_okraje_s)
{
    if (typ_snimku == 1)
    {
        Rect oblast1(0,0,vstupni_obraz.cols,velikost_okraje_r); // prvnich 20 radku
        Mat oblast11 = vstupni_obraz(oblast1);
        Scalar tempVal = mean(oblast11);
        double prumer_oblasti_11 = tempVal.val[0];
        vstupni_obraz(oblast1).setTo(prumer_oblasti_11,vstupni_obraz(oblast1) > prumer_oblasti_11);
    /*****************************************************************************************************/
        Rect oblast2(0,vstupni_obraz.rows-velikost_okraje_r,vstupni_obraz.cols,velikost_okraje_r); // poslednich 20 radku
        Mat oblast22 = vstupni_obraz(oblast2);
        tempVal = mean(oblast22);
        double prumer_oblasti_22 = tempVal.val[0];
        vstupni_obraz(oblast2).setTo(prumer_oblasti_22,vstupni_obraz(oblast2) > prumer_oblasti_22);
    /*****************************************************************************************************/
        Rect oblast3(0,0,velikost_okraje_s,vstupni_obraz.rows); // prvnich 20 sloupcu
        Mat oblast33 = vstupni_obraz(oblast3);
        tempVal = mean(oblast33);
        double prumer_oblasti_33 = tempVal.val[0];
        vstupni_obraz(oblast3).setTo(prumer_oblasti_33,vstupni_obraz(oblast3) > prumer_oblasti_33);
    /*****************************************************************************************************/
        Rect oblast4(vstupni_obraz.cols-velikost_okraje_s,0,velikost_okraje_s,vstupni_obraz.rows); // poslednich 20 sloupcu
        Mat oblast44 = vstupni_obraz(oblast4);
        tempVal = mean(oblast44);
        double prumer_oblasti_44 = tempVal.val[0];
        vstupni_obraz(oblast4).setTo(prumer_oblasti_44,vstupni_obraz(oblast4) > prumer_oblasti_44);
    }
    if (typ_snimku == 2)
    {
        if (velikost_okraje_r <= 0)
        {
            Rect oblast1(0,0,vstupni_obraz.cols,std::abs(velikost_okraje_r)); // prvnich 20 radku
            Mat oblast11 = vstupni_obraz(oblast1);
            Scalar tempVal = mean(oblast11);
            double prumer_oblasti_11 = tempVal.val[0];
            vstupni_obraz(oblast1).setTo(prumer_oblasti_11,vstupni_obraz(oblast1) > prumer_oblasti_11);
        }
        if (velikost_okraje_r >= 0)
        {
            Rect oblast2(0,vstupni_obraz.rows-std::abs(velikost_okraje_r),vstupni_obraz.cols,std::abs(velikost_okraje_r)); // poslednich 20 radku
            Mat oblast22 = vstupni_obraz(oblast2);
            Scalar tempVal = mean(oblast22);
            double prumer_oblasti_22 = tempVal.val[0];
            vstupni_obraz(oblast2).setTo(prumer_oblasti_22,vstupni_obraz(oblast2) > prumer_oblasti_22);
        }
        if (velikost_okraje_s <= 0)
        {
            Rect oblast3(0,0,std::abs(velikost_okraje_s),vstupni_obraz.rows); // prvnich 20 sloupcu
            Mat oblast33 = vstupni_obraz(oblast3);
            Scalar tempVal = mean(oblast33);
            double prumer_oblasti_33 = tempVal.val[0];
            vstupni_obraz(oblast3).setTo(prumer_oblasti_33,vstupni_obraz(oblast3) > prumer_oblasti_33);
        }
        if (velikost_okraje_s >= 0)
        {
            Rect oblast4(vstupni_obraz.cols-std::abs(velikost_okraje_s),0,std::abs(velikost_okraje_s),vstupni_obraz.rows); // poslednich 20 sloupcu
            Mat oblast44 = vstupni_obraz(oblast4);
            Scalar tempVal = mean(oblast44);
            double prumer_oblasti_44 = tempVal.val[0];
            vstupni_obraz(oblast4).setTo(prumer_oblasti_44,vstupni_obraz(oblast4) > prumer_oblasti_44);
        }
    }
    kontrola_typu_snimku_32C1(vstupni_obraz);
    /*rectangle(vstupni_obraz, oblast1, Scalar(255), 1, 8, 0);
    rectangle(vstupni_obraz, oblast2, Scalar(255), 1, 8, 0);
    rectangle(vstupni_obraz, oblast3, Scalar(255), 1, 8, 0);
    rectangle(vstupni_obraz, oblast4, Scalar(255), 1, 8, 0);*/
    //imshow("Okraje",vstupni_obraz);
}


void nulovani_okraju(cv::Mat& vstupni_obraz,int typ_snimku,int velikost_okraje_r,int velikost_okraje_s)
{
    if (typ_snimku == 1)
    {
        Rect oblast1(0,0,vstupni_obraz.cols,velikost_okraje_r); // prvnich 20 radku
        vstupni_obraz(oblast1).setTo(0);
        /**************************************************************************/
        Rect oblast2(0,vstupni_obraz.rows-velikost_okraje_r,vstupni_obraz.cols,velikost_okraje_r); // poslednich 20 radku
        vstupni_obraz(oblast2).setTo(0);
        /**************************************************************************/
        Rect oblast3(0,0,velikost_okraje_s,vstupni_obraz.rows); // prvnich 20 sloupcu
        vstupni_obraz(oblast3).setTo(0);
        /**************************************************************************/
        Rect oblast4(vstupni_obraz.cols-velikost_okraje_s,0,velikost_okraje_s,vstupni_obraz.rows); // poslednich 20 sloupcu
        vstupni_obraz(oblast4).setTo(0);
    }
    if (typ_snimku == 2)
    {
        if (velikost_okraje_r <= 0)
        {
            Rect oblast1(0,0,vstupni_obraz.cols,std::abs(velikost_okraje_r)+40); // prvnich 20 radku
            vstupni_obraz(oblast1).setTo(0);
            Rect oblast2(0,vstupni_obraz.rows-40,vstupni_obraz.cols,40); // poslednich 20 radku
            vstupni_obraz(oblast2).setTo(0);
        }
        if (velikost_okraje_r >= 0)
        {
            Rect oblast2(0,vstupni_obraz.rows-std::abs(velikost_okraje_r)-40,
                         vstupni_obraz.cols,std::abs(velikost_okraje_r)+40); // poslednich 20 radku
            vstupni_obraz(oblast2).setTo(0);
            Rect oblast1(0,0,vstupni_obraz.cols,40); // prvnich 20 radku
            vstupni_obraz(oblast1).setTo(0);
        }
        if (velikost_okraje_s <= 0)
        {
            Rect oblast3(0,0,std::abs(velikost_okraje_s)+40,vstupni_obraz.rows); // prvnich 20 sloupcu
            vstupni_obraz(oblast3).setTo(0);
            Rect oblast4(vstupni_obraz.cols-40,0,40,vstupni_obraz.rows); // poslednich 20 sloupcu
            vstupni_obraz(oblast4).setTo(0);
        }
        if (velikost_okraje_s >= 0)
        {
            Rect oblast4(vstupni_obraz.cols-std::abs(velikost_okraje_s)-40,0,
                         std::abs(velikost_okraje_s)+40,vstupni_obraz.rows); // poslednich 20 sloupcu
            vstupni_obraz(oblast4).setTo(0);
            Rect oblast3(0,0,40,vstupni_obraz.rows); // prvnich 20 sloupcu
            vstupni_obraz(oblast3).setTo(0);
        }
        //imshow("Nulovani",vstupni_obraz);
    }
}


cv::Point2d vypocet_teziste_frangi(const cv::Mat &frangi,
                                   const double& maximum_frangi,
                                   const cv::Point& souradnice_maxima_frangi)
{
    Point2d vystup(0,0);
    int pocitadlo = 0;
    float euklid = 0;
    float suma_rozdilu = 0;
    int rozdil_x = 0;
    int rozdil_y = 0;
    float frangi_x = 0;
    float frangi_y = 0;
    float hodnoty_pixelu_frangi = 0;
    for (int i = 0; i<frangi.rows; i++)
    {
        for (int j = 0; j<frangi.cols; j++)
        {
            if (frangi.at<float>(i,j)>=float(0.98*maximum_frangi))
            {
                rozdil_x = souradnice_maxima_frangi.x - j;
                rozdil_y = souradnice_maxima_frangi.y - i;
                suma_rozdilu = float(std::pow(rozdil_x,2.0f) + std::pow(rozdil_y,2.0f));
                euklid = std::sqrt(suma_rozdilu);
                if (euklid < 3)
                {
                    pocitadlo+=1;
                    hodnoty_pixelu_frangi += frangi.at<float>(i,j);
                    frangi_x += j*frangi.at<float>(i,j);
                    frangi_y += i*frangi.at<float>(i,j);
                }
            }
        }
    }
    vystup.x = double(frangi_x/hodnoty_pixelu_frangi);
    vystup.y = double(frangi_y/hodnoty_pixelu_frangi);
    return vystup;
}

cv::Point3d frangi_analyza(const cv::Mat vstupni_snimek,
                           int mod_zpracovani,
                           int presnost,
                           int zobraz_vysledek_detekce,
                           QString jmeno_okna,
                           int typ_snimku,
                           bool pritomnost_casove_znamky,
                           cv::Point3d mira_posunuti,
                           QVector<double> parametryFF)
{
    int uspech_detekce_maxima;
    Point3d vystup_funkce;
    qDebug()<<"kontrola vstupu do frangi_analyza.";
    qDebug()<<parametryFF.length();
    /******************************/
    frangi2d_opts_t opts;
    frangi2d_createopts(&opts);
    if (mod_zpracovani == 2)
    {
        opts.BlackWhite = false;
    }
    else if(mod_zpracovani == 1)
    {
        opts.BlackWhite = true;
    }
    opts.sigma_start = int(parametryFF[0]);
    opts.sigma_step = int(parametryFF[1]);
    opts.sigma_end = int(parametryFF[2]);
    opts.BetaOne = parametryFF[3];
    opts.BetaTwo = parametryFF[4];
    /******************************/
    int r = int(mira_posunuti.y);
    int s = int(mira_posunuti.x);
    Mat obraz_filtrovany,obraz_frangi,obraz_scale, obraz_angles;
    obraz_filtrovany = predzpracovani_obrazu(vstupni_snimek,60.0f,0.4f);
    if (typ_snimku == 1 && pritomnost_casove_znamky == 0) {osetreni_okraju_snimku(obraz_filtrovany,1,20,20);}
    if (typ_snimku == 2 && pritomnost_casove_znamky == 0) {osetreni_okraju_snimku(obraz_filtrovany,2,r,s);}
    if (typ_snimku == 2 && pritomnost_casove_znamky == 1) {osetreni_okraju_snimku(obraz_filtrovany,2,40,40);}
    if (typ_snimku == 1 && pritomnost_casove_znamky == 1) {osetreni_okraju_snimku(obraz_filtrovany,1,80,50);}
    frangi2d(obraz_filtrovany, obraz_frangi, obraz_scale, obraz_angles, opts);
    //qDebug()<<"Pocet kanalu frangiho vystupu "<<obraz_frangi.channels()<<" "<<obraz_frangi.type();
    //imwrite(jmeno_okna+"",obraz_frangi);
    obraz_scale.release();
    obraz_angles.release();
    obraz_filtrovany.release();
    if (typ_snimku == 1 && pritomnost_casove_znamky == 0) {nulovani_okraju(obraz_frangi,1,50,50);}
    if (typ_snimku == 2 && pritomnost_casove_znamky == 0) {nulovani_okraju(obraz_frangi,2,r,s);}
    if (typ_snimku == 2 && pritomnost_casove_znamky == 1) {nulovani_okraju(obraz_frangi,2,80,80);}
    if (typ_snimku == 1 && pritomnost_casove_znamky == 1) {nulovani_okraju(obraz_frangi,1,250,80);}
    /*************************************************************************/
    double maximum_obraz_frangi;
    Point max_loc_frangi;
    cv::minMaxLoc(obraz_frangi, NULL, &maximum_obraz_frangi, NULL, &max_loc_frangi);
    if ((max_loc_frangi.x!=max_loc_frangi.x)== 1 || (max_loc_frangi.y!=max_loc_frangi.y) == 1)
    {
        qDebug()<<"Maximum Frangiho funkce se nepodarilo detekovat!";
        uspech_detekce_maxima = 0;
        vystup_funkce.z = uspech_detekce_maxima;
        vystup_funkce.x = -10;
        vystup_funkce.y = -10;
        //return vystup_funkce;
    }
    else
    {
        uspech_detekce_maxima = 1;
        qDebug()<<"Detekce maxima Frangiho funkce se zdarila.";
        if (presnost == 1)
        {
            vystup_funkce.x = max_loc_frangi.x;
            vystup_funkce.y = max_loc_frangi.y;
            vystup_funkce.z = uspech_detekce_maxima;
        }
        if (presnost == 2)
        {
            cv::Point2d teziste = vypocet_teziste_frangi(obraz_frangi,maximum_obraz_frangi,max_loc_frangi);
            vystup_funkce.x = teziste.x;
            vystup_funkce.y = teziste.y;
            vystup_funkce.z = uspech_detekce_maxima;
        }
        if (zobraz_vysledek_detekce == 1)
        {
            drawMarker(obraz_frangi,max_loc_frangi,(0));
           //imshow(jmeno_okna,obraz_frangi);

        }
        /*if(zobraz_vysledek_detekce == 0)
        {
            return vystup_funkce;
        }*/
    }
    return vystup_funkce;
}

void inicializace_frangi_opt(QJsonObject nactenyObjekt, QString parametr, QVector<double>& nacteneParametry,
                             int &pozice)
{
    nacteneParametry[pozice] = nactenyObjekt[parametr].toDouble();
}

double data_z_frangi_opt(int pozice,QVector<double>& nacteneParametry)
{
    return nacteneParametry[pozice];
}
void velikost_frangi_opt(int velikost,QVector<double>& nacteneParametry){
    nacteneParametry = (QVector<double>(velikost));
}
