#include "analyza_obrazu/upravy_obrazu.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
//#include <plplot.h>
//#include <plstream.h>

#include <vector>
#include <math.h>
#include <string>
#include <algorithm>
#include <numeric>
#include <random>
using namespace cv;

cv::Mat translace_snimku(const cv::Mat& posunuty_original,const cv::Point3d& hodnoty_translace,int radky,int sloupce)
{
    cv::Mat srcX(radky,sloupce,CV_32FC1);
    cv::Mat srcY(radky,sloupce,CV_32FC1);
    cv::Mat vysledek_translace;
    //Mat result;
    for (int i = 0; i < radky; i++)
    {
        for (int j = 0; j < sloupce; j++)
        {
            srcX.at<float>(i,j) = float(j+hodnoty_translace.x);
            srcY.at<float>(i,j) = float(i+hodnoty_translace.y);
        }
    }
    cv::remap(posunuty_original,vysledek_translace,srcX,srcY,cv::INTER_LINEAR);

    srcX.release();
    srcY.release();
    return vysledek_translace;
}
cv::Mat rotace_snimku(const cv::Mat& snimek_po_translaci, const double uhel)
{
    cv::Point2f src_center(snimek_po_translaci.cols/2.0f, snimek_po_translaci.rows/2.0f);
    cv::Mat M = cv::getRotationMatrix2D(src_center,uhel,1);
    cv::Mat vysledny_snimek_rotace;// = cv::Mat::zeros(snimek_po_translaci.size(), CV_32FC3);
    cv::warpAffine(snimek_po_translaci,vysledny_snimek_rotace,M,snimek_po_translaci.size(),0,BORDER_REPLICATE);
    return vysledny_snimek_rotace;
}
void ukaz_Mat(std::string jmeno_okna,cv::Mat snimek_k_zobrazeni)
{
    cv::namedWindow(jmeno_okna);
    cv::Mat snimek_k_zobrazeni8U = cv::Mat::zeros(snimek_k_zobrazeni.rows,snimek_k_zobrazeni.cols,CV_8UC3);
    snimek_k_zobrazeni.copyTo(snimek_k_zobrazeni8U);
    snimek_k_zobrazeni.release();
    if (snimek_k_zobrazeni8U.type() != 0)
    {
        if (snimek_k_zobrazeni8U.channels() == 3)
        {
            cv::cvtColor(snimek_k_zobrazeni8U,snimek_k_zobrazeni8U,CV_BGR2GRAY);
            snimek_k_zobrazeni8U.convertTo(snimek_k_zobrazeni8U,CV_8UC1);
        }
        else
        {
            snimek_k_zobrazeni8U.convertTo(snimek_k_zobrazeni8U,CV_8UC1);
        }
    }
    imshow(jmeno_okna,snimek_k_zobrazeni8U);
    snimek_k_zobrazeni8U.release();
}

void kontrola_typu_snimku_32C1(cv::Mat& snimek_ke_kontrole)
{
    if (snimek_ke_kontrole.type() != 5)
    {
        if (snimek_ke_kontrole.channels() == 3)
        {
            cvtColor(snimek_ke_kontrole,snimek_ke_kontrole,CV_BGR2GRAY);
            snimek_ke_kontrole.convertTo(snimek_ke_kontrole,CV_32FC1);
        }
        else
        {
            snimek_ke_kontrole.convertTo(snimek_ke_kontrole,CV_32FC1);
        }
    }
}

void kontrola_typu_snimku_8C1(cv::Mat& snimek_ke_kontrole)
{
    if (snimek_ke_kontrole.type() != 0)
    {
        if (snimek_ke_kontrole.channels() == 3)
        {
            cv::cvtColor(snimek_ke_kontrole,snimek_ke_kontrole,CV_BGR2GRAY);
            snimek_ke_kontrole.convertTo(snimek_ke_kontrole,CV_8UC1);
        }
        else
        {
            snimek_ke_kontrole.convertTo(snimek_ke_kontrole,CV_8UC1);
        }
    }
}

cv::Mat kontrola_typu_snimku_64C1(cv::Mat& snimek_ke_kontrole)
{
    Mat upraveny;
    if (snimek_ke_kontrole.type() != 6)
    {
        if (snimek_ke_kontrole.channels() == 3)
        {
            cvtColor(snimek_ke_kontrole,upraveny,CV_BGR2GRAY);
            upraveny.convertTo(upraveny,CV_64FC1);
        }
        else
        {
            snimek_ke_kontrole.copyTo(upraveny);
            upraveny.convertTo(upraveny,CV_64FC1);
        }
    }
    return upraveny;
}

void kontrola_typu_snimku_8C3(cv::Mat& snimek_ke_kontrole)
{
    if (snimek_ke_kontrole.type() != 16)
    {
        if (snimek_ke_kontrole.channels() != 3)
        {
            cv::cvtColor(snimek_ke_kontrole,snimek_ke_kontrole,CV_GRAY2BGR);
            snimek_ke_kontrole.convertTo(snimek_ke_kontrole,CV_8UC3);
        }
        else
        {
            snimek_ke_kontrole.convertTo(snimek_ke_kontrole,CV_8UC3);
        }
    }
}
