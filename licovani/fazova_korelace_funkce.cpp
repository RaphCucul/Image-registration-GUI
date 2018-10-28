#include "fazova_korelace_funkce.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
cv::Point3d fk_translace_hann(const Mat &referencni_snimek, const Mat &posunuty)
{
    Point3d vysledek_posunuti(0,0,0);
    Mat referencni_snimek32f,posunuty32f;
    referencni_snimek.copyTo(referencni_snimek32f);
    posunuty.copyTo(posunuty32f);
    kontrola_typu_snimku_32C1(referencni_snimek32f);
    kontrola_typu_snimku_32C1(posunuty32f);
    //int typ_reference = referencni_snimek32f.type();
    //int typ_posunuty = posunuty32f.type();
    //double response = 0;
    cv::Mat hann;
    cv::createHanningWindow(hann, referencni_snimek32f.size(), CV_32FC1);
    kontrola_typu_snimku_32C1(hann);
    vysledek_posunuti = cv::phaseCorrelate(referencni_snimek32f,posunuty32f,5,hann);

    hann.release();
    referencni_snimek32f.release();
    posunuty32f.release();
    return vysledek_posunuti;
    //cout << "Result type "<<result.type()<<endl;
}

cv::Point3d fk_translace(const Mat &referencni_snimek, const Mat &posunuty)
{
    Point3d vysledek_posunuti;
    Mat referencni_snimek32f,posunuty32f;
    referencni_snimek.copyTo(referencni_snimek32f);
    posunuty.copyTo(posunuty32f);
    kontrola_typu_snimku_32C1(referencni_snimek32f);
    kontrola_typu_snimku_32C1(posunuty32f);
    //int typ_reference = referencni_snimek32f.type();
    //int typ_posunuty = posunuty32f.type();
    vysledek_posunuti = cv::phaseCorrelate(referencni_snimek32f,posunuty32f);

    referencni_snimek32f.release();
    posunuty32f.release();
    return vysledek_posunuti;
}

cv::Point3d fk_rotace(const Mat &referencni_snimek,
                      const Mat &posunuty,
                      double maximalni_uhel,
                      const double& hodnota_maxima_fk_translace,
                      cv::Point3d& hodnoty_translace)
{
    cv::Point3d vysledek_fazove_korelace;
    cv::Mat referencni_snimek8U,referencni_snimek32f, posunuty8U;
    referencni_snimek.copyTo(referencni_snimek8U);
    referencni_snimek.copyTo(referencni_snimek32f);
    posunuty.copyTo(posunuty8U);
    kontrola_typu_snimku_8C1(referencni_snimek8U);
    kontrola_typu_snimku_8C1(posunuty8U);

    cv::Mat logImage1 = cv::Mat::zeros(referencni_snimek8U.size(), CV_8UC1);
    cv::Mat logImage2 = cv::Mat::zeros(posunuty8U.size(), CV_8UC1);
    double magnitude = 40;
    //std::cout << referencni_snimek8U.type()<<" "<<posunuty8U.type()<<std::endl;
    cv::logPolar(referencni_snimek8U, logImage1, cv::Point2f(referencni_snimek8U.cols / 2.0f, referencni_snimek8U.rows / 2.0f), magnitude, cv::INTER_LINEAR);
    cv::logPolar(posunuty8U, logImage2, cv::Point2f(posunuty8U.cols / 2.0f, posunuty8U.rows / 2.0f), magnitude, cv::INTER_LINEAR);
    logImage1.convertTo(logImage1, CV_32FC1);
    logImage2.convertTo(logImage2, CV_32FC1);

    cv::Mat hann;
    //double response = 0;
    cv::createHanningWindow(hann, referencni_snimek.size(), CV_32FC1);
    cv::Point3d vysledek_rotace;
    if (std::abs(hodnoty_translace.x) > 10.0 || std::abs(hodnoty_translace.y) > 10.0)
    {vysledek_rotace = cv::phaseCorrelate(logImage1, logImage2,1,hann);}
    else {vysledek_rotace = cv::phaseCorrelate(logImage1, logImage2,5,hann);}

    if (vysledek_rotace.y > maximalni_uhel)
    {
        vysledek_rotace.y = maximalni_uhel;
    }

    cv::Point2f src_center(referencni_snimek.cols/2.0F, referencni_snimek.rows/2.0F);
    cv::Mat M = cv::getRotationMatrix2D(src_center,vysledek_rotace.y,1);
    cv::Mat vysledny_snimek_rotace = cv::Mat::zeros(referencni_snimek.size(), CV_32FC1);
    cv::warpAffine(posunuty,vysledny_snimek_rotace,M,posunuty.size(),0,BORDER_REPLICATE);
    //std::cout << vysledny_snimek_rotace.type() <<" "<< referencni_snimek.type() << std::endl;
    kontrola_typu_snimku_32C1(vysledny_snimek_rotace);
    kontrola_typu_snimku_32C1(referencni_snimek32f);

    cv::Point3d vysledek_translace_po_rotaci = cv::phaseCorrelate(referencni_snimek32f,vysledny_snimek_rotace,10,hann);

    //if ((vysledek_translace_po_rotaci.z - hodnota_maxima_fk_translace)>=0.0001)
    if (vysledek_translace_po_rotaci.z > hodnota_maxima_fk_translace)
    {
        vysledek_fazove_korelace.y = vysledek_rotace.y*360/(logImage2.cols);
    }
    else if (vysledek_translace_po_rotaci.z <= hodnota_maxima_fk_translace)
    {
        vysledek_fazove_korelace.y = 0;
        //std::cout << "uhel zmenen na hodnotu 0" <<std::endl;
    }
    vysledek_fazove_korelace.x = exp(vysledek_rotace.x/magnitude);
    vysledek_fazove_korelace.z = vysledek_translace_po_rotaci.z;

    logImage1.release();
    logImage2.release();
    referencni_snimek8U.release();
    posunuty8U.release();
    referencni_snimek32f.release();
    //posunuty.release();
    vysledny_snimek_rotace.release();
    //hann.release();

    return vysledek_fazove_korelace;
}

