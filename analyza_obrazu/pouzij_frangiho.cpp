#include "analyza_obrazu/pouzij_frangiho.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "analyza_obrazu/upravy_obrazu.h"
#include <vector>
#include <math.h>
#include <string>
#include <QDebug>
#include <QVector>
using namespace cv;
cv::Mat imageFiltrationPreprocessing(const cv::Mat& inputImage, float sigma_s, float sigma_r)
{
    //qDebug()<<inputImage.rows<<" "<<inputImage.cols;
    Mat filtered = cv::Mat::zeros(inputImage.rows,inputImage.cols,CV_8UC3);
    Mat outputImage;
    medianBlur(inputImage,filtered,5);
    Mat RF_obraz = cv::Mat::zeros(inputImage.rows,inputImage.cols,CV_8UC3);
    kontrola_typu_snimku_8C3(filtered);
    edgePreservingFilter(filtered,RF_obraz,1,sigma_s,sigma_r);
    //imshow("Filtrovany RF",RF_obraz);
    //qDebug()<<"Filtration completed.";
    if (RF_obraz.channels() == 3)
    {
        Mat ch1, ch3;
        std::vector<Mat> channels(3);
        split(RF_obraz, channels);
        ch1 = channels[0];
        outputImage = channels[1];
        ch3 = channels[2];
    }
    else{
        RF_obraz.copyTo(outputImage);
    }
    //imshow("Vybrany snimek",obrazek_vystupni);
    //qDebug()<<"Pocet kanalu vystupniho filtrovaneho snimku je "<<inputImage.channels();
    kontrola_typu_snimku_8C1(outputImage);
    return outputImage;
}

void borderProcessing(cv::Mat &inputImage, int imageType, int padding_r, int padding_s)
{
    if (imageType == 1)
    {
        Rect area1(0,0,inputImage.cols,padding_r); // first 20 rows
        Mat area11 = inputImage(area1);
        Scalar tempVal = mean(area11);
        double average_area_11 = tempVal.val[0];
        inputImage(area1).setTo(average_area_11,inputImage(area1) > average_area_11);

        Rect area2(0,inputImage.rows-padding_r,inputImage.cols,padding_r); // last 20 rows
        Mat area22 = inputImage(area2);
        tempVal = mean(area22);
        double average_area_22 = tempVal.val[0];
        inputImage(area2).setTo(average_area_22,inputImage(area2) > average_area_22);

        Rect area3(0,0,padding_s,inputImage.rows); // first 20 columns
        Mat area33 = inputImage(area3);
        tempVal = mean(area33);
        double average_area_33 = tempVal.val[0];
        inputImage(area3).setTo(average_area_33,inputImage(area3) > average_area_33);

        Rect area4(inputImage.cols-padding_s,0,padding_s,inputImage.rows); // last 20 columns
        Mat area44 = inputImage(area4);
        tempVal = mean(area44);
        double average_area_44 = tempVal.val[0];
        inputImage(area4).setTo(average_area_44,inputImage(area4) > average_area_44);
    }
    if (imageType == 2)
    {
        if (padding_r <= 0)
        {
            Rect area1(0,0,inputImage.cols,std::abs(padding_r)); // prvnich 20 radku
            Mat area11 = inputImage(area1);
            Scalar tempVal = mean(area11);
            double average_area_11 = tempVal.val[0];
            inputImage(area1).setTo(average_area_11,inputImage(area1) > average_area_11);
        }
        if (padding_r >= 0)
        {
            Rect area2(0,inputImage.rows-std::abs(padding_r),inputImage.cols,std::abs(padding_r)); // poslednich 20 radku
            Mat area22 = inputImage(area2);
            Scalar tempVal = mean(area22);
            double average_area_22 = tempVal.val[0];
            inputImage(area2).setTo(average_area_22,inputImage(area2) > average_area_22);
        }
        if (padding_s <= 0)
        {
            Rect area3(0,0,std::abs(padding_s),inputImage.rows); // prvnich 20 sloupcu
            Mat area33 = inputImage(area3);
            Scalar tempVal = mean(area33);
            double average_area_33 = tempVal.val[0];
            inputImage(area3).setTo(average_area_33,inputImage(area3) > average_area_33);
        }
        if (padding_s >= 0)
        {
            Rect area4(inputImage.cols-std::abs(padding_s),0,std::abs(padding_s),inputImage.rows); // poslednich 20 sloupcu
            Mat area44 = inputImage(area4);
            Scalar tempVal = mean(area44);
            double average_area_44 = tempVal.val[0];
            inputImage(area4).setTo(average_area_44,inputImage(area4) > average_area_44);
        }
    }
    kontrola_typu_snimku_32C1(inputImage);
    /*rectangle(vstupni_obraz, area1, Scalar(255), 1, 8, 0);
    rectangle(vstupni_obraz, area2, Scalar(255), 1, 8, 0);
    rectangle(vstupni_obraz, area3, Scalar(255), 1, 8, 0);
    rectangle(vstupni_obraz, area4, Scalar(255), 1, 8, 0);*/
    //imshow("Okraje",vstupni_obraz);
}


void zeroBorders(cv::Mat& inputImage, int imageType, int padding_r, int padding_s)
{
    if (imageType == 1)
    {
        Rect area1(0,0,inputImage.cols,padding_r); // rows
        inputImage(area1).setTo(0);

        Rect area2(0,inputImage.rows-padding_r,inputImage.cols,padding_r); // rows
        inputImage(area2).setTo(0);

        Rect area3(0,0,padding_s,inputImage.rows); // columns
        inputImage(area3).setTo(0);

        Rect area4(inputImage.cols-padding_s,0,padding_s,inputImage.rows); // columns
        inputImage(area4).setTo(0);
    }
    if (imageType == 2)
    {
        if (padding_r <= 0)
        {
            Rect area1(0,0,inputImage.cols,std::abs(padding_r)+40);
            inputImage(area1).setTo(0);
            Rect area2(0,inputImage.rows-40,inputImage.cols,40);
            inputImage(area2).setTo(0);
        }
        if (padding_r >= 0)
        {
            Rect area2(0,inputImage.rows-std::abs(padding_r)-40,
                         inputImage.cols,std::abs(padding_r)+40);
            inputImage(area2).setTo(0);
            Rect area1(0,0,inputImage.cols,40);
            inputImage(area1).setTo(0);
        }
        if (padding_s <= 0)
        {
            Rect area3(0,0,std::abs(padding_s)+40,inputImage.rows);
            inputImage(area3).setTo(0);
            Rect area4(inputImage.cols-40,0,40,inputImage.rows);
            inputImage(area4).setTo(0);
        }
        if (padding_s >= 0)
        {
            Rect area4(inputImage.cols-std::abs(padding_s)-40,0,
                         std::abs(padding_s)+40,inputImage.rows);
            inputImage(area4).setTo(0);
            Rect area3(0,0,40,inputImage.rows);
            inputImage(area3).setTo(0);
        }
        //imshow("Nulovani",vstupni_obraz);
    }
}


cv::Point2d FrangiSubpixel(const cv::Mat &frangi,
                                   const double& maximum_frangi,
                                   const cv::Point& maximumFrangiCoords)
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
    for (int i = 0; i<frangi.rows; i++)
    {
        for (int j = 0; j<frangi.cols; j++)
        {
            if (frangi.at<float>(i,j)>=float(0.98*maximum_frangi))
            {
                difference_x = maximumFrangiCoords.x - j;
                difference_y = maximumFrangiCoords.y - i;
                difference_sum = float(std::pow(difference_x,2.0f) + std::pow(difference_y,2.0f));
                euklid = std::sqrt(difference_sum);
                if (euklid < 3)
                {
                    counter+=1;
                    pixelValue += frangi.at<float>(i,j);
                    frangi_x += j*frangi.at<float>(i,j);
                    frangi_y += i*frangi.at<float>(i,j);
                }
            }
        }
    }
    output.x = double(frangi_x/pixelValue);
    output.y = double(frangi_y/pixelValue);
    return output;
}

cv::Point3d frangi_analysis(const cv::Mat inputFrame,
                           int processingMode,
                           int accuracy,
                           int showResult,
                           QString windowName,
                           int frameType,
                           cv::Point3d translation,
                           QVector<double> FrangiParameters)
{
    Point3d definitiveCoords;
    frangi2d_opts_t opts;
    frangi2d_createopts(&opts);
    if (processingMode == 2)
    {
        opts.BlackWhite = false;
    }
    else if(processingMode == 1)
    {
        opts.BlackWhite = true;
    }
    opts.sigma_start = int(FrangiParameters[0]);
    opts.sigma_step = int(FrangiParameters[1]);
    opts.sigma_end = int(FrangiParameters[2]);
    opts.BetaOne = FrangiParameters[3];
    opts.BetaTwo = FrangiParameters[4];

    int r = int(translation.y);
    int s = int(translation.x);

    Mat imageFiltered,imageFrangi,obraz_scale, imageAngles;
    imageFiltered = imageFiltrationPreprocessing(inputFrame,60.0f,0.4f);
    //qDebug()<<"filtration processed";
    if (frameType == 1) {borderProcessing(imageFiltered,1,20,20);}
    if (frameType == 2) {borderProcessing(imageFiltered,2,r,s);}
    //qDebug()<<"border processed";
    //if (frameType == 2 && pritomnost_casove_znamky == 1) {borderProcessing(imageFiltered,2,40,40);}
    //if (frameType == 1 && pritomnost_casove_znamky == 1) {borderProcessing(imageFiltered,1,80,50);}

    frangi2d(imageFiltered, imageFrangi, obraz_scale, imageAngles, opts);

    //qDebug()<<"Pocet kanalu frangiho vystupu "<<imageFrangi.channels()<<" "<<imageFrangi.type();
    //imwrite(jmeno_okna+"",imageFrangi);
    obraz_scale.release();
    imageAngles.release();
    imageFiltered.release();
    if (frameType == 1) {zeroBorders(imageFrangi,1,50,50);}
    if (frameType == 2) {zeroBorders(imageFrangi,2,r,s);}
    //if (frameType == 2 && pritomnost_casove_znamky == 1) {zeroBorders(imageFrangi,2,80,80);}
    //if (frameType == 1 && pritomnost_casove_znamky == 1) {zeroBorders(imageFrangi,1,250,80);}

    double maximum_imageFrangi;
    Point max_loc_frangi;
    cv::minMaxLoc(imageFrangi, NULL, &maximum_imageFrangi, NULL, &max_loc_frangi);
    if ((max_loc_frangi.x!=max_loc_frangi.x)== 1 || (max_loc_frangi.y!=max_loc_frangi.y) == 1)
    {
        //qDebug()<<"Maximum Frangiho funkce se nepodarilo detekovat!";
        definitiveCoords.z = 0.0;
        definitiveCoords.x = -10;
        definitiveCoords.y = -10;
    }
    else
    {
        //qDebug()<<"Detekce maxima Frangiho funkce se zdarila.";
        if (accuracy == 1)
        {
            definitiveCoords.x = max_loc_frangi.x;
            definitiveCoords.y = max_loc_frangi.y;
            definitiveCoords.z = 1.0;
        }
        if (accuracy == 2)
        {
            cv::Point2d teziste = FrangiSubpixel(imageFrangi,maximum_imageFrangi,max_loc_frangi);
            definitiveCoords.x = teziste.x;
            definitiveCoords.y = teziste.y;
            definitiveCoords.z = 1.0;
        }
        if (showResult == 1)
        {
            drawMarker(imageFrangi,max_loc_frangi,(0));
            cv::imshow(windowName.toLocal8Bit().constData(),imageFrangi);
        }
    }
    return definitiveCoords;
}
