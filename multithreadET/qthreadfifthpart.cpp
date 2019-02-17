#include "multithreadET/qthreadfifthpart.h"
#include "licovani/fazova_korelace_funkce.h"
#include "licovani/multiPOC_Ai1.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/korelacni_koeficient.h"

#include <QThread>
#include <QStringList>
#include <QVector>
#include <QDebug>

#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using cv::Mat;
using cv::Rect;
using cv::Point3d;

qThreadFifthPart::qThreadFifthPart(QStringList& videos,
                                   cv::Rect& CO_s,
                                   cv::Rect& CO_e,
                                   QVector<QVector<double>>& POCX,
                                   QVector<QVector<double>>& POCY,
                                   QVector<QVector<double>>& Angle,
                                   QVector<QVector<double>>& Fr_X,
                                   QVector<QVector<double>>& Fr_Y,
                                   QVector<QVector<double>>& Fr_E,
                                   bool scaleChanged,
                                   QVector<QVector<int>> &EvaluationComplete,
                                   QVector<QVector<int>>& frEvalSec,
                                   QVector<int>& referFrames,
                                   QVector<double> FrangiParams,
                                   QObject *parent):QThread(parent)
{
    videoList = videos;
    obtainedCutoffStandard = CO_s;
    obtainedCutoffExtra = CO_e;
    POC_x = POCX;
    POC_y = POCY;
    angle = Angle;
    frangi_x = Fr_X;
    frangi_y = Fr_Y;
    frangi_euklid = Fr_E;
    scaleCh = scaleChanged;
    framesCompleteEvaluation = EvaluationComplete;
    framesSecondEval = frEvalSec;
    FrangiParameters = FrangiParams;
    referencialFrames = referFrames;
}

void qThreadFifthPart::run()
{
    emit typeOfMethod(4);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    for (int indexVidea = 0; indexVidea < videoList.count(); indexVidea++)
    {
        frameCount = double(framesSecondEval[indexVidea].length());
        QString fullPath = videoList.at(indexVidea);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(indexVidea);
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        if (!cap.isOpened())
        {
            qWarning()<<"Unable to open"+fullPath;
            break;
        }
        cv::Mat referencni_snimek_temp,referencni_snimek,referencni_snimek32f,referencni_vyrez;
        cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[indexVidea]);
        if (!cap.read(referencni_snimek_temp))
        {
                qWarning()<<"Referrence image cannot be read!";
                break;
        }
        int rows;
        int cols;
        if (scaleCh == true)
        {
            referencni_snimek_temp(obtainedCutoffExtra).copyTo(referencni_snimek);
            rows = referencni_snimek.rows;
            cols = referencni_snimek.cols;
            referencni_snimek(obtainedCutoffStandard).copyTo(referencni_vyrez);
            referencni_snimek_temp.release();
        }
        else
        {
            referencni_snimek_temp.copyTo(referencni_snimek);
            rows = referencni_snimek.rows;
            cols = referencni_snimek.cols;
            referencni_snimek(obtainedCutoffStandard).copyTo(referencni_vyrez);
            referencni_snimek_temp.release();
        }
        cv::Point3d pt_temp(0,0,0);
        Point3d obraz_frangi_reverse = frangi_analysis(referencni_snimek,2,2,0,"",1,pt_temp,FrangiParameters);
        Mat obraz_vyrez;
        referencni_snimek(obtainedCutoffStandard).copyTo(referencni_vyrez);
        //for (unsigned int i = 0; i < 1; i++)
        frameCount = double(framesSecondEval[indexVidea].length());
        for (int i = 0; i < framesSecondEval[indexVidea].length(); i++) //snimky_k_provereni2.size()
        {
            emit percentageCompleted(qRound((indexVidea/videoCount)*100+((i/frameCount)*100.0)/videoCount));
            Mat slicovan_kompletne = cv::Mat::zeros(referencni_snimek.size(), CV_32FC3);
            Point3d mira_translace(0.0,0.0,0.0);
            double celkovy_angle = 0.0;
            int iterace = -1;double oblastMaxima = 5.0;double angleMaximalni = 0.1;
            int uspech_licovani = completeRegistration(cap,referencni_snimek,
                                                      framesSecondEval[indexVidea][i],
                                                      iterace,
                                                      oblastMaxima,
                                                      angleMaximalni,
                                                      obtainedCutoffExtra,
                                                      obtainedCutoffStandard,
                                                      scaleCh,
                                                      slicovan_kompletne,
                                                      mira_translace,celkovy_angle);
            qDebug() << framesSecondEval[indexVidea][i] <<" -> ";
            if (uspech_licovani == 0)
            {

                qDebug()  << "nelze slicovat, ohodnocení: 5";
                framesCompleteEvaluation[indexVidea][framesSecondEval[indexVidea][i]] = 5.0;
                POC_x[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                POC_y[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                angle[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                frangi_x[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                frangi_y[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                frangi_euklid[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                continue;
            }
            else
            {
                Mat mezivysledek32f,mezivysledek32f_vyrez;
                slicovan_kompletne.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(obtainedCutoffStandard).copyTo(mezivysledek32f_vyrez);
                double R1 = vypocet_KK(referencni_snimek,slicovan_kompletne,obtainedCutoffStandard);
                mezivysledek32f.release();
                mezivysledek32f_vyrez.release();
                Point3d korekce_bod(0,0,0);
                if (scaleCh == true)
                {
                    korekce_bod = fk_translace(referencni_snimek,slicovan_kompletne);
                    if (std::abs(korekce_bod.x)>=290 || std::abs(korekce_bod.y)>=290)
                    {
                        korekce_bod = fk_translace_hann(referencni_snimek,slicovan_kompletne);
                    }
                }
                else
                {
                    korekce_bod = fk_translace_hann(referencni_snimek,slicovan_kompletne);
                }
                Mat korekce = translace_snimku(slicovan_kompletne,korekce_bod,rows,cols);
                korekce.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(obtainedCutoffStandard).copyTo(mezivysledek32f_vyrez);
                double R2 = vypocet_KK(referencni_snimek,korekce,obtainedCutoffStandard);
                Point3d slicovany_frangi_reverse(0,0,0);
                double rozdil = R2-R1;
                if (rozdil>0.015)
                {
                    cv::Point3d extra_translace(0,0,0);
                    extra_translace.x = mira_translace.x+korekce_bod.x;
                    extra_translace.y = mira_translace.y+korekce_bod.y;
                    extra_translace.z = mira_translace.z;
                    qDebug()<< "Provedena korekce posunuti pro objektivnejsi analyzu skrze cevy.";
                    slicovany_frangi_reverse = frangi_analysis(korekce,2,2,0,"",2,extra_translace,FrangiParameters);
                }
                else
                {
                    slicovany_frangi_reverse = frangi_analysis(slicovan_kompletne,2,2,0,"",2,mira_translace,FrangiParameters);
                }
                slicovan_kompletne.release();
                if (slicovany_frangi_reverse.z == 0.0)
                {

                    qDebug()<< "Nelze zjistit maximum Frangiho funkce, ohodnoceni: 5";
                    framesCompleteEvaluation[indexVidea][framesSecondEval[indexVidea][i]] = 5.0;
                    POC_x[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                    POC_y[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                    angle[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                    frangi_x[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                    frangi_y[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
                    frangi_euklid[indexVidea][framesSecondEval[indexVidea][i]] = 999.0;
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
                        framesCompleteEvaluation[indexVidea][framesSecondEval[indexVidea][i]] = 0;
                        qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 0";
                    }
                    else if (euklid > 1.2 && euklid < 10)
                    {
                        framesCompleteEvaluation[indexVidea][framesSecondEval[indexVidea][i]] = 1;
                        qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 1";
                        frangi_x[indexVidea][framesSecondEval[indexVidea][i]] = slicovany_frangi_reverse.x;
                        frangi_y[indexVidea][framesSecondEval[indexVidea][i]] = slicovany_frangi_reverse.y;
                        frangi_euklid[indexVidea][framesSecondEval[indexVidea][i]] = euklid;
                    }
                    else if (euklid >=10)
                    {
                        framesCompleteEvaluation[indexVidea][framesSecondEval[indexVidea][i]] = 4;
                        qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 4";
                        frangi_x[indexVidea][framesSecondEval[indexVidea][i]] = slicovany_frangi_reverse.x;
                        frangi_y[indexVidea][framesSecondEval[indexVidea][i]] = slicovany_frangi_reverse.y;
                        frangi_euklid[indexVidea][framesSecondEval[indexVidea][i]] = euklid;
                    }
                    rozdil_x = 0;
                    rozdil_y = 0;
                    suma_rozdilu = 0;
                    euklid = 0;
                }

            }
        }

    }
    emit percentageCompleted(100);
    emit done(5);
}

QVector<QVector<int>> qThreadFifthPart::framesUpdateEvaluationComplete()
{
    return framesCompleteEvaluation;
}
QVector<QVector<double>> qThreadFifthPart::framesFrangiXestimated()
{
    return frangi_x;
}
QVector<QVector<double>> qThreadFifthPart::framesFrangiYestimated()
{
    return frangi_y;
}
QVector<QVector<double>> qThreadFifthPart::framesFrangiEuklidestimated()
{
    return frangi_euklid;
}
QVector<QVector<double>> qThreadFifthPart::framesPOCXestimated()
{
    return POC_x;
}
QVector<QVector<double>> qThreadFifthPart::framesPOCYestimated()
{
    return POC_y;
}
QVector<QVector<double>> qThreadFifthPart::framesAngleestimated()
{
    return angle;
}
