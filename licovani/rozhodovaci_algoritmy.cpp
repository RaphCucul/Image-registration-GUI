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
void firstDecisionAlgorithm(QVector<double> &i_badFrames_FirstEval,
                            QVector<double> &i_allFrameCompleteEval,
                            QVector<double> &i_POC_x,
                            QVector<double> &i_POC_y,
                            QVector<double> &i_Angle,
                            QVector<double> &i_frangi_x,
                            QVector<double> &i_frangi_y,
                            QVector<double> &i_frangi_euklid,
                            double i_averageCC,
                            double i_averageFWHM,
                            cv::VideoCapture& i_cap,
                            cv::Mat& i_referImage,
                            cv::Rect& i_COStandard,
                            cv::Rect& i_COExtra,
                            bool i_scaleChanged,
                            QVector<double> &i_framesFirstEval,
                            QVector<double> &i_computedCC,
                            QVector<double> &i_computedFWHM)
{
    for (int i = 0; i < i_badFrames_FirstEval.length(); i++)
    {
        Mat shifted_temp,shifted,shifted_vyrez,registratedFrame,registratedFrame_vyrez;
        Mat referencialFrame_alias, referencialFrame_alias_vyrez;
        i_referImage.copyTo(referencialFrame_alias);
        referencialFrame_alias(i_COStandard).copyTo(referencialFrame_alias_vyrez);
        int rows = referencialFrame_alias.rows;
        int cols = referencialFrame_alias.cols;
        i_cap.set(CV_CAP_PROP_POS_FRAMES,i_badFrames_FirstEval[i]);
        if (i_cap.read(shifted_temp) != 1)
        {
            qWarning()<<"Snimek "<<i<<" nelze slicovat!";
            i_POC_x[int(i_badFrames_FirstEval[i])] = 999.0;
            i_POC_y[int(i_badFrames_FirstEval[i])] = 999.0;
            i_Angle[int(i_badFrames_FirstEval[i])] = 999.0;
            i_frangi_x[int(i_badFrames_FirstEval[i])] = 999.0;
            i_frangi_y[int(i_badFrames_FirstEval[i])] = 999.0;
            i_frangi_euklid[int(i_badFrames_FirstEval[i])] = 999.0;
            continue;
        }
        if (i_scaleChanged == true)
        {
            shifted_temp(i_COExtra).copyTo(shifted);
            shifted(i_COStandard).copyTo(shifted_vyrez);
            shifted_temp.release();
        }
        else
        {
            shifted_temp.copyTo(shifted);
            shifted(i_COStandard).copyTo(shifted_vyrez);
            shifted_temp.release();
        }
        cv::Point3d pt(0,0,0);
        if (i_scaleChanged == true)
        {
            pt = fk_translace_hann(referencialFrame_alias,shifted);
            if (std::abs(pt.x)>=290 || std::abs(pt.y)>=290)
            {
                pt = fk_translace(referencialFrame_alias,shifted);
            }
        }
        if (i_scaleChanged == false)
        {
            pt = fk_translace_hann(referencialFrame_alias,shifted);
        }
        if (pt.x >= 55 || pt.y >= 55)
        {
            qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " nepripusten k analyze.";
            i_allFrameCompleteEval[int(i_badFrames_FirstEval[i])] = 5;
            i_POC_x[int(i_badFrames_FirstEval[i])] = 999;
            i_POC_y[int(i_badFrames_FirstEval[i])] = 999;
            i_Angle[int(i_badFrames_FirstEval[i])] = 999;
            i_frangi_x[int(i_badFrames_FirstEval[i])] = 999;
            i_frangi_y[int(i_badFrames_FirstEval[i])] = 999;
            i_frangi_euklid[int(i_badFrames_FirstEval[i])] = 999;
            shifted.release();
        }
        else
        {
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            registratedFrame = frameTranslation(shifted,pt,rows,cols);
            registratedFrame(i_COStandard).copyTo(registratedFrame_vyrez);
            double R = calculateCorrCoef(referencialFrame_alias,registratedFrame,i_COStandard);
            qDebug() <<i_badFrames_FirstEval[i]<< "R " << R <<" a FWHM " << FWHM;
            registratedFrame.release();
            registratedFrame_vyrez.release();
            shifted.release();
            double diff_corCoef = i_averageCC-R;
            double diff_FWHM = i_averageFWHM-FWHM;
            if ((std::abs(diff_corCoef) < 0.02) && (FWHM < i_averageFWHM)) //1.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " je vhodny ke slicovani.";
                i_allFrameCompleteEval[int(i_badFrames_FirstEval[i])] = 0.0;
                continue;
            }
            else if (R > i_averageCC && (std::abs(diff_FWHM)<=2||(FWHM < i_averageFWHM))) //5.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " je vhodny ke slicovani.";
                i_allFrameCompleteEval[int(i_badFrames_FirstEval[i])] = 0.0;
                continue;
            }
            else if (R >= i_averageCC && FWHM > i_averageFWHM) //4.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " je vhodny ke slicovani.";
                i_allFrameCompleteEval[int(i_badFrames_FirstEval[i])] = 0.0;
                continue;
            }
            else if ((std::abs(diff_corCoef) <= 0.02) && (FWHM > i_averageFWHM)) //2.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " bude proveren.";
                i_framesFirstEval.push_back(int(i_badFrames_FirstEval[i]));
                i_computedFWHM.push_back(FWHM);
                i_computedCC.push_back(R);
                continue;
            }
            else if ((diff_corCoef > 0.02) && (diff_corCoef < 0.18)) //3.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " bude proveren.";
                i_framesFirstEval.push_back(int(i_badFrames_FirstEval[i]));
                i_computedFWHM.push_back(FWHM);
                i_computedCC.push_back(R);
                continue;
            }
            else if ((diff_corCoef >= 0.05 && diff_corCoef < 0.18) && ((FWHM < i_averageFWHM) || i_averageFWHM > 35.0)) //6.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " bude proveren.";
                i_framesFirstEval.push_back(int(i_badFrames_FirstEval[i]));
                i_computedFWHM.push_back(FWHM);
                i_computedCC.push_back(R);
                continue;
            }
            else if ((diff_corCoef >= 0.05 && diff_corCoef < 0.18) && (FWHM <= (i_averageFWHM+10))) //8.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " bude proveren.";
                i_framesFirstEval.push_back(int(i_badFrames_FirstEval[i]));
                i_computedFWHM.push_back(FWHM);
                i_computedCC.push_back(R);
                continue;
            }

            else if ((diff_corCoef >= 0.2) && (FWHM > (i_averageFWHM+10))) //7.
            {
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " nepripusten k analyze.";
                i_allFrameCompleteEval[int(i_badFrames_FirstEval[i])] = 5;
                i_POC_x[int(i_badFrames_FirstEval[i])] = 999;
                i_POC_y[int(i_badFrames_FirstEval[i])] = 999;
                i_Angle[int(i_badFrames_FirstEval[i])] = 999;
                i_frangi_x[int(i_badFrames_FirstEval[i])] = 999;
                i_frangi_y[int(i_badFrames_FirstEval[i])] = 999;
                i_frangi_euklid[int(i_badFrames_FirstEval[i])] = 999;
                continue;
            }
            else
            {
                qDebug() << "Snimek "<< i_badFrames_FirstEval[i]<< " bude proveren - nevyhovel nikde.";
                /*allFrameCompleteEval[spatne_snimky_prvotni_ohodnoceni[i]] = 5;
                POC_x[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                POC_y[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                Angle[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_x[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_y[spatne_snimky_prvotni_ohodnoceni[i]] = 999;
                frangi_euklid[spatne_snimky_prvotni_ohodnoceni[i]] = 999;*/
                qDebug()<< "Snimek "<< i_badFrames_FirstEval[i]<< " bude proveren.";
                i_framesFirstEval.push_back(int(i_badFrames_FirstEval[i]));
                i_computedFWHM.push_back(FWHM);
                i_computedCC.push_back(R);

            }
        }

    }
}

void secondDecisionAlgorithm(QVector<double> &i_framesFirstEval,
                             QVector<double> &i_allFrameCompleteEval,
                             QVector<double> &i_computedCC,
                             QVector<double> &vcomputedFWHM,
                             QVector<double> &i_POC_x,
                             QVector<double> &i_POC_y,
                             QVector<double> &i_Angle,
                             QVector<double> &i_frangi_x,
                             QVector<double> &i_frangi_y,
                             QVector<double> &i_frangi_euklid,
                             double i_averageCC,
                             double i_averageFWHM,
                             QVector<double> &i_framesSecondEval)
{
    for (int b = 0; b < i_framesFirstEval.length(); b++)
    {
        if ((i_averageCC - i_computedCC[b]) <= 0.02)
        {
            if (vcomputedFWHM[b] < (i_averageFWHM + 30))
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " je vhodny ke slicovani.";
                i_allFrameCompleteEval[int(i_framesFirstEval[b])] = 0;
            }
            else
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " bude proveren.";
                i_framesSecondEval.push_back(i_framesFirstEval[b]);
            }
        }
        else if ((i_averageCC - i_computedCC[b]) >0.02 && (i_averageCC - i_computedCC[b]) < 0.05)
        {
            if (vcomputedFWHM[b] < (i_averageFWHM + 15))
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " je vhodny ke slicovani.";
                i_allFrameCompleteEval[int(i_framesFirstEval[b])] = 0;
            }
            else
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " bude proveren.";
                i_framesSecondEval.push_back(i_framesFirstEval[b]);
            }
        }
        else if ((i_averageCC - i_computedCC[b]) >= 0.05)
        {
            if (vcomputedFWHM[b] <= (i_averageFWHM + 5))
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " je vhodny ke slicovani.";
                i_allFrameCompleteEval[int(i_framesFirstEval[b])] = 0;
            }
            else if (vcomputedFWHM[b] > (i_averageFWHM + 10))
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " nepripusten k analyze.";
                i_allFrameCompleteEval[int(i_framesFirstEval[b])] = 5.0;
                i_POC_x[int(i_framesFirstEval[b])] = 999.0;
                i_POC_y[int(i_framesFirstEval[b])] = 999.0;
                i_Angle[int(i_framesFirstEval[b])] = 999.0;
                i_frangi_x[int(i_framesFirstEval[b])] = 9990.;
                i_frangi_y[int(i_framesFirstEval[b])] = 999.0;
                i_frangi_euklid[int(i_framesFirstEval[b])] = 999.0;
            }
            else
            {
                qDebug()<< "Snimek "<< i_framesFirstEval[b]<< " bude proveren.";
                i_framesSecondEval.push_back(int(i_framesFirstEval[b]));
            }
        }
    }
}

void thirdDecisionAlgorithm(cv::Mat& i_frame,
                            cv::Rect& i_coutouExtra,
                            cv::Rect& i_cutoutStandard,
                            QVector<double>& i_frangi_x,
                            QVector<double>& i_frangi_y,
                            QVector<double>& i_frangi_euklid,
                            QVector<double>& i_POC_x,
                            QVector<double>& i_POC_y,
                            QVector<double>& i_Angle,
                            bool i_scaleChanged,
                            cv::VideoCapture& i_cap,
                            QVector<double>& i_allFrameCompleteEval,
                            QVector<double>& i_framesSecondEval,
                            QVector<double>& i_FrangiParameters,
                            QMap<QString,int> i_margins)
{
    cv::Point3d pt_temp(0,0,0);
    Point3d frame_frangi_reverse = frangi_analysis(i_frame,2,2,0,"",1,pt_temp,i_FrangiParameters,i_margins);//!
    Mat frame_cutout;
    i_frame(i_cutoutStandard).copyTo(frame_cutout);
    int rows = i_frame.rows;
    int cols = i_frame.cols;
    for (int i = 0; i < i_framesSecondEval.length(); i++)
    {

        Mat slicovan_kompletne = cv::Mat::zeros(i_frame.size(), CV_32FC3);
        Point3d translation;
        QVector<double> pocX;
        QVector<double> pocY;
        QVector<double> totalAngle;
        int iterace = -1;double oblastMaxima = 5.0;double uhelMaximalni = 0.1;
        int uspech_licovani = completeRegistration(i_cap,i_frame,
                                                   i_framesSecondEval[i],
                                                   iterace,
                                                   oblastMaxima,
                                                   uhelMaximalni,
                                                   i_coutouExtra,
                                                   i_cutoutStandard,
                                                   i_scaleChanged,
                                                   slicovan_kompletne,
                                                   pocX,pocY,totalAngle);
        qDebug() << i_framesSecondEval[i] <<" -> ";
        if (uspech_licovani == 0)
        {

            qDebug()  << "unable to analyse this frame.";
            i_allFrameCompleteEval[int(i_framesSecondEval[i])] = 5.0;
            i_POC_x[int(i_framesSecondEval[i])] = 999.0;
            i_POC_y[int(i_framesSecondEval[i])] = 999.0;
            i_Angle[int(i_framesSecondEval[i])] = 999.0;
            i_frangi_x[int(i_framesSecondEval[i])] = 999.0;
            i_frangi_y[int(i_framesSecondEval[i])] = 999.0;
            i_frangi_euklid[int(i_framesSecondEval[i])] = 999.0;
            continue;
        }
        else
        {
            Mat mezivysledek32f,mezivysledek32f_vyrez;
            slicovan_kompletne.copyTo(mezivysledek32f);
            transformMatTypeTo32C1(mezivysledek32f);
            mezivysledek32f(i_cutoutStandard).copyTo(mezivysledek32f_vyrez);
            double R1 = calculateCorrCoef(i_frame,slicovan_kompletne,i_cutoutStandard);
            mezivysledek32f.release();
            mezivysledek32f_vyrez.release();
            Point3d korekce_bod(0,0,0);
            if (i_scaleChanged == true)
            {
                korekce_bod = fk_translace(i_frame,slicovan_kompletne);
                if (std::abs(korekce_bod.x)>=290 || std::abs(korekce_bod.y)>=290)
                {
                    korekce_bod = fk_translace_hann(i_frame,slicovan_kompletne);
                }
            }
            else
            {
                korekce_bod = fk_translace_hann(i_frame,slicovan_kompletne);
            }
            Mat korekce = frameTranslation(slicovan_kompletne,korekce_bod,rows,cols);
            korekce.copyTo(mezivysledek32f);
            transformMatTypeTo32C1(mezivysledek32f);
            mezivysledek32f(i_cutoutStandard).copyTo(mezivysledek32f_vyrez);
            double R2 = calculateCorrCoef(i_frame,korekce,i_cutoutStandard);
            Point3d registratedFrame_frangi_reverse(0,0,0);
            double rozdil = R2-R1;
            if (rozdil>0.015)
            {
                cv::Point3d extra_translace(0,0,0);
                extra_translace.x = translation.x+korekce_bod.x;
                extra_translace.y = translation.y+korekce_bod.y;
                extra_translace.z = translation.z;
                qDebug()<< "Provedena korekce posunuti pro objektivnejsi analyzu skrze cevy.";
                registratedFrame_frangi_reverse = frangi_analysis(korekce,2,2,0,"",2,extra_translace,i_FrangiParameters,i_margins);//!
            }
            else
            {
                registratedFrame_frangi_reverse = frangi_analysis(slicovan_kompletne,2,2,0,"",2,translation,i_FrangiParameters,i_margins);//!
            }
            slicovan_kompletne.release();
            if (registratedFrame_frangi_reverse.z == 0.0)
            {

                qDebug()<< "Nelze zjistit maximum Frangiho funkce, ohodnoceni: 5";
                i_allFrameCompleteEval[int(i_framesSecondEval[i])] = 5.0;
                i_POC_x[int(i_framesSecondEval[i])] = 999.0;
                i_POC_y[int(i_framesSecondEval[i])] = 999.0;
                i_Angle[int(i_framesSecondEval[i])] = 999.0;
                i_frangi_x[int(i_framesSecondEval[i])] = 999.0;
                i_frangi_y[int(i_framesSecondEval[i])] = 999.0;
                i_frangi_euklid[int(i_framesSecondEval[i])] = 999.0;
                continue;
            }
            else
            {
                double rozdil_x = frame_frangi_reverse.x - registratedFrame_frangi_reverse.x;
                double rozdil_y = frame_frangi_reverse.y - registratedFrame_frangi_reverse.y;
                double suma_rozdilu = std::pow(rozdil_x,2.0) + std::pow(rozdil_y,2.0);
                double euklid = std::sqrt(suma_rozdilu);
                if (euklid <= 1.2)
                {
                    i_allFrameCompleteEval[int(i_framesSecondEval[i])] = 0;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 0";
                }
                else if (euklid > 1.2 && euklid < 10)
                {
                    i_allFrameCompleteEval[int(i_framesSecondEval[i])] = 1;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 1";
                    i_frangi_x[int(i_framesSecondEval[i])] = registratedFrame_frangi_reverse.x;
                    i_frangi_y[int(i_framesSecondEval[i])] = registratedFrame_frangi_reverse.y;
                    i_frangi_euklid[int(i_framesSecondEval[i])] = euklid;
                }
                else if (euklid >=10)
                {
                    i_allFrameCompleteEval[int(i_framesSecondEval[i])] = 4;
                    qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 4";
                    i_frangi_x[int(i_framesSecondEval[i])] = registratedFrame_frangi_reverse.x;
                    i_frangi_y[int(i_framesSecondEval[i])] = registratedFrame_frangi_reverse.y;
                    i_frangi_euklid[int(i_framesSecondEval[i])] = euklid;
                }
                rozdil_x = 0;
                rozdil_y = 0;
                suma_rozdilu = 0;
                euklid = 0;
            }

        }
    }
}
