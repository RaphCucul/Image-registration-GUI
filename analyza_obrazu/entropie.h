#ifndef ENTROPIE_H_INCLUDED
#define ENTROPIE_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "analyza_obrazu/upravy_obrazu.h"
#include <QProgressBar>
#include <QVector>

/**
 * @brief Function calculates entropy and tennengrad of the given frame.
 * @param i_specificImage
 * @param i_entropy
 * @param i_tennengrad
 * @return
 */
bool calculateParametersET(cv::Mat &i_specificImage, double &i_entropy, cv::Scalar &i_tennengrad);

/**
 * @brief Function wraps calcutaion of entropy and tennengrad of the video
 * @param capture
 * @param entropy
 * @param tennengrad
 * @param progbar
 * @return
 */
bool entropy_tennengrad_video(cv::VideoCapture& i_capture,
                              QVector<double>& i_entropy,
                              QVector<double>& i_tennengrad);

double binFrequency(cv::Mat &inputImage,int &histogramSize);

/**
 * @brief To plot entropy and tennengrad values to the same graph, it is necessary to standardize the data.
 * @param i_dataStandardized
 * @param i_dataOriginal
 * @param i_minimum
 * @param i_maximum
 */
void dataStandardization(QVector<double>& i_dataStandardized,QVector<double>& i_dataOriginal,
                             double i_minimum,double i_maximum);

#endif // ENTROPIE_H_INCLUDED
