#ifndef POUZIJ_FRANGIHO_H_INCLUDED
#define POUZIJ_FRANGIHO_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QString>
#include "frangi.h"
#include "upravy_obrazu.h"
#include <QVector>
#include <QJsonObject>

/**
 * @brief Recursive and median filtration are applied on the image.
 * @param inputImage
 * @param sigma_s
 * @param sigma_r
 * @return
 */
cv::Mat imageFiltrationPreprocessing(const cv::Mat &i_inputImage, float i_sigma_s, float i_sigma_r);

/** Function eliminates the borders of the image to get rid of undesirable light artefacts etc. to minimize chance
 * frangi filter will be analysing these artefacts too
 * @brief borderProcessing
 * @param inputImage
 * @param imageType
 * @param padding_r
 * @param padding_s
 */
void borderProcessing(cv::Mat &i_inputImage,int i_imageType, int i_padding_top,int i_padding_bottom,
                      int i_padding_left, int i_padding_right);

/**
 * @brief Function set zeros to specific pixels
 * @param inputImage
 * @param imageType
 * @param padding_r
 * @param padding_s
 */
void zeroBorders(cv::Mat &i_inputImage,int i_imageType, int i_padding_top,int i_padding_bottom,
                 int i_padding_left, int i_padding_right);

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
 * @param i_margins
 * @param i_ratios
 * @return
 */
cv::Point3d frangi_analysis(const cv::Mat i_inputFrame,
                            int i_processingMode,
                            int i_accuracy,
                            int i_showResult,
                            QString i_windowName,
                            int i_frameType,
                            cv::Point3d i_translation,
                            QVector<double> i_FrangiParameters,
                            QMap<QString, int> i_margins);

/**
 * @brief Computing subpixel accuracy of the Frangi maximum
 * @param frangi
 * @param maximum_frangi
 * @param maximumFrangiCoords
 * @return
 */
cv::Point2d FrangiSubpixel(const cv::Mat &i_frangi,
                                   const double& i_maximum_frangi,
                                   const cv::Point& i_maximumFrangiCoords);
#endif // POUZIJ_FRANGIHO_H_INCLUDED
