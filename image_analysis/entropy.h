#ifndef ENTROPY_H_INCLUDED
#define ENTROPY_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "image_analysis/image_processing.h"
#include <QProgressBar>
#include <QVector>

/**
 * @file entropy.h
 * Functions calculating entropy and tennengrad parameter of a frame.
 */

/**
 * @brief Calculates entropy and tennengrad parameter of a frame.
 * @param[in] i_specificImage - input frame
 * @param[out] i_entropy - calculated entropy
 * @param[out] i_tennengrad - calculated tennengrad
 * @return True if the calculation ends successfully, otherwise false.
 */
bool calculateParametersET(cv::Mat &i_specificImage, double &i_entropy, cv::Scalar &i_tennengrad);

/**
 * @brief Wraps the calcutaion of an entropy and tennengrad parameter of a video.
 * @param[in] capture - video for analysis
 * @param[out] entropy - vector for entropy of all video frames
 * @param[out] tennengrad - vector for tennengrad of all video frames
 * @return True if the calculation ended successfully, otherwise false.
 */
bool entropy_tennengrad_video(cv::VideoCapture& i_capture,
                              QVector<double>& i_entropy,
                              QVector<double>& i_tennengrad);

/**
 * @brief Calculates the sum of the histogram.
 * @param[in] inputImage - histogram
 * @param[in] histogramSize
 * @return The sum of the histogram.
 */
double binFrequency(cv::Mat &inputImage,int &histogramSize);

/**
 * @brief Standardised input data. Maximum and minimum value must be provided.
 * @param[out] i_dataStandardized
 * @param[in] i_dataOriginal
 * @param[in] i_minimum
 * @param[in] i_maximum
 */
void dataStandardization(QVector<double>& i_dataStandardized,QVector<double>& i_dataOriginal,
                             double i_minimum,double i_maximum);

#endif // ENTROPY_H_INCLUDED
