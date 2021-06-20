#include "registration/phase_correlation_function.h"
#include "image_analysis/image_processing.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include <QDebug>
cv::Point3d pc_translation_hann(const Mat &i_referentialFrame,
                              const Mat &i_shiftedFrame,
                              double i_calcAreaSize)
{
    Point3d translation_result(0.0,0.0,0.0);
    Mat referentialFrame32f,shiftedFrame32f;
    i_referentialFrame.copyTo(referentialFrame32f);
    i_shiftedFrame.copyTo(shiftedFrame32f);
    transformMatTypeTo32C1(referentialFrame32f);
    transformMatTypeTo32C1(shiftedFrame32f);
    cv::Mat hann;
    cv::createHanningWindow(hann, i_referentialFrame.size(), CV_32FC1);
    //transformMatTypeTo32C1(hann);
    qDebug()<<"Referential "<<referentialFrame32f.rows<<" "<<referentialFrame32f.cols;
    qDebug()<<"Shifted "<<shiftedFrame32f.rows<<" "<<shiftedFrame32f.cols;
    translation_result = cv::phaseCorrelate(referentialFrame32f,shiftedFrame32f,i_calcAreaSize,hann);
    qDebug()<<"Hann "<<translation_result.x<<" "<<translation_result.y;
    hann.release();
    referentialFrame32f.release();
    shiftedFrame32f.release();
    return translation_result;
}

cv::Point3d pc_translation(const Mat &i_referentialFrame,
                         const Mat &i_shiftedFrame,
                         double i_calcAreaSize)
{
    Point3d translation_result(0.0,0.0,0.0);
    Mat referentialFrame32f,shiftedFrame32f;
    i_referentialFrame.copyTo(referentialFrame32f);
    i_shiftedFrame.copyTo(shiftedFrame32f);
    transformMatTypeTo32C1(referentialFrame32f);
    transformMatTypeTo32C1(shiftedFrame32f);

    translation_result = cv::phaseCorrelate(referentialFrame32f,shiftedFrame32f,i_calcAreaSize);
    referentialFrame32f.release();
    shiftedFrame32f.release();
    return translation_result;
}

cv::Point3d pc_rotation(const Mat &i_referentialFrame,
                      const Mat &i_shiftedFrame,
                      double i_maximalAngle,
                      const double& i_fkTranslation_maximumValue,
                      cv::Point3d& i_translation)
{
    cv::Point3d translation_result(0.0,0.0,0.0);
    cv::Mat referentialFrame8U,referentialFrame32f, shiftedFrame8U;
    i_referentialFrame.copyTo(referentialFrame8U);
    i_referentialFrame.copyTo(referentialFrame32f);
    i_shiftedFrame.copyTo(shiftedFrame8U);
    transformMatTypeTo8C1(referentialFrame8U);
    transformMatTypeTo8C1(shiftedFrame8U);

    cv::Mat logImage1 = cv::Mat::zeros(referentialFrame8U.size(), CV_8UC1);
    cv::Mat logImage2 = cv::Mat::zeros(shiftedFrame8U.size(), CV_8UC1);
    double magnitude = 40;
    cv::logPolar(referentialFrame8U, logImage1, cv::Point2f(referentialFrame8U.cols / 2.0f, referentialFrame8U.rows / 2.0f), magnitude, cv::INTER_LINEAR);
    cv::logPolar(shiftedFrame8U, logImage2, cv::Point2f(shiftedFrame8U.cols / 2.0f, shiftedFrame8U.rows / 2.0f), magnitude, cv::INTER_LINEAR);
    logImage1.convertTo(logImage1, CV_32FC1);
    logImage2.convertTo(logImage2, CV_32FC1);

    cv::Mat hann;
    //double response = 0;
    cv::createHanningWindow(hann, i_referentialFrame.size(), CV_32FC1);
    cv::Point3d rotation_result(0.0,0.0,0.0);
    if (std::abs(i_translation.x) > 10.0 || std::abs(i_translation.y) > 10.0)
    {rotation_result = cv::phaseCorrelate(logImage1, logImage2,1,hann);}
    else {rotation_result = cv::phaseCorrelate(logImage1, logImage2,5,hann);}

    if (rotation_result.y > i_maximalAngle)
    {
        rotation_result.y = i_maximalAngle;
    }

    cv::Point2f src_center(i_referentialFrame.cols/2.0F, i_referentialFrame.rows/2.0F);
    cv::Mat M = cv::getRotationMatrix2D(src_center,rotation_result.y,1);
    cv::Mat rotationResultFrame = cv::Mat::zeros(i_referentialFrame.size(), CV_32FC1);
    cv::warpAffine(i_shiftedFrame,rotationResultFrame,M,i_shiftedFrame.size(),0,BORDER_REPLICATE);
    transformMatTypeTo32C1(rotationResultFrame);
    transformMatTypeTo32C1(referentialFrame32f);

    cv::Point3d translationAfterRotationResults(0.0,0.0,0.0);
    translationAfterRotationResults = cv::phaseCorrelate(referentialFrame32f,rotationResultFrame,10,hann);
    if (translationAfterRotationResults.z > i_fkTranslation_maximumValue)
    {
        translation_result.y = rotation_result.y*360/(logImage2.cols);
    }
    else if (translationAfterRotationResults.z <= i_fkTranslation_maximumValue)
    {
        translation_result.y = 0;
    }
    translation_result.x = exp(rotation_result.x/magnitude);
    translation_result.z = translationAfterRotationResults.z;

    logImage1.release();
    logImage2.release();
    referentialFrame8U.release();
    shiftedFrame8U.release();
    referentialFrame32f.release();
    rotationResultFrame.release();

    return translation_result;
}

