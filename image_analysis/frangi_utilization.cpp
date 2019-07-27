#include "image_analysis/frangi_utilization.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "image_analysis/image_processing.h"
#include <vector>
#include <math.h>
#include <string>
#include <QDebug>
#include <QVector>
using namespace cv;
cv::Mat imageFiltrationPreprocessing(const cv::Mat& i_inputImage, float i_sigma_s, float i_sigma_r)
{
    Mat filtered = cv::Mat::zeros(i_inputImage.rows,i_inputImage.cols,CV_8UC3);
    Mat outputImage;
    medianBlur(i_inputImage,filtered,5);
    Mat RF_frame = cv::Mat::zeros(i_inputImage.rows,i_inputImage.cols,CV_8UC3);
    transformMatTypeTo8C3(filtered);
    edgePreservingFilter(filtered,RF_frame,1,i_sigma_s,i_sigma_r);

    if (RF_frame.channels() == 3)
    {
        Mat ch1, ch3;
        std::vector<Mat> channels(3);
        split(RF_frame, channels);
        ch1 = channels[0];
        outputImage = channels[1];
        ch3 = channels[2];
    }
    else{
        RF_frame.copyTo(outputImage);
    }
    transformMatTypeTo8C1(outputImage);
    return outputImage;
}

void borderProcessing(cv::Mat &i_inputImage, int i_imageType, int i_padding_top, int i_padding_bottom, int i_padding_left, int i_padding_right)
{
    if (i_imageType == 1)
    {
        Rect area1(0,0,i_inputImage.cols,i_padding_top); // first 20 rows
        Mat area11 = i_inputImage(area1);
        Scalar tempVal = mean(area11);
        double average_area_11 = tempVal.val[0];
        i_inputImage(area1).setTo(average_area_11,i_inputImage(area1) > average_area_11);
        //rectangle(i_inputImage, area1, Scalar(255), 1, 8, 0);

        Rect area2(0,i_inputImage.rows-i_padding_bottom,i_inputImage.cols,i_padding_bottom); // last 20 rows
        Mat area22 = i_inputImage(area2);
        tempVal = mean(area22);
        double average_area_22 = tempVal.val[0];
        i_inputImage(area2).setTo(average_area_22,i_inputImage(area2) > average_area_22);
        //rectangle(i_inputImage, area2, Scalar(255), 1, 8, 0);

        Rect area3(0,0,i_padding_left,i_inputImage.rows); // first 20 columns
        Mat area33 = i_inputImage(area3);
        tempVal = mean(area33);
        double average_area_33 = tempVal.val[0];
        i_inputImage(area3).setTo(average_area_33,i_inputImage(area3) > average_area_33);
        //rectangle(i_inputImage, area3, Scalar(255), 1, 8, 0);

        Rect area4(i_inputImage.cols-i_padding_right,0,i_padding_right,i_inputImage.rows); // last 20 columns
        Mat area44 = i_inputImage(area4);
        tempVal = mean(area44);
        double average_area_44 = tempVal.val[0];
        i_inputImage(area4).setTo(average_area_44,i_inputImage(area4) > average_area_44);
        //rectangle(i_inputImage, area4, Scalar(255), 1, 8, 0);
    }
    if (i_imageType == 2)
    {
        Q_UNUSED(i_padding_left)
        Q_UNUSED(i_padding_right)
        if (i_padding_top <= 0)
        {
            Rect area1(0,0,i_inputImage.cols,std::abs(i_padding_top)); // first 20 rows
            Mat area11 = i_inputImage(area1);
            Scalar tempVal = mean(area11);
            double average_area_11 = tempVal.val[0];
            i_inputImage(area1).setTo(average_area_11,i_inputImage(area1) > average_area_11);
            //rectangle(i_inputImage, area1, Scalar(255), 1, 8, 0);
        }
        if (i_padding_top >= 0)
        {
            Rect area2(0,i_inputImage.rows-std::abs(i_padding_bottom),i_inputImage.cols,std::abs(i_padding_bottom)); // last 20 rows
            Mat area22 = i_inputImage(area2);
            Scalar tempVal = mean(area22);
            double average_area_22 = tempVal.val[0];
            i_inputImage(area2).setTo(average_area_22,i_inputImage(area2) > average_area_22);
            //rectangle(i_inputImage, area2, Scalar(255), 1, 8, 0);
        }
        if (i_padding_bottom <= 0)
        {
            Rect area3(0,0,std::abs(i_padding_left),i_inputImage.rows); // first 20 columns
            Mat area33 = i_inputImage(area3);
            Scalar tempVal = mean(area33);
            double average_area_33 = tempVal.val[0];
            i_inputImage(area3).setTo(average_area_33,i_inputImage(area3) > average_area_33);
            //rectangle(i_inputImage, area3, Scalar(255), 1, 8, 0);
        }
        if (i_padding_bottom >= 0)
        {
            Rect area4(i_inputImage.cols-std::abs(i_padding_bottom),0,std::abs(i_padding_bottom),i_inputImage.rows); // poslednich 20 sloupcu
            Mat area44 = i_inputImage(area4);
            Scalar tempVal = mean(area44);
            double average_area_44 = tempVal.val[0];
            i_inputImage(area4).setTo(average_area_44,i_inputImage(area4) > average_area_44);
            //rectangle(i_inputImage, area4, Scalar(255), 1, 8, 0);
        }
    }
    //imshow("Margins",i_inputImage);
    transformMatTypeTo32C1(i_inputImage);
}

void zeroBorders(cv::Mat& i_inputImage, int i_imageType, int i_padding_top,
                 int i_padding_bottom, int i_padding_left, int i_padding_right)
{
    if (i_imageType == 1)
    {
        Rect area1(0,0,i_inputImage.cols,i_padding_top); // rows
        i_inputImage(area1).setTo(0);

        Rect area2(0,i_inputImage.rows-i_padding_bottom,i_inputImage.cols,i_padding_bottom); // rows
        i_inputImage(area2).setTo(0);

        Rect area3(0,0,i_padding_left,i_inputImage.rows); // columns
        i_inputImage(area3).setTo(0);

        Rect area4(i_inputImage.cols-i_padding_right,0,i_padding_right,i_inputImage.rows); // columns
        i_inputImage(area4).setTo(0);
    }
    if (i_imageType == 2)
    {
        Q_UNUSED(i_padding_left)
        Q_UNUSED(i_padding_right)
        if (i_padding_top <= 0)
        {
            Rect area1(0,0,i_inputImage.cols,std::abs(i_padding_top)+40);
            i_inputImage(area1).setTo(0);
            Rect area2(0,i_inputImage.rows-40,i_inputImage.cols,40);
            i_inputImage(area2).setTo(0);
        }
        if (i_padding_top >= 0)
        {
            Rect area2(0,i_inputImage.rows-std::abs(i_padding_top)-40,
                         i_inputImage.cols,std::abs(i_padding_top)+40);
            i_inputImage(area2).setTo(0);
            Rect area1(0,0,i_inputImage.cols,40);
            i_inputImage(area1).setTo(0);
        }
        if (i_padding_bottom <= 0)
        {
            Rect area3(0,0,std::abs(i_padding_bottom)+40,i_inputImage.rows);
            i_inputImage(area3).setTo(0);
            Rect area4(i_inputImage.cols-40,0,40,i_inputImage.rows);
            i_inputImage(area4).setTo(0);
        }
        if (i_padding_bottom >= 0)
        {
            Rect area4(i_inputImage.cols-std::abs(i_padding_bottom)-40,0,
                         std::abs(i_padding_bottom)+40,i_inputImage.rows);
            i_inputImage(area4).setTo(0);
            Rect area3(0,0,40,i_inputImage.rows);
            i_inputImage(area3).setTo(0);
        }
        //imshow("Zeros",vstupni_obraz);
    }
}


cv::Point2d FrangiSubpixel(const cv::Mat &i_frangi,
                                   const double& i_maximum_frangi,
                                   const cv::Point& i_maximumFrangiCoords)
{
    Point2d output(0,0);
    int counter = 0;
    float euklid = 0;
    float difference_sum = 0;
    int difference_x = 0;
    int difference_y = 0;
    float frangi_x = 0;
    float frangi_y = 0;
    float pixelValue = 0;
    for (int i = 0; i<i_frangi.rows; i++)
    {
        for (int j = 0; j<i_frangi.cols; j++)
        {
            if (i_frangi.at<float>(i,j)>=float(0.98*i_maximum_frangi))
            {
                difference_x = i_maximumFrangiCoords.x - j;
                difference_y = i_maximumFrangiCoords.y - i;
                difference_sum = float(std::pow(difference_x,2.0f) + std::pow(difference_y,2.0f));
                euklid = std::sqrt(difference_sum);
                if (euklid < 3)
                {
                    counter+=1;
                    pixelValue += i_frangi.at<float>(i,j);
                    frangi_x += j*i_frangi.at<float>(i,j);
                    frangi_y += i*i_frangi.at<float>(i,j);
                }
            }
        }
    }
    output.x = double(frangi_x/pixelValue);
    output.y = double(frangi_y/pixelValue);
    return output;
}

cv::Point3d frangi_analysis(const cv::Mat i_inputFrame,
                            int i_processingMode,
                            int i_accuracy,
                            int i_showResult,
                            QString i_windowName,
                            int i_frameType,
                            cv::Point3d i_translation,
                            QVector<double> i_FrangiParameters,
                            QMap<QString,int> i_margins)
{
    Point3d definitiveCoords;
    frangi2d_opts_t opts;
    frangi2d_createopts(&opts);
    if (i_processingMode == 2)
    {
        opts.BlackWhite = false;
    }
    else if(i_processingMode == 1)
    {
        opts.BlackWhite = true;
    }
    opts.sigma_start = int(i_FrangiParameters[0]);
    opts.sigma_step = int(i_FrangiParameters[1]);
    opts.sigma_end = int(i_FrangiParameters[2]);
    opts.BetaOne = i_FrangiParameters[3];
    opts.BetaTwo = i_FrangiParameters[4];

    int r = int(i_translation.y);
    int s = int(i_translation.x);

    Mat imageFiltered,imageFrangi,obraz_scale, imageAngles;
    imageFiltered = imageFiltrationPreprocessing(i_inputFrame,60.0f,0.4f);
    //qDebug()<<"filtration processed";
    if (i_frameType == 1) {borderProcessing(imageFiltered,1,
                                            i_margins["top_m"],i_margins["bottom_m"],
                                            i_margins["left_m"],i_margins["right_m"]);}
    else if (i_frameType == 2) {borderProcessing(imageFiltered,2,r,s,0,0);}
    else if (i_frameType == 3) {borderProcessing(imageFiltered,1,
                                                 20,20,
                                                 20,20);}
    //qDebug()<<"border processed";
    cv::Mat _pom;
    imageFiltered.copyTo(_pom);
    transformMatTypeTo8C3(_pom);
    //cv::imshow("forFrangi",_pom);

    frangi2d(imageFiltered, imageFrangi, obraz_scale, imageAngles, opts);

    //imwrite("frangi",imageFrangi);
    obraz_scale.release();
    imageAngles.release();
    imageFiltered.release();
    if (i_frameType == 1) {zeroBorders(imageFrangi,1,i_margins["top_m"],i_margins["bottom_m"],
                            i_margins["left_m"],i_margins["right_m"]);}
    if (i_frameType == 2) {zeroBorders(imageFrangi,2,r,s,0,0);}

    double maximum_imageFrangi;
    Point max_loc_frangi;
    cv::minMaxLoc(imageFrangi, NULL, &maximum_imageFrangi, NULL, &max_loc_frangi);
    if ((max_loc_frangi.x!=max_loc_frangi.x)== 1 || (max_loc_frangi.y!=max_loc_frangi.y) == 1)
    {
        definitiveCoords.z = 0.0;
        definitiveCoords.x = -10;
        definitiveCoords.y = -10;
    }
    else
    {
        if (i_accuracy == 1)
        {
            definitiveCoords.x = max_loc_frangi.x;
            definitiveCoords.y = max_loc_frangi.y;
            definitiveCoords.z = 1.0;
        }
        if (i_accuracy == 2)
        {
            cv::Point2d teziste = FrangiSubpixel(imageFrangi,maximum_imageFrangi,max_loc_frangi);
            definitiveCoords.x = teziste.x;
            definitiveCoords.y = teziste.y;
            definitiveCoords.z = 1.0;
        }
        if (i_showResult == 1)
        {
            namedWindow(i_windowName.toLocal8Bit().constData());
            drawMarker(imageFrangi,max_loc_frangi,(0));
            cv::imshow(i_windowName.toLocal8Bit().constData(),imageFrangi);
        }
    }
    return definitiveCoords;
}
