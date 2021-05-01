#include "registration/registration_correction.h"
#include "registration/multiPOC_Ai1.h"
#include "image_analysis/image_processing.h"
#include "image_analysis/correlation_coefficient.h"
#include "image_analysis/frangi_utilization.h"
#include "registration/phase_correlation_function.h"

#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QMap>

using cv::Mat;
using cv::VideoCapture;
using cv::VideoWriter;
using cv::Rect;
using cv::Point3d;
using cv::Size;

bool registrateBestFrames(cv::VideoCapture& i_cap,
                          cv::Mat& i_referentialFrame,
                          cv::Mat& i_shiftedFrame,
                          cv::Point3d i_coordsFrangiStandardReferencialReverse,
                          int i_index_moved,
                          double i_iteration,
                          double i_areaMaximum,
                          double i_angle,
                          cv::Rect i_cutoutExtra,
                          cv::Rect i_cutoutStandard,
                          bool i_scaleChanged,
                          QMap<QString,double> i_frangiParameters,
                          QVector<double> &_pocX,
                          QVector<double> &_pocY,
                          QVector<double> &_frangiX,
                          QVector<double> &_frangiY,
                          QVector<double> &_frangiEucl,
                          QVector<double> &_maxAngles,
                          QMap<QString,int> i_margins,
                          bool video)
{
    Mat fullyRegistratedFrame = cv::Mat::zeros(cv::Size(i_referentialFrame.cols,i_referentialFrame.rows), CV_32FC3);
    QVector<double> _tempPOCX = _pocX;
    QVector<double> _tempPOCY = _pocY;
    QVector<double> _tempAngles = _maxAngles;
    _frangiX.push_back(0.0);
    _frangiY.push_back(0.0);
    _frangiEucl.push_back(0.0);
    /*cv::Rect adjustedCutoutStandard = adjustStandardCutout(i_cutoutExtra,i_cutoutStandard,
                                                           i_referencialFrame.rows,i_referencialFrame.cols);*/

    if (!completeRegistration(i_cap,
                              i_referentialFrame,
                              i_shiftedFrame,
                              i_index_moved,
                              i_iteration,
                              i_areaMaximum,
                              i_angle,
                              i_cutoutExtra,
                              i_cutoutStandard,
                              i_scaleChanged,
                              fullyRegistratedFrame,
                              _tempPOCX,_tempPOCY,_tempAngles,video)){
        qWarning()<< "Registration error. Frame "<<i_index_moved<<" was not registrated.";
        _frangiX[0]=999.0;
        _frangiY[0]=999.0;
        _frangiEucl[0]=999.0;
        fullyRegistratedFrame.release();
        return false;
    }
    else
    {
        qDebug()<<"Full registration correct. Results: "<<_tempPOCX[0]<<" "<<_tempPOCY[0];
        qDebug()<<"Full registration correct. Rows: "<<fullyRegistratedFrame.rows<<" cols: "<<fullyRegistratedFrame.cols;

        // because of extra cutout, fullyRegistratedFrame do not need to have "full" size
        cv::Mat _referentialSubstituion,referencialCutout;
        //if (i_scaleChanged){
           // i_referencialFrame(i_cutoutExtra).copyTo(_referentialSubstituion);
            //_referentialSubstituion(adjustedCutoutStandard).copyTo(referencialCutout);
        //}
        //else{
            i_referentialFrame.copyTo(_referentialSubstituion);
            _referentialSubstituion(i_cutoutStandard).copyTo(referencialCutout);
        //}

        int rows = _referentialSubstituion.rows;
        int cols = _referentialSubstituion.cols;
        if (std::abs(_tempPOCX[0]) == 999.0)
        {
            qWarning()<< "Frame "<<i_index_moved<<" written without changes.";
            fullyRegistratedFrame.release();
            _frangiX[0]=999.0;
            _frangiY[0]=999.0;
            _frangiEucl[0]=999.0;
            return false;
        }
        else
        {
            Mat shifted_temp;
            if (video) {
                i_cap.set(CV_CAP_PROP_POS_FRAMES,i_index_moved);
                if (i_cap.read(shifted_temp)!=1)
                {
                    qWarning()<<"Frame "<<i_index_moved<<" could not be read!";
                    return false;
                }
            }
            else {
                i_shiftedFrame.copyTo(shifted_temp);
            }

            Mat interresult32f,interresult32f_vyrez,shifted;
            if (i_scaleChanged == true)
                shifted_temp(i_cutoutExtra).copyTo(shifted);
            else
                shifted_temp.copyTo(shifted);

            Point3d correction(0.0,0.0,0.0);
            Mat fullyRegistratedFrame_correction = cv::Mat::zeros(cv::Size(_referentialSubstituion.cols,_referentialSubstituion.rows),
                                                                  CV_32FC3);
            if (!translationCorrection(fullyRegistratedFrame,
                                       _referentialSubstituion,
                                       fullyRegistratedFrame_correction,
                                       i_cutoutStandard,
                                       correction,
                                       i_areaMaximum)){
                qWarning()<<"Frame "<<i_index_moved<<" - registration correction failed";
                return false;
            }
            else{
                if (correction.x > 0.0 || correction.y > 0.0)
                {
                    qDebug()<<"Correction: "<<correction.x<<" "<<correction.y;
                    _tempPOCX[0] += correction.x;
                    _tempPOCY[0] += correction.y;
                    Point3d pt6 = pc_translation_hann(_referentialSubstituion,fullyRegistratedFrame_correction,
                                                      i_areaMaximum);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = pc_translation(_referentialSubstituion,fullyRegistratedFrame_correction,
                                             i_areaMaximum);
                    }
                    qDebug()<<"Checking translation after correction: "<<pt6.x<<" "<<pt6.y;
                    _tempPOCX[0] += pt6.x;
                    _tempPOCY[0] += pt6.y;
                }

                fullyRegistratedFrame_correction.copyTo(interresult32f);
                transformMatTypeTo32C1(interresult32f);

                //if (!i_scaleChanged)
                    interresult32f(i_cutoutStandard).copyTo(interresult32f_vyrez);
                //else
                    //interresult32f(adjustedCutoutStandard).copyTo(interresult32f_vyrez);

                double CC_first = calculateCorrCoef(_referentialSubstituion,fullyRegistratedFrame_correction,
                                                    i_cutoutStandard);
                Point3d _tempTranslation = Point3d(_tempPOCX[0],_tempPOCY[0],0.0);
                int mode = i_frangiParameters.contains("mode") ? i_frangiParameters["mode"] : 1;
                qDebug()<<"Applying frangi mode "<<mode;
                Point3d frangi_registrated_reverse = frangi_analysis(fullyRegistratedFrame_correction,mode,2,0,"",2,
                                                                     _tempTranslation,i_frangiParameters,i_margins);

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

                Mat shifted2 = frameTranslation(shifted,finalTranslation,rows,cols);
                Mat finalRegistrationFrame = frameRotation(shifted2,_tempAngles[0]);

                shifted2.release();
                double CC_second = calculateCorrCoef(_referentialSubstituion,finalRegistrationFrame,
                                                     i_cutoutStandard);
                if (CC_first >= CC_second)
                {
                    qDebug()<< "Frame "<<i_index_moved<<" written after standard registration.";
                    qDebug()<<"R1: "<<CC_first<<" R2: "<<CC_second;
                    qDebug()<<"Translation: "<<_tempPOCX[0]<<" "<<_tempPOCY[0]<<" "<<_tempAngles[0];
                    if (i_scaleChanged == true)
                    {
                        int rows = shifted_temp.rows;
                        int cols = shifted_temp.cols;
                        Mat shifted_original = frameTranslation(shifted_temp,_tempTranslation,rows,cols);
                        Mat finalRegistrationFrame2 = frameRotation(shifted_original,_tempAngles[0]);
                        fullyRegistratedFrame.release();
                        fullyRegistratedFrame_correction.release();
                        finalRegistrationFrame2.release();
                        finalRegistrationFrame.release();
                        shifted_original.release();
                        shifted_temp.release();
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
                    qDebug()<< "Frame "<<i_index_moved<<" written after vein analysis";
                    qDebug()<<" Translation: "<<finalTranslation.x<<" "<<finalTranslation.y<<" "<<_tempAngles[0];
                    if (i_scaleChanged == true)
                    {
                        int rows = shifted_temp.rows;
                        int cols = shifted_temp.cols;
                        Mat shifted_original = frameTranslation(shifted_temp,finalTranslation,rows,cols);
                        Mat finalRegistration2 = frameRotation(shifted_original,_tempAngles[0]);
                        fullyRegistratedFrame.release();
                        fullyRegistratedFrame_correction.release();
                        finalRegistration2.release();
                        shifted_original.release();
                        shifted_temp.release();
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
                interresult32f.release();
                interresult32f_vyrez.release();
                return true;
            }
        }
    }
}

bool translationCorrection(cv::Mat& i_frameRegistrated,
                                  cv::Mat& i_frame,
                                  cv::Mat &i_frameCorrected,
                                  cv::Rect &i_cutoutStandard,
                                  cv::Point3d &i_correction,
                                  double i_areaMaximum)
{
    try {
        Mat interresult,interresult32f,interresult32f_vyrez,frameCutout;
        i_frameRegistrated.copyTo(interresult);
        int rows = i_frameRegistrated.rows;
        int cols = i_frameRegistrated.cols;
        i_frame(i_cutoutStandard).copyTo(frameCutout);

        double CC_first = calculateCorrCoef(i_frame,interresult,i_cutoutStandard);
        Point3d correction_translation(0.0,0.0,0.0);
        correction_translation = pc_translation_hann(i_frame,interresult,i_areaMaximum);
        if (std::abs(correction_translation.x) > 290.0 || std::abs(correction_translation.y) > 290.0)
        {
            correction_translation = pc_translation(i_frame,interresult,i_areaMaximum);
        }
        if (std::abs(correction_translation.x) > 290.0 || std::abs(correction_translation.y) > 290.0)
        {
            correction_translation = pc_translation(frameCutout,interresult32f_vyrez,i_areaMaximum);
        }
        i_frameCorrected = frameTranslation(interresult,correction_translation,rows,cols);
        double CC_second = calculateCorrCoef(i_frame,i_frameCorrected,i_cutoutStandard);
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
