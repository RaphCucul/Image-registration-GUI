#include "image_analysis/image_processing.h"
#include <opencv2/opencv.hpp>

#include <vector>
#include <math.h>
#include <string>
#include <algorithm>
#include <numeric>
#include <random>
using namespace cv;

cv::Mat frameTranslation(const cv::Mat& i_orig, const cv::Point3d& i_shift, int i_rows, int i_cols)
{
    cv::Mat srcX(i_rows,i_cols,CV_32FC1);
    cv::Mat srcY(i_rows,i_cols,CV_32FC1);
    cv::Mat translation_result;
    //Mat result;
    for (int i = 0; i < i_rows; i++)
    {
        for (int j = 0; j < i_cols; j++)
        {
            srcX.at<float>(i,j) = float(j+i_shift.x);
            srcY.at<float>(i,j) = float(i+i_shift.y);
        }
    }
    cv::remap(i_orig,translation_result,srcX,srcY,cv::INTER_LINEAR);

    srcX.release();
    srcY.release();
    return translation_result;
}
cv::Mat frameRotation(const cv::Mat& i_frameAfterTranslation, const double i_angle)
{
    cv::Point2f src_center(i_frameAfterTranslation.cols/2.0f, i_frameAfterTranslation.rows/2.0f);
    cv::Mat M = cv::getRotationMatrix2D(src_center,i_angle,1);
    cv::Mat rotation_result;// = cv::Mat::zeros(i_frameAfterTranslation.size(), CV_32FC3);
    cv::warpAffine(i_frameAfterTranslation,rotation_result,M,i_frameAfterTranslation.size(),0,BORDER_REPLICATE);
    return rotation_result;
}

void transformMatTypeTo32C1(cv::Mat& i_MatToCheck)
{
    if (i_MatToCheck.type() != 5)
    {
        if (i_MatToCheck.channels() == 3)
        {
            cvtColor(i_MatToCheck,i_MatToCheck,CV_BGR2GRAY);
            i_MatToCheck.convertTo(i_MatToCheck,CV_32FC1);
        }
        else
        {
            i_MatToCheck.convertTo(i_MatToCheck,CV_32FC1);
        }
    }
}

cv::Mat convertMatFrom32FTo8U(cv::Mat& i_MatToCheck)
{
    cv::Mat _returnMat;
    i_MatToCheck.convertTo(_returnMat,CV_8UC1,255,0);
    return _returnMat;
}

void transformMatTypeTo8C1(cv::Mat& i_MatToCheck)
{
    if (i_MatToCheck.type() != 0)
    {
        if (i_MatToCheck.channels() == 3)
        {
            cv::cvtColor(i_MatToCheck,i_MatToCheck,CV_BGR2GRAY);
            i_MatToCheck.convertTo(i_MatToCheck,CV_8UC1);
        }
        else
        {
            i_MatToCheck.convertTo(i_MatToCheck,CV_8UC1);
        }
    }
}

cv::Mat transformMatTypeTo64C1(cv::Mat& i_MatToCheck)
{
    Mat modified;
    if (i_MatToCheck.type() != 6)
    {
        if (i_MatToCheck.channels() == 3)
        {
            cvtColor(i_MatToCheck,modified,CV_BGR2GRAY);
            modified.convertTo(modified,CV_64FC1);
        }
        else
        {
            i_MatToCheck.copyTo(modified);
            modified.convertTo(modified,CV_64FC1);
        }
    }
    return modified;
}

void transformMatTypeTo8C3(cv::Mat& i_MatToCheck)
{
    if (i_MatToCheck.type() != 16)
    {
        if (i_MatToCheck.channels() != 3)
        {
            cv::cvtColor(i_MatToCheck,i_MatToCheck,CV_GRAY2BGR);
            i_MatToCheck.convertTo(i_MatToCheck,CV_8UC3);
        }
        else
        {
            i_MatToCheck.convertTo(i_MatToCheck,CV_8UC3);
        }
    }
}

QString MatType2String(int type) {
    QString r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}

QRect transform_CV_RECT_to_QRect(cv::Rect i_rect){
    return QRect(i_rect.x,i_rect.y,i_rect.width,i_rect.height);
}

cv::Rect transform_QRect_to_CV_RECT(QRect i_rect){
    return cv::Rect(i_rect.x(),i_rect.y(),i_rect.width(),i_rect.height());
}

QPoint transform_CV_POINT_to_QPoint(cv::Point i_point){
    return QPoint(i_point.x,i_point.y);
}

QPointF transform_CV_POINTF_to_QPointF(cv::Point3d i_point){
    return QPointF(i_point.x,i_point.y);
}

cv::Point3d transform_QPoint_to_CV_Point3d(QPoint i_point){
    return cv::Point3d(i_point.x(),i_point.y(),0.0);
}
