#ifndef MULTIPOC_AI1_H_INCLUDED
#define MULTIPOC_AI1_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QCheckBox>
#include <QObject>

/**
 * @file multiPOC_Ai1.h
 * The file contains a declaration of the main registration function and a few helper functions used for the frame
 * preprocessing and cutout calculations.
 */

/**
 * @brief The main function performing the registration of two frames - referencial and another one.
 * @param cap
 * @param i_referencial
 * @param i_shifted
 * @param i_translatedNo
 * @param i_iteration
 * @param i_areaMaximum
 * @param i_angleLimit
 * @param i_cutoutExtra
 * @param i_cutoutStandard
 * @param i_scaleChanged
 * @param i_completelyRegistrated
 * @param i_pocX
 * @param i_pocY
 * @param i_maxAngles
 * @param video
 * @return False if registration fails, otherwise true.
 */
bool completeRegistration(cv::VideoCapture& cap,
                          cv::Mat& i_referencial,
                          cv::Mat& i_shifted,
                          int i_translatedNo,
                          double i_iteration,
                          double i_areaMaximum,
                          double i_angleLimit,
                          cv::Rect& i_cutoutExtra,
                          cv::Rect& i_cutoutStandard,
                          int i_scaleChanged,
                          cv::Mat& i_completelyRegistrated,
                          QVector<double> &i_pocX,
                          QVector<double> &i_pocY,
                          QVector<double> &i_maxAngles,
                          bool video = true);

/**
 * @brief It preprocesses the input frame before main registration function is called. Populates cv::Rect objects
 * according to the changes/selections made by a user. If the change in extra cutout was made, it is necessary to recalculate
 * standard cutout, if this parameter was modified too.
 * @param i_referencial
 * @param i_frangi_point
 * @param i_standardCutout
 * @param i_frangiParameters
 * @param frangiRatios
 * @param frangiMargins
 * @return False if preprocessing fails, otherwise true.
 */
bool preprocessingCompleteRegistration(cv::Mat &i_referencial,
                                       cv::Point3d& i_frangi_point,
                                       cv::Rect &i_standardCutout,
                                       QMap<QString, double> i_frangiParameters,
                                       QMap<QString,double> frangiRatios,
                                       QMap<QString,int> frangiMargins);

/**
 * @brief It calculates the standard cutout using maximum frangi coordinates.
 * @param i_frangi
 * @param i_ratios
 * @param i_rows
 * @param i_cols
 * @return Calculated standard cutout cv::Rect object.
 */
cv::Rect calculateStandardCutout(cv::Point3d i_frangi, QMap<QString, double> i_ratios, int i_rows,
                                 int i_cols);

/**
 * @brief It recalculates the standard cutout from the extra cutout.
 * @param i_extraCutoutParameters
 * @param i_originalStandardCutout
 * @param i_rows
 * @param i_cols
 * @return Recalculated standard cutout.
 */
cv::Rect adjustStandardCutout(cv::Rect i_extraCutoutParameters, cv::Rect i_originalStandardCutout,
                              int i_rows, int i_cols);
#endif // MULTIPOC_AI1_H_INCLUDED
