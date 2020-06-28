#ifndef FRANGI_UTILIZATION_H_INCLUDED
#define FRANGI_UTILIZATION_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QString>
#include "frangi.h"
#include "image_analysis/image_processing.h"
#include <QVector>
#include <QJsonObject>
#include <QObject>
#include <QMap>

/**
 * @file frangi_utilization.h
 * The file contains declarations of functions using functions declared in frangi.h
 * to calculate maximum frangi coordinates.
 */

/**
 * @brief Recursive and median filtration is applied on a frame.
 * @param inputImage
 * @param sigma_s
 * @param sigma_r
 * @return Filtered frame.
 */
cv::Mat imageFiltrationPreprocessing(const cv::Mat &i_inputImage, float i_sigma_s, float i_sigma_r);

/**
 * @brief It modifies margins of a frame to eliminate light artefacts. The chance frangi filter will be analysing
 * these artefacts is reduced.
 * @param i_inputImage
 * @param i_imageType
 * @param i_padding_top
 * @param i_padding_bottom
 * @param i_padding_left
 * @param i_padding_right
 */
void borderProcessing(cv::Mat &i_inputImage,int i_imageType, int i_padding_top,int i_padding_bottom,
                      int i_padding_left, int i_padding_right);

/**
 * @brief It sets a zero value for border pixels of the frame. The size of the border can be changed by a user.
 * @param i_inputImage
 * @param i_imageType
 * @param i_padding_top
 * @param i_padding_bottom
 * @param i_padding_left
 * @param i_padding_right
 */
void zeroBorders(cv::Mat &i_inputImage,int i_imageType, int i_padding_top,int i_padding_bottom,
                 int i_padding_left, int i_padding_right);

/**
 * @brief It calculates frangi maximum coordinates. The frame type indicates, if the maximum is calculated from
 * registrated or referential frame. Registrated frame is supposed to have black margins. The precision
 * of the calculation can be pixel or subpixel. If you choose to show the calculation result,
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
 * @return Calculated maximum frangi coordinates.
 */
cv::Point3d frangi_analysis(const cv::Mat i_inputFrame,
                            int i_processingMode,
                            int i_accuracy,
                            int i_showResult,
                            QString i_windowName,
                            int i_frameType,
                            cv::Point3d i_translation,
                            QMap<QString, double> i_FrangiParameters,
                            QMap<QString, int> i_margins);

/**
 * @brief It computes Frangi maximum coordinates with subpixel precision.
 * @param frangi
 * @param maximum_frangi
 * @param maximumFrangiCoords
 * @return Recalculated frangi corrdinates with subpixel precision.
 */
cv::Point2d FrangiSubpixel(const cv::Mat &i_frangi,
                           const double& i_maximum_frangi,
                           const cv::Point& i_maximumFrangiCoords);
#endif // FRANGI_UTILIZATION_H_INCLUDED
