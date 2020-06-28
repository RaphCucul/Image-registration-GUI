#include "registration/phase_correlation_function.h"
#include "image_analysis/image_processing.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
cv::Point3d pc_translation_hann(const Mat &i_referencialFrame,
                              const Mat &i_shiftedFrame,
                              double i_calcAreaSize)
{
    Point3d translation_result(0.0,0.0,0.0);
    Mat referencialFrame32f,shiftedFrame32f;
    i_referencialFrame.copyTo(referencialFrame32f);
    i_shiftedFrame.copyTo(shiftedFrame32f);
    transformMatTypeTo32C1(referencialFrame32f);
    transformMatTypeTo32C1(shiftedFrame32f);
    cv::Mat hann;
    cv::createHanningWindow(hann, referencialFrame32f.size(), CV_32FC1);
    transformMatTypeTo32C1(hann);
    translation_result = cv::phaseCorrelate(referencialFrame32f,shiftedFrame32f,i_calcAreaSize,hann);

    hann.release();
    referencialFrame32f.release();
    shiftedFrame32f.release();
    return translation_result;
}

cv::Point3d pc_translation(const Mat &referencni_snimek,
                         const Mat &i_shiftedFrame,
                         double i_calcAreaSize)
{
    Point3d translation_result(0.0,0.0,0.0);
    Mat referencialFrame32f,shiftedFrame32f;
    referencni_snimek.copyTo(referencialFrame32f);
    i_shiftedFrame.copyTo(shiftedFrame32f);
    transformMatTypeTo32C1(referencialFrame32f);
    transformMatTypeTo32C1(shiftedFrame32f);

    translation_result = cv::phaseCorrelate(referencialFrame32f,shiftedFrame32f,i_calcAreaSize);

    referencialFrame32f.release();
    shiftedFrame32f.release();
    return translation_result;
}

cv::Point3d pc_rotation(const Mat &i_referencialFrame,
                      const Mat &i_shiftedFrame,
                      double i_maximalAngle,
                      const double& i_fkTranslation_maximumValue,
                      cv::Point3d& i_translation)
{
    cv::Point3d translation_result(0.0,0.0,0.0);
    cv::Mat referencialFrame8U,referencialFrame32f, shiftedFrame8U;
    i_referencialFrame.copyTo(referencialFrame8U);
    i_referencialFrame.copyTo(referencialFrame32f);
    i_shiftedFrame.copyTo(shiftedFrame8U);
    transformMatTypeTo8C1(referencialFrame8U);
    transformMatTypeTo8C1(shiftedFrame8U);

    cv::Mat logImage1 = cv::Mat::zeros(referencialFrame8U.size(), CV_8UC1);
    cv::Mat logImage2 = cv::Mat::zeros(shiftedFrame8U.size(), CV_8UC1);
    double magnitude = 40;
    cv::logPolar(referencialFrame8U, logImage1, cv::Point2f(referencialFrame8U.cols / 2.0f, referencialFrame8U.rows / 2.0f), magnitude, cv::INTER_LINEAR);
    cv::logPolar(shiftedFrame8U, logImage2, cv::Point2f(shiftedFrame8U.cols / 2.0f, shiftedFrame8U.rows / 2.0f), magnitude, cv::INTER_LINEAR);
    logImage1.convertTo(logImage1, CV_32FC1);
    logImage2.convertTo(logImage2, CV_32FC1);

    cv::Mat hann;
    //double response = 0;
    cv::createHanningWindow(hann, i_referencialFrame.size(), CV_32FC1);
    cv::Point3d rotation_result(0.0,0.0,0.0);
    if (std::abs(i_translation.x) > 10.0 || std::abs(i_translation.y) > 10.0)
    {rotation_result = cv::phaseCorrelate(logImage1, logImage2,1,hann);}
    else {rotation_result = cv::phaseCorrelate(logImage1, logImage2,5,hann);}

    if (rotation_result.y > i_maximalAngle)
    {
        rotation_result.y = i_maximalAngle;
    }

    cv::Point2f src_center(i_referencialFrame.cols/2.0F, i_referencialFrame.rows/2.0F);
    cv::Mat M = cv::getRotationMatrix2D(src_center,rotation_result.y,1);
    cv::Mat rotationResultFrame = cv::Mat::zeros(i_referencialFrame.size(), CV_32FC1);
    cv::warpAffine(i_shiftedFrame,rotationResultFrame,M,i_shiftedFrame.size(),0,BORDER_REPLICATE);
    transformMatTypeTo32C1(rotationResultFrame);
    transformMatTypeTo32C1(referencialFrame32f);

    cv::Point3d translationAfterRotationResults(0.0,0.0,0.0);
    translationAfterRotationResults = cv::phaseCorrelate(referencialFrame32f,rotationResultFrame,10,hann);

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
    referencialFrame8U.release();
    shiftedFrame8U.release();
    referencialFrame32f.release();
    //posunuty.release();
    rotationResultFrame.release();
    //hann.release();

    return translation_result;
}

