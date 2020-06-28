#ifndef CORRELATION_COEFFICIENT_H_INCLUDED
#define CORRELATION_COEFFICIENT_H_INCLUDED
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

/**
 * @file correlation_coefficient.h
 * The file contains a declaration of a function calculating a correlation coefficient of two frames.
 */

/**
 * @brief It calculates the correlation coefficient of referencial and registrated frame.
 * @param i_referential - a referential frame
 * @param i_registrated - a registrated frame
 * @param i_cutoutCorrelation - an area of the frame which will be used for the calculation of the CC
 * @return The value of the correlation coefficient.
 */
double calculateCorrCoef(const cv::Mat& i_referential, const cv::Mat& i_registrated,cv::Rect i_cutoutCorrelation);
#endif // CORRELATION_COEFFICIENT_H_INCLUDED
