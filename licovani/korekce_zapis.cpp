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

#include <QDebug>

using cv::Mat;
using cv::VideoCapture;
using cv::VideoWriter;
using cv::Rect;
using cv::Point3d;
using cv::Size;

bool licovani_nejvhodnejsich_snimku(cv::VideoCapture& i_cap,
                                    cv::Mat& i_referencialFrame,
                                    cv::Point3d i_coordsFrangiStandardReferencialReverse,
                                    int i_index_noved,
                                    int i_iteration,
                                    double i_areaMaximum,
                                    double i_angle,
                                    cv::Rect& i_cutoutExtra,
                                    cv::Rect& i_cutoutStandard,
                                    bool i_scaleChanged,
                                    QVector<double> i_frangiParameters,
                                    QVector<double> &_pocX,
                                    QVector<double> &_pocY,
                                    QVector<double> &_frangiX,
                                    QVector<double> &_frangiY,
                                    QVector<double> &_frangiEucl,
                                    QVector<double> &_maxAngles)
{
    Mat fullyRegistratedFrame = cv::Mat::zeros(cv::Size(i_referencialFrame.cols,i_referencialFrame.rows), CV_32FC3);
    QVector<double> _tempPOCX = _pocX;
    QVector<double> _tempPOCY = _pocY;
    QVector<double> _tempAngles = _maxAngles;
    _frangiX.push_back(0.0);
    _frangiY.push_back(0.0);
    _frangiEucl.push_back(0.0);

    if (!completeRegistration(i_cap,
                              i_referencialFrame,
                              i_index_noved,
                              i_iteration,
                              i_areaMaximum,
                              i_angle,
                              i_cutoutExtra,
                              i_cutoutStandard,
                              i_scaleChanged,
                              fullyRegistratedFrame,
                              _tempPOCX,_tempPOCY,_tempAngles)){
        qWarning()<< "Registration error. Frame "<<i_index_noved<<" was not registrated.";
        _frangiX[0]=999.0;
        _frangiY[0]=999.0;
        _frangiEucl[0]=999.0;
        fullyRegistratedFrame.release();
        return false;
    }
    else
    {
        qDebug()<<"Full registration correct. Results: "<<_tempPOCX[0]<<" "<<_tempPOCY[0];
        Mat referencialCutout;
        i_referencialFrame(i_cutoutStandard).copyTo(referencialCutout);
        int rows = i_referencialFrame.rows;
        int cols = i_referencialFrame.cols;
        if (std::abs(_tempPOCX[0]) == 999.0)
        {
            qWarning()<< "Frame "<<i_index_noved<<" written without changes.";
            fullyRegistratedFrame.release();
            _frangiX[0]=999.0;
            _frangiY[0]=999.0;
            _frangiEucl[0]=999.0;
            return false;
        }
        else
        {
            Mat posunuty_temp;
            i_cap.set(CV_CAP_PROP_POS_FRAMES,i_index_noved);
            if (i_cap.read(posunuty_temp)!=1)
            {
                qWarning()<<"Frame "<<i_index_noved<<" could not be read!";
                return false;
            }
            Mat mezivysledek32f,mezivysledek32f_vyrez,posunuty;
            if (i_scaleChanged == true)
            {
                posunuty_temp(i_cutoutExtra).copyTo(posunuty);
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
            }

            Point3d translationCorrection(0.0,0.0,0.0);
            Mat fullyRegistratedFrame_correction = cv::Mat::zeros(cv::Size(i_referencialFrame.cols,i_referencialFrame.rows), CV_32FC3);
            if (!eventualni_korekce_translace(fullyRegistratedFrame,i_referencialFrame,fullyRegistratedFrame_correction,
                                              i_cutoutStandard,translationCorrection)){
                qWarning()<<"Frame "<<i_index_noved<<" - registration correction failed";
                return false;
            }
            else{
                if (translationCorrection.x > 0.0 || translationCorrection.y > 0.0)
                {
                    qDebug()<<"Correction: "<<translationCorrection.x<<" "<<translationCorrection.y;
                    _tempPOCX[0] += translationCorrection.x;
                    _tempPOCY[0] += translationCorrection.y;
                    Point3d pt6 = fk_translace_hann(i_referencialFrame,fullyRegistratedFrame_correction);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = fk_translace(i_referencialFrame,fullyRegistratedFrame_correction);
                    }
                    qDebug()<<"Checking translation after correction: "<<pt6.x<<" "<<pt6.y;
                    _tempPOCX[0] += pt6.x;
                    _tempPOCY[0] += pt6.y;
                }

                fullyRegistratedFrame_correction.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(i_cutoutStandard).copyTo(mezivysledek32f_vyrez);
                double CC_first = vypocet_KK(i_referencialFrame,fullyRegistratedFrame_correction,i_cutoutStandard);
                Point3d _tempTranslation = Point3d(_tempPOCX[0],_tempPOCY[0],0.0);

                Point3d frangi_registrated_reverse = frangi_analysis(fullyRegistratedFrame_correction,2,2,0,"",2,_tempTranslation,i_frangiParameters);

                _frangiX[0] = frangi_registrated_reverse.x;
                _frangiY[0] = frangi_registrated_reverse.y;
                double yydef = i_coordsFrangiStandardReferencialReverse.x - frangi_registrated_reverse.x;
                double xxdef = i_coordsFrangiStandardReferencialReverse.y - frangi_registrated_reverse.y;
                double differenceSum = std::pow(xxdef,2.0) + std::pow(yydef,2.0);
                double euklid = std::sqrt(differenceSum);
                _frangiEucl[0] = euklid;

                Point3d finalTranslation(0.0,0.0,0.0);
                finalTranslation.y = _tempPOCY[0] - yydef;
                finalTranslation.x = _tempPOCX[0] - xxdef;
                finalTranslation.z = 0;
                xxdef = 0.0;
                yydef = 0.0;
                differenceSum = 0.0;
                euklid = 0.0;

                Mat posunuty2 = translace_snimku(posunuty,finalTranslation,rows,cols);
                Mat finalRegistrationFrame = rotace_snimku(posunuty2,_tempAngles[0]);

                posunuty2.release();
                double CC_second = vypocet_KK(i_referencialFrame,finalRegistrationFrame,i_cutoutStandard);
                if (CC_first >= CC_second)
                {
                    qDebug()<< "Frame "<<i_index_noved<<" written after standard registration.";
                    qDebug()<<"R1: "<<CC_first<<" R2: "<<CC_second;
                    qDebug()<<"Translation: "<<_tempPOCX[0]<<" "<<_tempPOCY[0]<<" "<<_tempAngles[0];
                    if (i_scaleChanged == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,_tempTranslation,radky,sloupce);
                        Mat finalRegistrationFrame2 = rotace_snimku(posunuty_original,_tempAngles[0]);
                        fullyRegistratedFrame.release();
                        fullyRegistratedFrame_correction.release();
                        finalRegistrationFrame2.release();
                        finalRegistrationFrame.release();
                        posunuty_original.release();
                        posunuty_temp.release();
                    }
                    else
                    {
                        fullyRegistratedFrame.release();
                        finalRegistrationFrame.release();
                    }
                    _pocX = _tempPOCX;
                    _pocY = _tempPOCY;
                    _maxAngles = _tempAngles;
                }
                else
                {
                    qDebug()<< "Frame "<<i_index_noved<<" written after vein analysis";
                    qDebug()<<" Translation: "<<finalTranslation.x<<" "<<finalTranslation.y<<" "<<_tempAngles[0];
                    if (i_scaleChanged == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,finalTranslation,radky,sloupce);
                        Mat finalRegistration2 = rotace_snimku(posunuty_original,_tempAngles[0]);
                        fullyRegistratedFrame.release();
                        fullyRegistratedFrame_correction.release();
                        finalRegistration2.release();
                        posunuty_original.release();
                        posunuty_temp.release();
                    }
                    else
                    {
                        fullyRegistratedFrame.release();
                    }
                    _tempPOCX[0] = finalTranslation.x;
                    _tempPOCY[0] = finalTranslation.y;
                    _pocX = _tempPOCX;
                    _pocY = _tempPOCY;
                    _maxAngles = _tempAngles;
                }
                mezivysledek32f.release();
                mezivysledek32f_vyrez.release();
                return true;
            }
        }
    }
}

bool eventualni_korekce_translace(cv::Mat& i_frameRegistrated, cv::Mat& i_frame, cv::Mat &i_frameCorrected,
                                  cv::Rect &i_cutoutStandard,
                                     cv::Point3d &i_correction)
{
    try {
        Mat mezivysledek,mezivysledek32f,mezivysledek32f_vyrez,frameCutout;
        i_frameRegistrated.copyTo(mezivysledek);
        int rows = i_frameRegistrated.rows;
        int cols = i_frameRegistrated.cols;
        //mezivysledek.copyTo(mezivysledek32f);
        //kontrola_typu_snimku_32C1(mezivysledek32f);
        //mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
        i_frame(i_cutoutStandard).copyTo(frameCutout);

        double CC_first = vypocet_KK(i_frame,mezivysledek,i_cutoutStandard);

        //mezivysledek32f.release();
        //mezivysledek32f_vyrez.release();

        Point3d correction_translation(0.0,0.0,0.0);
        correction_translation = fk_translace_hann(i_frame,mezivysledek);
        if (std::abs(correction_translation.x) > 290.0 || std::abs(correction_translation.y) > 290.0)
        {
            correction_translation = fk_translace(i_frame,mezivysledek);
        }
        if (std::abs(correction_translation.x) > 290.0 || std::abs(correction_translation.y) > 290.0)
        {
            correction_translation = fk_translace(frameCutout,mezivysledek32f_vyrez);
        }
        i_frameCorrected = translace_snimku(mezivysledek,correction_translation,rows,cols);
        double CC_second = vypocet_KK(i_frame,i_frameCorrected,i_cutoutStandard);
        if ((CC_second > CC_first) && ((std::abs(correction_translation.x) > 0.3) || (std::abs(correction_translation.y) > 0.3)))
        {
            i_correction = correction_translation;
            return true;
        }
        else
            return true;

    } catch (std::exception &e) {
        qWarning()<<"Registration correction error: "<<e.what();
        return false;
    }
}
