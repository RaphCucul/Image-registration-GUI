#ifndef ENTROPIE_H_INCLUDED
#define ENTROPIE_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "analyza_obrazu/upravy_obrazu.h"
#include <QProgressBar>
#include <QVector>

void calculateParametersET(cv::Mat &specificImage, double &entropy, cv::Scalar &tennengrad);

/**
 * @brief Function wraps calcutaion of entropy and tennengrad of the video
 * @param capture
 * @param entropy
 * @param tennengrad
 * @param progbar
 * @return
 */
bool entropy_tennengrad_video(cv::VideoCapture& capture, QVector<double>& entropy,
                              QVector<double>& tennengrad,QProgressBar* progbar);

double binFrequency(cv::Mat &inputImage,int &histogramSize);

void standardizedData(QVector<double>& dataStandardized,QVector<double>& dataOriginal,
                             double minimum,double maximum);


//void aktualizaceProgressBaru(QProgressBar* pb, int procento);

#endif // ENTROPIE_H_INCLUDED
