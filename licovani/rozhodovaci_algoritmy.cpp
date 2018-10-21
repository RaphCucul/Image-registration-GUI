#include "licovani/rozhodovaci_algoritmy.h"
#include "licovani/fazova_korelace_funkce.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "licovani/multiPOC_Ai1.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <QVector>
#include <iterator>
#include <QDebug>
using cv::Mat;
using cv::Point3d;
using std::cout;
using std::endl;
void rozhodovani_prvni(QVector<double> &spatne_snimky_prvotni_ohodnoceni,
                       QVector<double> &hodnoceni_vsech_snimku_videa,
                       QVector<double> &POC_x,
                       QVector<double> &POC_y,
                       QVector<double> &uhel,
                       QVector<double> &frangi_x,
                       QVector<double> &frangi_y,
                       QVector<double> &frangi_euklid,
                       double prumerny_korelacni_koeficient,
                       double prumerne_FWHM,
                       cv::VideoCapture& cap,
                       cv::Mat& referencni_snimek,
                       cv::Rect& vyrez_korelace_standardni,
                       cv::Rect& vyrez_korelace_navic,
                       bool zmena_velikosti_obrazu,
                       QVector<double> &snimky_k_provereni_prvni,
                       QVector<double> &vypoctene_hodnoty_R,
                       QVector<double> &vypoctene_hodnoty_FWHM)
{
    for (int i = 0; i < spatne_snimky_prvotni_ohodnoceni.size(); i++)
    {
        Mat posunuty_temp,posunuty,posunuty_vyrez,slicovany,slicovany_vyrez;
        Mat obraz, obraz_vyrez;
        referencni_snimek.copyTo(obraz);
        obraz(vyrez_korelace_standardni).copyTo(obraz_vyrez);
        int rows = obraz.rows;
        int cols = obraz.cols;
        cap.set(CV_CAP_PROP_POS_FRAMES,spatne_snimky_prvotni_ohodnoceni[i]);
        if (cap.read(posunuty_temp) != 1)
        {
            qWarning()<<"Snimek "<<i<<" nelze slicovat!";
            POC_x[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999.0;
            POC_y[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999.0;
            uhel[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999.0;
            frangi_x[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999.0;
            frangi_y[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999.0;
            frangi_euklid[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999.0;
            continue;
        }
        if (zmena_velikosti_obrazu == true)
        {
            posunuty_temp(vyrez_korelace_navic).copyTo(posunuty);
            posunuty(vyrez_korelace_standardni).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        else
        {
            posunuty_temp.copyTo(posunuty);
            posunuty(vyrez_korelace_standardni).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        cv::Point3d pt(0,0,0);
        if (zmena_velikosti_obrazu == true)
        {
            pt = fk_translace_hann(obraz,posunuty,5);
            if (std::abs(pt.x)>=290 || std::abs(pt.y)>=290)
            {
                pt = fk_translace(obraz,posunuty,5);
            }
        }
        if (zmena_velikosti_obrazu == false){pt = fk_translace_hann(obraz,posunuty,5);}
        if (pt.x >= 55 || pt.y >= 55)
        {
            qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " nepripusten k analyze.";
            hodnoceni_vsech_snimku_videa[int(spatne_snimky_prvotni_ohodnoceni[i])] = 5;
            POC_x[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            POC_y[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            uhel[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            frangi_x[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            frangi_y[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            frangi_euklid[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            posunuty.release();
        }
        else
        {
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            slicovany = translace_snimku(posunuty,pt,rows,cols);
            slicovany(vyrez_korelace_standardni).copyTo(slicovany_vyrez);
            double R = vypocet_KK(obraz,slicovany,vyrez_korelace_standardni);
            //cout << "R " << R <<" a FWHM " << FWHM << " ";
            slicovany.release();
            slicovany_vyrez.release();
            posunuty.release();
            if ((std::abs(prumerny_korelacni_koeficient - R) < 0.02) && (FWHM < prumerne_FWHM)) //1.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " je vhodny ke slicovani.";
                hodnoceni_vsech_snimku_videa[int(spatne_snimky_prvotni_ohodnoceni[i])] = 0;
            }
		else if (R > prumerny_korelacni_koeficient && std::abs(FWHM-prumerne_FWHM)<=2) //5.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " je vhodny ke slicovani.";
                hodnoceni_vsech_snimku_videa[int(spatne_snimky_prvotni_ohodnoceni[i])] = 0;
            }
		else if (R > prumerny_korelacni_koeficient && FWHM > prumerne_FWHM) //4.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " je vhodny ke slicovani.";
                hodnoceni_vsech_snimku_videa[int(spatne_snimky_prvotni_ohodnoceni[i])] = 0;
            }
            else if ((std::abs(prumerny_korelacni_koeficient - R) <= 0.02) && (FWHM > prumerne_FWHM)) //2.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " bude proveren.";
                snimky_k_provereni_prvni.push_back(int(spatne_snimky_prvotni_ohodnoceni[i]));
                vypoctene_hodnoty_FWHM.push_back(FWHM);
                vypoctene_hodnoty_R.push_back(R);
            }
            else if (((prumerny_korelacni_koeficient - R) > 0.02) && ((prumerny_korelacni_koeficient - R) < 0.1)) //3.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " bude proveren.";
                snimky_k_provereni_prvni.push_back(int(spatne_snimky_prvotni_ohodnoceni[i]));
                vypoctene_hodnoty_FWHM.push_back(FWHM);
                vypoctene_hodnoty_R.push_back(R);
            }
		else if (((prumerny_korelacni_koeficient - R) >= 0.05) && ((FWHM < prumerne_FWHM) || prumerne_FWHM > 35.0)) //6.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " bude proveren.";
                snimky_k_provereni_prvni.push_back(int(spatne_snimky_prvotni_ohodnoceni[i]));
                vypoctene_hodnoty_FWHM.push_back(FWHM);
                vypoctene_hodnoty_R.push_back(R);
            }
		else if (((prumerny_korelacni_koeficient - R) >= 0.05) && (FWHM <= (prumerne_FWHM+10))) //8.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " bude proveren.";
                snimky_k_provereni_prvni.push_back(int(spatne_snimky_prvotni_ohodnoceni[i]));
                vypoctene_hodnoty_FWHM.push_back(FWHM);
                vypoctene_hodnoty_R.push_back(R);
            }

            else if (((prumerny_korelacni_koeficient - R) >= 0.1) && (FWHM > (prumerne_FWHM+10))) //7.
            {
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " nepripusten k analyze.";
                hodnoceni_vsech_snimku_videa[int(spatne_snimky_prvotni_ohodnoceni[i])] = 5;
                POC_x[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
                POC_y[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
                uhel[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
                frangi_x[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
                frangi_y[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
                frangi_euklid[int(spatne_snimky_prvotni_ohodnoceni[i])] = 999;
            }
            else
            {
                //cout << "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " nepripusten k analyze."<<endl;
                /*hodnoceni_vsech_snimku_videa[spatne_snimky_prvotni_ohodnoceni[i]] = 5;
                POC_x[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                POC_y[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                uhel[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_x[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_y[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_euklid[spatne_snimky_prvotni_ohodnoceni[i]] = 999;*/
                qDebug()<< "Snimek "<< spatne_snimky_prvotni_ohodnoceni[i]<< " bude proveren.";
                snimky_k_provereni_prvni.push_back(int(spatne_snimky_prvotni_ohodnoceni[i]));
                        vypoctene_hodnoty_FWHM.push_back(FWHM);
                vypoctene_hodnoty_R.push_back(R);

            }
        }
    }
}

void rozhodovani_druhe(QVector<double> &snimky_k_provereni_prvni,
                       QVector<double> &hodnoceni_vsech_snimku_videa,
                       QVector<double> &vypoctene_hodnoty_R,
                       QVector<double> &vypoctene_hodnoty_FWHM,
                       QVector<double> &POC_x,
                       QVector<double> &POC_y,
                       QVector<double> &uhel,
                       QVector<double> &frangi_x,
                       QVector<double> &frangi_y,
                       QVector<double> &frangi_euklid,
                       double prumerny_korelacni_koeficient,
                       double prumerne_FWHM,
                       QVector<double> &snimky_k_provereni_druhy)
{
    for (int b = 0; b < snimky_k_provereni_prvni.size(); b++)
    {
        if ((prumerny_korelacni_koeficient - vypoctene_hodnoty_R[b]) <= 0.01)
        {
            if (vypoctene_hodnoty_FWHM[b] < (prumerne_FWHM + 2))
            {
               qDebug()<< "Snimek "<< snimky_k_provereni_prvni[b]<< " je vhodny ke slicovani.";
                hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_prvni[b])] = 0;
            }
            else
            {
                qDebug()<< "Snimek "<< snimky_k_provereni_prvni[b]<< " bude proveren.";
                snimky_k_provereni_druhy.push_back(snimky_k_provereni_prvni[b]);
            }
        }
        else if ((prumerny_korelacni_koeficient - vypoctene_hodnoty_R[b]) > 0.01)
        {
            if (vypoctene_hodnoty_FWHM[b] <= (prumerne_FWHM + 2.5))
            {
                qDebug()<< "Snimek "<< snimky_k_provereni_prvni[b]<< " je vhodny ke slicovani.";
                hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_prvni[b])] = 0;
            }
            else if (vypoctene_hodnoty_FWHM[b] > (prumerne_FWHM + 10))
            {
                qDebug()<< "Snimek "<< snimky_k_provereni_prvni[b]<< " nepripusten k analyze.";
                hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_prvni[b])] = 5.0;
                POC_x[int(snimky_k_provereni_prvni[b])] = 999.0;
                POC_y[int(snimky_k_provereni_prvni[b])] = 999.0;
                uhel[int(snimky_k_provereni_prvni[b])] = 999.0;
                frangi_x[int(snimky_k_provereni_prvni[b])] = 9990.;
                frangi_y[int(snimky_k_provereni_prvni[b])] = 999.0;
                frangi_euklid[int(snimky_k_provereni_prvni[b])] = 999.0;
            }
            else
            {
                qDebug()<< "Snimek "<< snimky_k_provereni_prvni[b]<< " bude proveren.";
                snimky_k_provereni_druhy.push_back(int(snimky_k_provereni_prvni[b]));
            }
        }
    }
}

void rozhodovani_treti(cv::Mat& obraz,
                       cv::Rect& vyrez_korelace_navic,
                       cv::Rect& vyrez_korelace_standardni,
                       QVector<double> &frangi_x,
                       QVector<double> &frangi_y,
                       QVector<double> &frangi_euklid,
                       QVector<double> &POC_x,
                       QVector<double> &POC_y,
                       QVector<double> &uhel,
                       bool zmena_velikosti_snimku,
                       bool casova_znamka,
                       int iterace,
                       double oblastMaxima,
                       double uhelMaximalni,
                       cv::VideoCapture& cap,
                       QVector<double> &hodnoceni_vsech_snimku_videa,
                       QVector<double> &snimky_k_provereni_druhy,
                       QVector<double> &parametryFrangianalyzy)
{
    cv::Point3d pt_temp(0,0,0);
    Point3d obraz_frangi_reverse = frangi_analyza(obraz,2,2,0,"",1,false,pt_temp,parametryFrangianalyzy);//!
    Mat obraz_vyrez;
    obraz(vyrez_korelace_standardni).copyTo(obraz_vyrez);
    int rows = obraz.rows;
    int cols = obraz.cols;
    //for (unsigned int i = 0; i < 1; i++)
    for (int i = 0; i < snimky_k_provereni_druhy.size(); i++) //snimky_k_provereni2.size()
    {

        Mat slicovan_kompletne = cv::Mat::zeros(obraz.size(), CV_32FC3);
        Point3d mira_translace;
        double celkovy_uhel = 0;
        int uspech_licovani = kompletni_slicovani(cap,obraz,
                                                  snimky_k_provereni_druhy[i],
                                                  iterace,
                                                  oblastMaxima,
                                                  uhelMaximalni,
                                                  vyrez_korelace_navic,
                                                  vyrez_korelace_standardni,
                                                  zmena_velikosti_snimku,
                                                  slicovan_kompletne,
                                                  mira_translace,celkovy_uhel);
        //cout << "Typ: "<<slicovan_kompletne.type()<<endl;
        cout << snimky_k_provereni_druhy[i] <<" -> ";
        if (uspech_licovani == 0)
        {

            cout << "nelze slicovat, ohodnocení: 5"<<endl;
            hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_druhy[i])] = 5.0;
            POC_x[int(snimky_k_provereni_druhy[i])] = 999.0;
            POC_y[int(snimky_k_provereni_druhy[i])] = 999.0;
            uhel[int(snimky_k_provereni_druhy[i])] = 999.0;
            frangi_x[int(snimky_k_provereni_druhy[i])] = 999.0;
            frangi_y[int(snimky_k_provereni_druhy[i])] = 999.0;
            frangi_euklid[int(snimky_k_provereni_druhy[i])] = 999.0;
            continue;
        }
        else
        {
            Mat mezivysledek32f,mezivysledek32f_vyrez;
            slicovan_kompletne.copyTo(mezivysledek32f);
            kontrola_typu_snimku_32C1(mezivysledek32f);
            mezivysledek32f(vyrez_korelace_standardni).copyTo(mezivysledek32f_vyrez);
            double R1 = vypocet_KK(obraz,slicovan_kompletne,vyrez_korelace_standardni);
            mezivysledek32f.release();
            mezivysledek32f_vyrez.release();
            Point3d korekce_bod(0,0,0);
            if (zmena_velikosti_snimku == true)
            {
                korekce_bod = fk_translace(obraz,slicovan_kompletne,5.0);
                if (std::abs(korekce_bod.x)>=290 || std::abs(korekce_bod.y)>=290)
                {
                    korekce_bod = fk_translace_hann(obraz,slicovan_kompletne,5.0);
                }
            }
            if (zmena_velikosti_snimku == false){korekce_bod = fk_translace_hann(obraz,slicovan_kompletne,5.0);}
            Mat korekce = translace_snimku(slicovan_kompletne,korekce_bod,rows,cols);
            korekce.copyTo(mezivysledek32f);
            kontrola_typu_snimku_32C1(mezivysledek32f);
            mezivysledek32f(vyrez_korelace_standardni).copyTo(mezivysledek32f_vyrez);
            double R2 = vypocet_KK(obraz,korekce,vyrez_korelace_standardni);
            Point3d slicovany_frangi_reverse(0,0,0);
            double rozdil = R2-R1;
            if ((R2-R1)>0.015)
            {
                cv::Point3d extra_translace(0,0,0);
                extra_translace.x = mira_translace.x+korekce_bod.x;
                extra_translace.y = mira_translace.y+korekce_bod.y;
                extra_translace.z = mira_translace.z;
                qDebug()<< "Provedena korekce posunuti pro objektivnejsi analyzu skrze cevy."<<endl;
                slicovany_frangi_reverse = frangi_analyza(korekce,2,2,0,"",2,false,extra_translace,parametryFrangianalyzy);//!
            }
            else
            {
                slicovany_frangi_reverse = frangi_analyza(slicovan_kompletne,2,2,0,"",2,false,mira_translace,parametryFrangianalyzy);//!
            }
            slicovan_kompletne.release();
            if (slicovany_frangi_reverse.z == 0.0)
            {

                qDebug()<< "Nelze zjistit maximum Frangiho funkce, ohodnoceni: 5";
                hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_druhy[i])] = 5.0;
                POC_x[int(snimky_k_provereni_druhy[i])] = 999.0;
                POC_y[int(snimky_k_provereni_druhy[i])] = 999.0;
                uhel[int(snimky_k_provereni_druhy[i])] = 999.0;
                frangi_x[int(snimky_k_provereni_druhy[i])] = 999.0;
                frangi_y[int(snimky_k_provereni_druhy[i])] = 999.0;
                frangi_euklid[int(snimky_k_provereni_druhy[i])] = 999.0;
                continue;
            }
            else
            {
                double rozdil_x = obraz_frangi_reverse.x - slicovany_frangi_reverse.x;
                double rozdil_y = obraz_frangi_reverse.y - slicovany_frangi_reverse.y;
                double suma_rozdilu = std::pow(rozdil_x,2.0) + std::pow(rozdil_y,2.0);
                double euklid = std::sqrt(suma_rozdilu);
                if (euklid <= 1.2)
                {
                    hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_druhy[i])] = 0;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 0";
                }
                else if (euklid > 1.2 && euklid < 10)
                {
                    hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_druhy[i])] = 1;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 1";
                    frangi_x[int(snimky_k_provereni_druhy[i])] = slicovany_frangi_reverse.x;
                    frangi_y[int(snimky_k_provereni_druhy[i])] = slicovany_frangi_reverse.y;
                    frangi_euklid[int(snimky_k_provereni_druhy[i])] = euklid;
                }
                else if (euklid >=10)
                {
                    hodnoceni_vsech_snimku_videa[int(snimky_k_provereni_druhy[i])] = 4;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 4";
                    frangi_x[int(snimky_k_provereni_druhy[i])] = slicovany_frangi_reverse.x;
                    frangi_y[int(snimky_k_provereni_druhy[i])] = slicovany_frangi_reverse.y;
                    frangi_euklid[int(snimky_k_provereni_druhy[i])] = euklid;
                }
                rozdil_x = 0;
                rozdil_y = 0;
                suma_rozdilu = 0;
                euklid = 0;
            }

        }
    }
}
