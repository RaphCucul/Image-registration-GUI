#ifndef POUZIJ_FRANGIHO_H_INCLUDED
#define POUZIJ_FRANGIHO_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QString>
#include "frangi.h"
#include "upravy_obrazu.h"
#include <QVector>
#include <QJsonObject>

/**
 * @brief In function, recursive and median filtration is applied on the image
 * @param inputImage
 * @param sigma_s
 * @param sigma_r
 * @return
 */
cv::Mat imageFiltrationPreprocessing(const cv::Mat &inputImage, float sigma_s, float sigma_r);

/** Function eliminates the borders of the image to get rid of undesirable light artefacts etc. to minimize chance
 * frangi filter will be analysing these artefacts too
 * @brief borderProcessing
 * @param inputImage
 * @param imageType
 * @param padding_r
 * @param padding_s
 */
void borderProcessing(cv::Mat &inputImage,int imageType, int padding_r,int padding_s);

/**
 * @brief Function set zeros to specific pixels
 * @param vstupni_obraz
 * @param typ_snimku
 * @param velikost_okraje_r
 * @param velikost_okraje_s
 */
void zeroBorders(cv::Mat &inputImage,int imageType, int padding_r,int padding_s);

/**
 * @brief Function calculates frangi maximum coords. Frame type indicates, if the maximum is calculated from
 * the translated image or from referencial image. Translated image is supposed to have black corners. The accuracy
 * of the calculation can be pixel or subpixel. If you choose to show the calculation and detection result,
 * the name of the window can be set. The processing mode says, if the frangi filter should be standard or reverse.
 * @param inputFrame
 * @param processingMode
 * @param accuracy
 * @param showResult
 * @param windowName
 * @param frameType
 * @param translation
 * @param FrangiParameters
 * @return
 */
cv::Point3d frangi_analysis(const cv::Mat inputFrame,
                           int processingMode,
                           int accuracy,
                           int showResult,
                           QString windowName,
                           int frameType,
                           cv::Point3d translation,
                           QVector<double> FrangiParameters);

/**
 * @brief Computing subpixel accuracy of the Frangi maximum
 * @param frangi
 * @param maximum_frangi
 * @param maximumFrangiCoords
 * @return
 */
cv::Point2d FrangiSubpixel(const cv::Mat &frangi,
                                   const double& maximum_frangi,
                                   const cv::Point& maximumFrangiCoords);
#endif // POUZIJ_FRANGIHO_H_INCLUDED
