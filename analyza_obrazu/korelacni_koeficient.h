#ifndef KORELACNI_KOEFICIENT_H_INCLUDED
#define KORELACNI_KOEFICIENT_H_INCLUDED
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
/**
 * @brief Function calculates the correlation coefficient of referencial and registrated frame.
 * @param i_referencial
 * @param i_registrated
 * @param i_cutoutCorrelation
 * @return
 */
double calculateCorrCoef(const cv::Mat& i_referencial, const cv::Mat& i_registrated,cv::Rect i_cutoutCorrelation);
#endif // KORELACNI_KOEFICIENT_H_INCLUDED
