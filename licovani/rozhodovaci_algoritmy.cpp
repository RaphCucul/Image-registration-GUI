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

//#include <iostream>
#include <QVector>
//#include <iterator>
#include <QDebug>
using cv::Mat;
using cv::Point3d;
void firstDecisionAlgorithm(QVector<double> &badFrames_FirstEval,
                            QVector<double> &allFrameCompleteEval,
                            QVector<double> &POC_x,
                            QVector<double> &POC_y,
                            QVector<double> &Angle,
                            QVector<double> &frangi_x,
                            QVector<double> &frangi_y,
                            QVector<double> &frangi_euklid,
                            double averageCC,
                            double averageFWHM,
                            cv::VideoCapture& cap,
                            cv::Mat& referImage,
                            cv::Rect& COStandard,
                            cv::Rect& COExtra,
                            bool scaleChanged,
                            QVector<double> &framesFirstEval,
                            QVector<double> &computedCC,
                            QVector<double> &computedFWHM)
{
    for (int i = 0; i < badFrames_FirstEval.length(); i++)
    {
        Mat posunuty_temp,posunuty,posunuty_vyrez,slicovany,slicovany_vyrez;
        Mat obraz, obraz_vyrez;
        referImage.copyTo(obraz);
        obraz(COStandard).copyTo(obraz_vyrez);
        int rows = obraz.rows;
        int cols = obraz.cols;
        cap.set(CV_CAP_PROP_POS_FRAMES,badFrames_FirstEval[i]);
        if (cap.read(posunuty_temp) != 1)
        {
            qWarning()<<"Snimek "<<i<<" nelze slicovat!";
            POC_x[int(badFrames_FirstEval[i])] = 999.0;
            POC_y[int(badFrames_FirstEval[i])] = 999.0;
            Angle[int(badFrames_FirstEval[i])] = 999.0;
            frangi_x[int(badFrames_FirstEval[i])] = 999.0;
            frangi_y[int(badFrames_FirstEval[i])] = 999.0;
            frangi_euklid[int(badFrames_FirstEval[i])] = 999.0;
            continue;
        }
        if (scaleChanged == true)
        {
            posunuty_temp(COExtra).copyTo(posunuty);
            posunuty(COStandard).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        else
        {
            posunuty_temp.copyTo(posunuty);
            posunuty(COStandard).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        cv::Point3d pt(0,0,0);
        if (scaleChanged == true)
        {
            pt = fk_translace_hann(obraz,posunuty);
            if (std::abs(pt.x)>=290 || std::abs(pt.y)>=290)
            {
                pt = fk_translace(obraz,posunuty);
            }
        }
        if (scaleChanged == false)
        {
            pt = fk_translace_hann(obraz,posunuty);
        }
        if (pt.x >= 55 || pt.y >= 55)
        {
            qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " nepripusten k analyze.";
            allFrameCompleteEval[int(badFrames_FirstEval[i])] = 5;
            POC_x[int(badFrames_FirstEval[i])] = 999;
            POC_y[int(badFrames_FirstEval[i])] = 999;
            Angle[int(badFrames_FirstEval[i])] = 999;
            frangi_x[int(badFrames_FirstEval[i])] = 999;
            frangi_y[int(badFrames_FirstEval[i])] = 999;
            frangi_euklid[int(badFrames_FirstEval[i])] = 999;
            posunuty.release();
        }
        else
        {
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            slicovany = translace_snimku(posunuty,pt,rows,cols);
            slicovany(COStandard).copyTo(slicovany_vyrez);
            double R = vypocet_KK(obraz,slicovany,COStandard);
            qDebug() <<badFrames_FirstEval[i]<< "R " << R <<" a FWHM " << FWHM;
            slicovany.release();
            slicovany_vyrez.release();
            posunuty.release();
            double rozdilnostKK = averageCC-R;
            double rozdilnostFWHM = averageFWHM-FWHM;
            if ((std::abs(rozdilnostKK) < 0.02) && (FWHM < averageFWHM)) //1.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " je vhodny ke slicovani.";
                allFrameCompleteEval[int(badFrames_FirstEval[i])] = 0.0;
                continue;
            }
            else if (R > averageCC && (std::abs(rozdilnostFWHM)<=2||(FWHM < averageFWHM))) //5.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " je vhodny ke slicovani.";
                allFrameCompleteEval[int(badFrames_FirstEval[i])] = 0.0;
                continue;
            }
            else if (R >= averageCC && FWHM > averageFWHM) //4.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " je vhodny ke slicovani.";
                allFrameCompleteEval[int(badFrames_FirstEval[i])] = 0.0;
                continue;
            }
            else if ((std::abs(rozdilnostKK) <= 0.02) && (FWHM > averageFWHM)) //2.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " bude proveren.";
                framesFirstEval.push_back(int(badFrames_FirstEval[i]));
                computedFWHM.push_back(FWHM);
                computedCC.push_back(R);
                continue;
            }
            else if ((rozdilnostKK > 0.02) && (rozdilnostKK < 0.18)) //3.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " bude proveren.";
                framesFirstEval.push_back(int(badFrames_FirstEval[i]));
                computedFWHM.push_back(FWHM);
                computedCC.push_back(R);
                continue;
            }
            else if ((rozdilnostKK >= 0.05 && rozdilnostKK < 0.18) && ((FWHM < averageFWHM) || averageFWHM > 35.0)) //6.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " bude proveren.";
                framesFirstEval.push_back(int(badFrames_FirstEval[i]));
                computedFWHM.push_back(FWHM);
                computedCC.push_back(R);
                continue;
            }
            else if ((rozdilnostKK >= 0.05 && rozdilnostKK < 0.18) && (FWHM <= (averageFWHM+10))) //8.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " bude proveren.";
                framesFirstEval.push_back(int(badFrames_FirstEval[i]));
                computedFWHM.push_back(FWHM);
                computedCC.push_back(R);
                continue;
            }

            else if ((rozdilnostKK >= 0.2) && (FWHM > (averageFWHM+10))) //7.
            {
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " nepripusten k analyze.";
                allFrameCompleteEval[int(badFrames_FirstEval[i])] = 5;
                POC_x[int(badFrames_FirstEval[i])] = 999;
                POC_y[int(badFrames_FirstEval[i])] = 999;
                Angle[int(badFrames_FirstEval[i])] = 999;
                frangi_x[int(badFrames_FirstEval[i])] = 999;
                frangi_y[int(badFrames_FirstEval[i])] = 999;
                frangi_euklid[int(badFrames_FirstEval[i])] = 999;
                continue;
            }
            else
            {
                qDebug() << "Snimek "<< badFrames_FirstEval[i]<< " bude proveren - nevyhovel nikde.";
                /*allFrameCompleteEval[spatne_snimky_prvotni_ohodnoceni[i]] = 5;
                POC_x[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                POC_y[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                Angle[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_x[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_y[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_euklid[spatne_snimky_prvotni_ohodnoceni[i]] = 999;*/
                qDebug()<< "Snimek "<< badFrames_FirstEval[i]<< " bude proveren.";
                framesFirstEval.push_back(int(badFrames_FirstEval[i]));
                computedFWHM.push_back(FWHM);
                computedCC.push_back(R);

            }
        }

    }
}

void secondDecisionAlgorithm(QVector<double> &framesFirstEval,
                             QVector<double> &allFrameCompleteEval,
                             QVector<double> &computedCC,
                             QVector<double> &computedFWHM,
                             QVector<double> &POC_x,
                             QVector<double> &POC_y,
                             QVector<double> &Angle,
                             QVector<double> &frangi_x,
                             QVector<double> &frangi_y,
                             QVector<double> &frangi_euklid,
                             double averageCC,
                             double averageFWHM,
                             QVector<double> &framesSecondEval)
{
    for (int b = 0; b < framesFirstEval.length(); b++)
    {
        if ((averageCC - computedCC[b]) <= 0.02)
        {
            if (computedFWHM[b] < (averageFWHM + 30))
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " je vhodny ke slicovani.";
                allFrameCompleteEval[int(framesFirstEval[b])] = 0;
            }
            else
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " bude proveren.";
                framesSecondEval.push_back(framesFirstEval[b]);
            }
        }
        else if ((averageCC - computedCC[b]) >0.02 && (averageCC - computedCC[b]) < 0.05)
        {
            if (computedFWHM[b] < (averageFWHM + 15))
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " je vhodny ke slicovani.";
                allFrameCompleteEval[int(framesFirstEval[b])] = 0;
            }
            else
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " bude proveren.";
                framesSecondEval.push_back(framesFirstEval[b]);
            }
        }
        else if ((averageCC - computedCC[b]) >= 0.05)
        {
            if (computedFWHM[b] <= (averageFWHM + 5))
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " je vhodny ke slicovani.";
                allFrameCompleteEval[int(framesFirstEval[b])] = 0;
            }
            else if (computedFWHM[b] > (averageFWHM + 10))
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " nepripusten k analyze.";
                allFrameCompleteEval[int(framesFirstEval[b])] = 5.0;
                POC_x[int(framesFirstEval[b])] = 999.0;
                POC_y[int(framesFirstEval[b])] = 999.0;
                Angle[int(framesFirstEval[b])] = 999.0;
                frangi_x[int(framesFirstEval[b])] = 9990.;
                frangi_y[int(framesFirstEval[b])] = 999.0;
                frangi_euklid[int(framesFirstEval[b])] = 999.0;
            }
            else
            {
                qDebug()<< "Snimek "<< framesFirstEval[b]<< " bude proveren.";
                framesSecondEval.push_back(int(framesFirstEval[b]));
            }
        }
    }
}

void thirdDecisionAlgorithm(cv::Mat& image,
                            cv::Rect& coutouExtra,
                            cv::Rect& cutoutStandard,
                            QVector<double>& frangi_x,
                            QVector<double>& frangi_y,
                            QVector<double>& frangi_euklid,
                            QVector<double>& POC_x,
                            QVector<double>& POC_y,
                            QVector<double>& Angle,
                            bool scaleChanged,
                            cv::VideoCapture& cap,
                            QVector<double>& allFrameCompleteEval,
                            QVector<double>& framesSecondEval,
                            QVector<double>& FrangiParameters)
{
    cv::Point3d pt_temp(0,0,0);
    Point3d image_frangi_reverse = frangi_analysis(image,2,2,0,"",1,pt_temp,FrangiParameters);//!
    Mat image_cutout;
    image(cutoutStandard).copyTo(image_cutout);
    int rows = image.rows;
    int cols = image.cols;
    //for (unsigned int i = 0; i < 1; i++)
    for (int i = 0; i < framesSecondEval.length(); i++) //snimky_k_provereni2.size()
    {

        Mat slicovan_kompletne = cv::Mat::zeros(image.size(), CV_32FC3);
        Point3d mira_translace;
        QVector<double> pocX;
        QVector<double> pocY;
        QVector<double> celkovy_uhel;
        int iterace = -1;double oblastMaxima = 5.0;double uhelMaximalni = 0.1;
        int uspech_licovani = completeRegistration(cap,image,
                                                   framesSecondEval[i],
                                                   iterace,
                                                   oblastMaxima,
                                                   uhelMaximalni,
                                                   coutouExtra,
                                                   cutoutStandard,
                                                   scaleChanged,
                                                   slicovan_kompletne,
                                                   pocX,pocY,celkovy_uhel);
        qDebug() << framesSecondEval[i] <<" -> ";
        if (uspech_licovani == 0)
        {

            qDebug()  << "unable to analyse this frame.";
            allFrameCompleteEval[int(framesSecondEval[i])] = 5.0;
            POC_x[int(framesSecondEval[i])] = 999.0;
            POC_y[int(framesSecondEval[i])] = 999.0;
            Angle[int(framesSecondEval[i])] = 999.0;
            frangi_x[int(framesSecondEval[i])] = 999.0;
            frangi_y[int(framesSecondEval[i])] = 999.0;
            frangi_euklid[int(framesSecondEval[i])] = 999.0;
            continue;
        }
        else
        {
            Mat mezivysledek32f,mezivysledek32f_vyrez;
            slicovan_kompletne.copyTo(mezivysledek32f);
            kontrola_typu_snimku_32C1(mezivysledek32f);
            mezivysledek32f(cutoutStandard).copyTo(mezivysledek32f_vyrez);
            double R1 = vypocet_KK(image,slicovan_kompletne,cutoutStandard);
            mezivysledek32f.release();
            mezivysledek32f_vyrez.release();
            Point3d korekce_bod(0,0,0);
            if (scaleChanged == true)
            {
                korekce_bod = fk_translace(image,slicovan_kompletne);
                if (std::abs(korekce_bod.x)>=290 || std::abs(korekce_bod.y)>=290)
                {
                    korekce_bod = fk_translace_hann(image,slicovan_kompletne);
                }
            }
            else
            {
                korekce_bod = fk_translace_hann(image,slicovan_kompletne);
            }
            Mat korekce = translace_snimku(slicovan_kompletne,korekce_bod,rows,cols);
            korekce.copyTo(mezivysledek32f);
            kontrola_typu_snimku_32C1(mezivysledek32f);
            mezivysledek32f(cutoutStandard).copyTo(mezivysledek32f_vyrez);
            double R2 = vypocet_KK(image,korekce,cutoutStandard);
            Point3d slicovany_frangi_reverse(0,0,0);
            double rozdil = R2-R1;
            if (rozdil>0.015)
            {
                cv::Point3d extra_translace(0,0,0);
                extra_translace.x = mira_translace.x+korekce_bod.x;
                extra_translace.y = mira_translace.y+korekce_bod.y;
                extra_translace.z = mira_translace.z;
                qDebug()<< "Provedena korekce posunuti pro objektivnejsi analyzu skrze cevy.";
                slicovany_frangi_reverse = frangi_analysis(korekce,2,2,0,"",2,extra_translace,FrangiParameters);//!
            }
            else
            {
                slicovany_frangi_reverse = frangi_analysis(slicovan_kompletne,2,2,0,"",2,mira_translace,FrangiParameters);//!
            }
            slicovan_kompletne.release();
            if (slicovany_frangi_reverse.z == 0.0)
            {

                qDebug()<< "Nelze zjistit maximum Frangiho funkce, ohodnoceni: 5";
                allFrameCompleteEval[int(framesSecondEval[i])] = 5.0;
                POC_x[int(framesSecondEval[i])] = 999.0;
                POC_y[int(framesSecondEval[i])] = 999.0;
                Angle[int(framesSecondEval[i])] = 999.0;
                frangi_x[int(framesSecondEval[i])] = 999.0;
                frangi_y[int(framesSecondEval[i])] = 999.0;
                frangi_euklid[int(framesSecondEval[i])] = 999.0;
                continue;
            }
            else
            {
                double rozdil_x = image_frangi_reverse.x - slicovany_frangi_reverse.x;
                double rozdil_y = image_frangi_reverse.y - slicovany_frangi_reverse.y;
                double suma_rozdilu = std::pow(rozdil_x,2.0) + std::pow(rozdil_y,2.0);
                double euklid = std::sqrt(suma_rozdilu);
                if (euklid <= 1.2)
                {
                    allFrameCompleteEval[int(framesSecondEval[i])] = 0;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 0";
                }
                else if (euklid > 1.2 && euklid < 10)
                {
                    allFrameCompleteEval[int(framesSecondEval[i])] = 1;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 1";
                    frangi_x[int(framesSecondEval[i])] = slicovany_frangi_reverse.x;
                    frangi_y[int(framesSecondEval[i])] = slicovany_frangi_reverse.y;
                    frangi_euklid[int(framesSecondEval[i])] = euklid;
                }
                else if (euklid >=10)
                {
                    allFrameCompleteEval[int(framesSecondEval[i])] = 4;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 4";
                    frangi_x[int(framesSecondEval[i])] = slicovany_frangi_reverse.x;
                    frangi_y[int(framesSecondEval[i])] = slicovany_frangi_reverse.y;
                    frangi_euklid[int(framesSecondEval[i])] = euklid;
                }
                rozdil_x = 0;
                rozdil_y = 0;
                suma_rozdilu = 0;
                euklid = 0;
            }

        }
    }
}
