#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/upravy_obrazu.h"
using namespace std;
using cv::Mat;

double calculateCorrCoef(const cv::Mat &i_referencial, const cv::Mat &i_registrated, cv::Rect i_cutoutCorrelation)
{
    Mat referencialFrame_cutout,registratedFrame_cutout,referencialFrame_64,registratedFrame_64;
    i_referencial.copyTo(referencialFrame_64);
    i_registrated.copyTo(registratedFrame_64);
    referencialFrame_64 = transformMatTypeTo64C1(referencialFrame_64);
    registratedFrame_64 = transformMatTypeTo64C1(registratedFrame_64);
    referencialFrame_64(i_cutoutCorrelation).copyTo(referencialFrame_cutout);
    registratedFrame_64(i_cutoutCorrelation).copyTo(registratedFrame_cutout);
    //int typ_reference = referencialFrame_cutout.type();
    //int typ_slicovany = registratedFrame_cutout.type();
    cv::Scalar average_referencial,average_registrated;
    average_referencial = cv::mean(referencialFrame_cutout);
    average_registrated = cv::mean(registratedFrame_cutout);
    subtract(referencialFrame_cutout,average_referencial[0],referencialFrame_cutout);
    subtract(registratedFrame_cutout,average_registrated[0],registratedFrame_cutout);
    cv::Scalar pom1,pom2,pom3;
    double r;
    pom1 = cv::sum(referencialFrame_cutout.mul(registratedFrame_cutout));
    pom2 = cv::sum(referencialFrame_cutout.mul(referencialFrame_cutout));
    pom3 = cv::sum(registratedFrame_cutout.mul(registratedFrame_cutout));
    r = pom1[0]/std::sqrt(pom2[0]*pom3[0]);
    return r;
}
