#include "licovani/korekce_zapis.h"
#include "licovani/multiPOC_Ai1.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "licovani/fazova_korelace_funkce.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

using cv::Mat;
using cv::VideoCapture;
using cv::VideoWriter;
using cv::Rect;
using cv::Point3d;
using cv::Size;
using std::cerr;
using std::cout;
using std::endl;
void licovani_nejvhodnejsich_snimku(cv::VideoCapture& cap,
                                    cv::VideoWriter& zap,
                                    cv::Mat& referencni_snimek,
                                    cv::Point3d bod_RefS_reverse,
                                    int index_posunuty,
                                    int &iterace,
                                    double &oblastMaxima,
                                    double &uhel,
                                    cv::Rect& vyrez_korelace_extra,
                                    cv::Rect& vyrez_korelace_standard,
                                    bool zmena_meritka,
                                    QVector<double> &parametry_frangi,
                                    QVector<double>& frangi_x,
                                    QVector<double>& frangi_y,
                                    QVector<double>& frangi_euklid,
                                    QVector<double>& POC_x,
                                    QVector<double>& POC_y,
                                    QVector<double>& celkovyUhel,
                                    QVector<double>& finalPOC_x,
                                    QVector<double>& finalPOC_y)
{
    Mat plne_slicovany_snimek = cv::Mat::zeros(Size(referencni_snimek.cols,referencni_snimek.rows), CV_32FC3);
    Point3d mira_translace(0.0,0.0,0.0);
    double celkovy_uhel = 0;
    int uspech_licovani = kompletni_slicovani(cap,
                                              referencni_snimek,
                                              index_posunuty,
                                              iterace,
                                              oblastMaxima,
                                              uhel,
                                              vyrez_korelace_extra,
                                              vyrez_korelace_standard,
                                              zmena_meritka,
                                              plne_slicovany_snimek,
                                              mira_translace,
                                              celkovy_uhel);
    Mat refSnimek_vyrez;
    referencni_snimek(vyrez_korelace_standard).copyTo(refSnimek_vyrez);
    int rows = referencni_snimek.rows;
    int cols = referencni_snimek.cols;
    if (uspech_licovani == 0)
    {
        cerr<< "Licovani skoncilo chybou. Snimek "<<index_posunuty<<" se nepodarilo uspesne slicovat"<<endl;
        Mat posunuty;
        cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
        if (cap.read(posunuty)!=1)
        {
            cerr<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!"<<endl;
        }
        else
        {
            cout << "Snimek "<<index_posunuty<<" zapsan beze zmen. Licovani skoncilo chybou."<<endl<<endl;
            zap.write(posunuty);
            posunuty.release();
            plne_slicovany_snimek.release();
        }
        celkovyUhel[index_posunuty] = 0.0;
        finalPOC_x[index_posunuty] = 0.0;
        finalPOC_y[index_posunuty] = 0.0;
    }
    else
    {
        if (std::abs(mira_translace.x) >= 55 || std::abs(mira_translace.y) >= 55)
        {
            Mat posunuty;
            cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
            if (cap.read(posunuty)!=1)
            {
                cerr<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!"<<endl;
            }
            else
            {
                cout << "Snimek "<<index_posunuty<<" zapsan beze zmen. Hodnota translace prekrocila prah."<<endl<<endl;
                zap.write(posunuty);
                posunuty.release();
            }
            celkovyUhel[index_posunuty] = 0.0;
            finalPOC_x[index_posunuty] = 0.0;
            finalPOC_y[index_posunuty] = 0.0;
        }
        else
        {
            Mat posunuty_temp;
            cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
            if (cap.read(posunuty_temp)!=1)
            {
                cerr<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!"<<endl;
            }
            Mat mezivysledek32f,mezivysledek32f_vyrez,posunuty;
            if (zmena_meritka == true)
            {
                posunuty_temp(vyrez_korelace_extra).copyTo(posunuty);
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
            }
            Point3d translace_korekce(0,0,0);
            Mat plne_slicovany_snimek = eventualni_korekce_translace(plne_slicovany_snimek,referencni_snimek,
                                                                     vyrez_korelace_standard,translace_korekce,oblastMaxima);
            if (translace_korekce.x != 0.0)
            {
                mira_translace.x+=translace_korekce.x;
                mira_translace.y+=translace_korekce.y;
                Point3d pt6 = fk_translace_hann(referencni_snimek,plne_slicovany_snimek);
                if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                {
                    pt6 = fk_translace(referencni_snimek,plne_slicovany_snimek);
                }
                cout << "kontrola Y: " << pt6.y <<" X: "<<pt6.x << endl;
                finalPOC_x[index_posunuty] = pt6.x;
                finalPOC_y[index_posunuty] = pt6.y;
            }
            POC_x[index_posunuty] = mira_translace.x;
            POC_y[index_posunuty] = mira_translace.y;
            celkovyUhel[index_posunuty] = celkovy_uhel;

            plne_slicovany_snimek.copyTo(mezivysledek32f);
            kontrola_typu_snimku_32C1(mezivysledek32f);
            mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
            double R_prvni = vypocet_KK(referencni_snimek,plne_slicovany_snimek,vyrez_korelace_standard);
            Point3d frangi_bod_slicovany_reverse = frangi_analyza(plne_slicovany_snimek,2,2,0,"",2,false,mira_translace,parametry_frangi);
            frangi_x[index_posunuty] = frangi_bod_slicovany_reverse.x;
            frangi_y[index_posunuty] = frangi_bod_slicovany_reverse.y;
            double yydef = bod_RefS_reverse.x - frangi_bod_slicovany_reverse.x;
            double xxdef = bod_RefS_reverse.y - frangi_bod_slicovany_reverse.y;
            //cout << yydef << " " << xxdef << endl;
            double suma_rozdilu = std::pow(xxdef,2.0) + std::pow(yydef,2.0);
            double euklid = std::sqrt(suma_rozdilu);
            frangi_euklid[index_posunuty] = euklid;
            xxdef = 0;
            yydef = 0;
            suma_rozdilu = 0;

            Point3d vysledne_posunuti;
            vysledne_posunuti.y = mira_translace.y - yydef;
            vysledne_posunuti.x = mira_translace.x - xxdef;
            vysledne_posunuti.z = 0.0;
            //cout << vysledne_posunuti.y << " " << vysledne_posunuti.x << endl;//pt3.x <<endl;
            Mat posunuty2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
            Mat finalni_licovani = rotace_snimku(posunuty2,celkovy_uhel);
            Mat finalni_licovani_32f,finalni_licovani_32f_vyrez;
            finalni_licovani.copyTo(finalni_licovani_32f);
            kontrola_typu_snimku_32C1(finalni_licovani_32f);
            finalni_licovani_32f(vyrez_korelace_standard).copyTo(finalni_licovani_32f_vyrez);
            finalni_licovani_32f.release();
            posunuty2.release();
            double R_druhy = vypocet_KK(referencni_snimek,finalni_licovani,vyrez_korelace_standard);
            finalni_licovani_32f_vyrez.release();
            if (R_prvni >= R_druhy)
            {
                //cout << "Snimek "<<index_posunuty<<" zapsan pouze po standardnim slicovani."<<endl;
                //cout<<" Mira translace: "<<mira_translace.x<<" "<<mira_translace.y<<" "<<celkovy_uhel<<endl<<endl;
                if (zmena_meritka == true)
                {
                    int radky = posunuty_temp.rows;
                    int sloupce = posunuty_temp.cols;
                    Mat posunuty_original = translace_snimku(posunuty_temp,mira_translace,radky,sloupce);
                    Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                    zap.write(finalni_licovani2);
                    plne_slicovany_snimek.release();
                    finalni_licovani2.release();
                    finalni_licovani.release();
                    posunuty_original.release();
                    posunuty_temp.release();
                }
                else
                {
                    zap.write(plne_slicovany_snimek);
                    plne_slicovany_snimek.release();
                    finalni_licovani.release();
                }
                celkovyUhel[index_posunuty] = 0.0;
                finalPOC_x[index_posunuty] = 0.0;
                finalPOC_y[index_posunuty] = 0.0;
                euklid = 0.0;
            }
            else
            {
                cout << "Snimek "<<index_posunuty<<" zapsan po uprave translace skrze cevy."<<endl;
                cout<<" Mira translace: "<<vysledne_posunuti.x<<" "<<vysledne_posunuti.y<<" "<<celkovy_uhel<<endl<<endl;
                if (zmena_meritka == true)
                {
                    int radky = posunuty_temp.rows;
                    int sloupce = posunuty_temp.cols;
                    Mat posunuty_original = translace_snimku(posunuty_temp,vysledne_posunuti,radky,sloupce);
                    Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                    zap.write(finalni_licovani2);
                    plne_slicovany_snimek.release();
                    finalni_licovani2.release();
                    finalni_licovani.release();
                    posunuty_original.release();
                    posunuty_temp.release();
                }
                else
                {
                    zap.write(finalni_licovani);
                    plne_slicovany_snimek.release();
                    finalni_licovani.release();
                }
                celkovyUhel[index_posunuty] = 0.0;
                finalPOC_x[index_posunuty] = 0.0;
                finalPOC_y[index_posunuty] = 0.0;
                euklid = 0.0;
            }
            mezivysledek32f.release();
            mezivysledek32f_vyrez.release();
        }
    }
}

cv::Mat eventualni_korekce_translace(cv::Mat& slicovany_snimek,cv::Mat& obraz,cv::Rect& vyrez_korelace_standard,
                                     cv::Point3d& korekce_bod,double &oblastMaxima)
{
    Mat mezivysledek,mezivysledek32f,mezivysledek32f_vyrez,obraz_vyrez;
    slicovany_snimek.copyTo(mezivysledek);
    int rows = slicovany_snimek.rows;
    int cols = slicovany_snimek.cols;
    mezivysledek.copyTo(mezivysledek32f);
    kontrola_typu_snimku_32C1(mezivysledek32f);
    mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
    obraz(vyrez_korelace_standard).copyTo(obraz_vyrez);
    double R1 = vypocet_KK(obraz,mezivysledek,vyrez_korelace_standard);
    cout << "Korelace 1: "<<R1<<endl;
    mezivysledek32f.release();
    mezivysledek32f_vyrez.release();
    korekce_bod = fk_translace_hann(obraz,mezivysledek);
    Point3d mira_korekcniho_posunuti = fk_translace_hann(obraz,mezivysledek);
    if (std::abs(mira_korekcniho_posunuti.x) > 290 || std::abs(mira_korekcniho_posunuti.y) > 290)
    {
        mira_korekcniho_posunuti = fk_translace(obraz,mezivysledek);
    }
    if (std::abs(mira_korekcniho_posunuti.x) > 290 || std::abs(mira_korekcniho_posunuti.y) > 290)
    {
        mira_korekcniho_posunuti = fk_translace(obraz_vyrez,mezivysledek32f_vyrez);
    }
    cout << "korekce Y: " << korekce_bod.y <<" X: "<<korekce_bod.x<< endl;
    Mat korekce = translace_snimku(mezivysledek,korekce_bod,rows,cols);
    korekce.copyTo(mezivysledek32f);
    kontrola_typu_snimku_32C1(mezivysledek32f);
    mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
    double R2 = vypocet_KK(obraz,korekce,vyrez_korelace_standard);
    cout << "Korelace 2: "<<R2<<endl;
    //double rozdil = R2-R1;
    if ((R2 > R1) && ((std::abs(mira_korekcniho_posunuti.x) > 0.3) || (std::abs(mira_korekcniho_posunuti.y) > 0.3)))
    {
        cout << "snimek zkorigovan." << endl;
        korekce_bod = mira_korekcniho_posunuti;
        return korekce;
    }
    else
    {        
        cout << "snimek nezkorigovan." << endl;
        return mezivysledek;
    }
}
