#include "multithreadET/qthreadthirdpart.h"
#include <QDebug>
#include <QVector>
#include <QStringList>
#include <QThread>

#include "licovani/fazova_korelace_funkce.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "util/souborove_operace.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using cv::Mat;
using cv::Rect;

qThreadThirdPart::qThreadThirdPart(QStringList &sV, QVector<QVector<int> >& sSprOhKomplet,
                                   QVector<QVector<int> > &ohodKomp, QVector<int>& refKomplet,
                                   QVector<double> &RK, QVector<double> &FWHMK,
                                   cv::Rect &VK_standard, cv::Rect &VK_extra, bool zM, QObject *parent):QThread(parent)
{
    videoList = sV;
    badFrames_firstEvaluation = sSprOhKomplet;
    framesEvaluationCompelte = ohodKomp;
    referencialFrames = refKomplet;
    obtainedCutoffStandard = VK_standard;
    obtainedCutoffExtra = VK_extra;
    scaleChanged = zM;
    averageCCcomplete = RK;
    averageFWHMcomplete = FWHMK;
}

void qThreadThirdPart::run()
{
    /// Sixth part - using average values of FWHM and correlation coefficient, the bad frames are split into multiple
    /// categories according to the their values of FWHM and correlation coefficient.

    emit typeOfMethod(2);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    bool errorOccured = false;
    for (int indexVidea=0; indexVidea < videoList.count(); indexVidea++)
    {
        QVector<int> spatneSnimky;
        spatneSnimky = badFrames_firstEvaluation[indexVidea];
        QString fullPath = videoList.at(indexVidea);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(indexVidea);
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        if (!cap.isOpened()){
            emit unexpectedTermination("Cannot open a video for analysis",3,"hardError");
            errorOccured = true;
            break;
        }
        cv::Mat referencni_snimek_temp,referencni_snimek,referencni_snimek32f,referencni_vyrez;
        cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[indexVidea]);
        if (!cap.read(referencni_snimek_temp))
        {
            emit unexpectedTermination("Referencial image cannot be read!",3,"hardError");
            errorOccured = true;
            break;
        }
        int rows = 0;
        int cols = 0;
        if (scaleChanged == true)
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
        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> pomVecD(frameCount,0.0);
        QVector<double> POC_x,POC_y,uhel,frangi_x,frangi_y,frangi_euklid,vypoctene_hodnoty_FWHM,
                vypoctene_hodnoty_R;
        frangi_x = pomVecD;frangi_y=pomVecD;frangi_euklid=pomVecD;
        POC_x=pomVecD;POC_y=pomVecD;uhel=pomVecD;
        QVector<int> snimky_k_provereni_prvni;
        frameCount = spatneSnimky.length();
        for (int i = 0;i < spatneSnimky.length(); i++)
        {
            emit percentageCompleted(qRound((double(indexVidea)/videoCount)*100.0+((double(i)/double(frameCount))*100.0)/videoCount));
            Mat posunuty_temp,posunuty,posunuty_vyrez,slicovany,slicovany_vyrez;
            Mat obraz, obraz_vyrez;
            referencni_snimek.copyTo(obraz);
            obraz(obtainedCutoffStandard).copyTo(obraz_vyrez);

            cap.set(CV_CAP_PROP_POS_FRAMES,spatneSnimky[i]);
            if (cap.read(posunuty_temp) != 1)
            {
                QString errorMessage = QString("Frame number %1 could not be opened").arg(i);
                unexpectedTermination(errorMessage,1,"softError");
                POC_x[badFrames_firstEvaluation[indexVidea][i]] = 999.0;
                POC_y[badFrames_firstEvaluation[indexVidea][i]] = 999.0;
                uhel[badFrames_firstEvaluation[indexVidea][i]] = 999.0;
                frangi_x[badFrames_firstEvaluation[indexVidea][i]] = 999.0;
                frangi_y[badFrames_firstEvaluation[indexVidea][i]] = 999.0;
                frangi_euklid[badFrames_firstEvaluation[indexVidea][i]] = 999.0;
                continue;
            }
            if (scaleChanged == true)
            {
                posunuty_temp(obtainedCutoffExtra).copyTo(posunuty);
                posunuty(obtainedCutoffStandard).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
                posunuty(obtainedCutoffStandard).copyTo(posunuty_vyrez);
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
                qDebug()<< "Frame "<< spatneSnimky[i]<< " will not be analysed.";
                framesEvaluationCompelte[indexVidea][spatneSnimky[i]] = 5;
                POC_x[badFrames_firstEvaluation[indexVidea][i]] = 999;
                POC_y[badFrames_firstEvaluation[indexVidea][i]] = 999;
                uhel[badFrames_firstEvaluation[indexVidea][i]] = 999;
                frangi_x[badFrames_firstEvaluation[indexVidea][i]] = 999;
                frangi_y[badFrames_firstEvaluation[indexVidea][i]] = 999;
                frangi_euklid[badFrames_firstEvaluation[indexVidea][i]] = 999;
                posunuty.release();
            }
            else
            {
                double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt.z);
                double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                slicovany = translace_snimku(posunuty,pt,rows,cols);
                slicovany(obtainedCutoffStandard).copyTo(slicovany_vyrez);
                double R = vypocet_KK(obraz,slicovany,obtainedCutoffStandard);
                qDebug() <<"Tested frame has "<<spatneSnimky[i]<< "R " << R <<" a FWHM " << FWHM;
                slicovany.release();
                slicovany_vyrez.release();
                posunuty.release();
                double rozdilnostKK = averageCCcomplete[indexVidea]-R;
                double rozdilnostFWHM = averageFWHMcomplete[indexVidea]-FWHM;
                if ((std::abs(rozdilnostKK) < 0.02) && (FWHM < averageFWHMcomplete[indexVidea])) //1.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " ready for registration.";
                    framesEvaluationCompelte[indexVidea][badFrames_firstEvaluation[indexVidea][i]] = 0.0;
                    continue;
                }
                else if (R > averageCCcomplete[indexVidea] && (std::abs(rozdilnostFWHM)<=2||(FWHM < averageFWHMcomplete[indexVidea]))) //5.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " ready for registration.";
                    framesEvaluationCompelte[indexVidea][badFrames_firstEvaluation[indexVidea][i]] = 0.0;
                    continue;
                }
                else if (R >= averageCCcomplete[indexVidea] && FWHM > averageFWHMcomplete[indexVidea]) //4.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " ready for registration.";
                    framesEvaluationCompelte[indexVidea][badFrames_firstEvaluation[indexVidea][i]] = 0.0;
                    continue;
                }
                else if ((std::abs(rozdilnostKK) <= 0.02) && (FWHM > averageFWHMcomplete[indexVidea])) //2.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " will be analysed in the next step.";
                    snimky_k_provereni_prvni.push_back(badFrames_firstEvaluation[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }
                else if ((rozdilnostKK > 0.02) && (rozdilnostKK < 0.18)) //3.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " will be analysed in the next step.";
                    snimky_k_provereni_prvni.push_back(badFrames_firstEvaluation[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }
                else if ((rozdilnostKK >= 0.05 && rozdilnostKK < 0.18) && ((FWHM < averageFWHMcomplete[indexVidea]) || averageFWHMcomplete[indexVidea] > 35.0)) //6.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " will be analysed in the next step.";
                    snimky_k_provereni_prvni.push_back(badFrames_firstEvaluation[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }
                else if ((rozdilnostKK >= 0.05 && rozdilnostKK < 0.18) && (FWHM <= (averageFWHMcomplete[indexVidea]+10))) //8.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " will be analysed in the next step.";
                    snimky_k_provereni_prvni.push_back(badFrames_firstEvaluation[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }

                else if ((rozdilnostKK >= 0.2) && (FWHM > (averageFWHMcomplete[indexVidea]+10))) //7.
                {
                    qDebug()<< "Frame "<< spatneSnimky[i]<< " will not be registrated.";
                    framesEvaluationCompelte[indexVidea][badFrames_firstEvaluation[indexVidea][i]] = 5;
                    POC_x[badFrames_firstEvaluation[indexVidea][i]] = 999;
                    POC_y[badFrames_firstEvaluation[indexVidea][i]] = 999;
                    uhel[badFrames_firstEvaluation[indexVidea][i]] = 999;
                    frangi_x[badFrames_firstEvaluation[indexVidea][i]] = 999;
                    frangi_y[badFrames_firstEvaluation[indexVidea][i]] = 999;
                    frangi_euklid[badFrames_firstEvaluation[indexVidea][i]] = 999;
                    continue;
                }
                else
                {
                    qDebug() << "Frame "<< spatneSnimky[i]<< " will be analysed in the next step.";
                    snimky_k_provereni_prvni.push_back(badFrames_firstEvaluation[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);

                }
            }

        }
        framesComputedCC.append(vypoctene_hodnoty_R);
        framesComputedFWHM.append(vypoctene_hodnoty_FWHM);
        framesFirstCompleteEvaluation.append(snimky_k_provereni_prvni);
        framesFrangiX.append(frangi_x);
        framesFrangiY.append(frangi_y);
        framesFrangiEuklid.append(frangi_euklid);
        framesPOCX.append(POC_x);
        framesPOCY.append(POC_y);
        framesUhel.append(uhel);
    }
    if (!errorOccured){
        emit percentageCompleted(100);
        emit done(3);
    }
}
QVector<QVector<int>> qThreadThirdPart::framesUpdateEvaluation()
{
    return framesEvaluationCompelte;
}
QVector<QVector<int>> qThreadThirdPart::framesFirstEvaluationComplete()
{
    return framesFirstCompleteEvaluation;
}
QVector<QVector<double>> qThreadThirdPart::framesProblematic_CC()
{
    return framesComputedCC;
}
QVector<QVector<double>> qThreadThirdPart::framesProblematic_FWHM()
{
    return framesComputedFWHM;
}
QVector<QVector<double>> qThreadThirdPart::framesFrangiXestimated()
{
    return framesFrangiX;
}
QVector<QVector<double>> qThreadThirdPart::framesFrangiYestimated()
{
    return framesFrangiY;
}
QVector<QVector<double>> qThreadThirdPart::framesFrangiEuklidestimated()
{
    return framesFrangiEuklid;
}
QVector<QVector<double>> qThreadThirdPart::framesPOCXestimated()
{
    return framesPOCX;
}
QVector<QVector<double>> qThreadThirdPart::framesPOCYestimated()
{
    return framesPOCY;
}
QVector<QVector<double>> qThreadThirdPart::framesUhelestimated()
{
    return framesUhel;
}
