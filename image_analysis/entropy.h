#ifndef ENTROPY_H_INCLUDED
#define ENTROPY_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "image_analysis/image_processing.h"
#include <QProgressBar>
#include <QVector>

/**
 * @file entropy.h
 * The file contains declarations of functions calculating entropy and tennengrad parameter of a frame.
 */

/**
 * @brief It calculates entropy and tennengrad parameter of the given frame.
 * @param[in] i_specificImage
 * @param[out] i_entropy
 * @param[out] i_tennengrad
 * @return True if the calculation ended successfully, otherwise false.
 */
bool calculateParametersET(cv::Mat &i_specificImage, double &i_entropy, cv::Scalar &i_tennengrad);

/**
 * @brief It wraps the calcutaion of entropy and tennengrad parameter of the video.
 * @param[in] capture - video for analysis
 * @param[out] entropy
 * @param[out] tennengrad
 * @return True if the calculation ended successfully, otherwise false.
 */
bool entropy_tennengrad_video(cv::VideoCapture& i_capture,
                              QVector<double>& i_entropy,
                              QVector<double>& i_tennengrad);

/**
 * @brief It calculates the sum of the histogram.
 * @param[in] inputImage - histogram
 * @param[in] histogramSize
 * @return The sum of the histogram.
 */
double binFrequency(cv::Mat &inputImage,int &histogramSize);

/**
 * @brief It standardised input data. Maximum and minimum value must be provided.
 * @param[out] i_dataStandardized
 * @param[in] i_dataOriginal
 * @param[in] i_minimum
 * @param[in] i_maximum
 */
void dataStandardization(QVector<double>& i_dataStandardized,QVector<double>& i_dataOriginal,
                             double i_minimum,double i_maximum);

#endif // ENTROPY_H_INCLUDED
