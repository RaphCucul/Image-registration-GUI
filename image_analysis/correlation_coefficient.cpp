#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "image_analysis/correlation_coefficient.h"
#include "image_analysis/image_processing.h"
using namespace std;
using cv::Mat;

double calculateCorrCoef(const cv::Mat &i_referential, const cv::Mat &i_registrated, cv::Rect i_cutoutCorrelation)
{
    Mat referentialFrame_cutout,registratedFrame_cutout,referentialFrame_64,registratedFrame_64;
    i_referential.copyTo(referentialFrame_64);
    i_registrated.copyTo(registratedFrame_64);
    referentialFrame_64 = transformMatTypeTo64C1(referentialFrame_64);
    registratedFrame_64 = transformMatTypeTo64C1(registratedFrame_64);
    referentialFrame_64(i_cutoutCorrelation).copyTo(referentialFrame_cutout);
    registratedFrame_64(i_cutoutCorrelation).copyTo(registratedFrame_cutout);

    cv::Scalar average_referential,average_registrated;
    average_referential = cv::mean(referentialFrame_cutout);
    average_registrated = cv::mean(registratedFrame_cutout);
    subtract(referentialFrame_cutout,average_referential[0],referentialFrame_cutout);
    subtract(registratedFrame_cutout,average_registrated[0],registratedFrame_cutout);
    cv::Scalar pom1,pom2,pom3;
    double r;
    pom1 = cv::sum(referentialFrame_cutout.mul(registratedFrame_cutout));
    pom2 = cv::sum(referentialFrame_cutout.mul(referentialFrame_cutout));
    pom3 = cv::sum(registratedFrame_cutout.mul(registratedFrame_cutout));
    r = pom1[0]/std::sqrt(pom2[0]*pom3[0]);
    return r;
}
