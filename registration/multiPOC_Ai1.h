#ifndef MULTIPOC_AI1_H_INCLUDED
#define MULTIPOC_AI1_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QCheckBox>
#include <QObject>

/**
 * @file multiPOC_Ai1.h
 * Main registration function and a few helper functions used for the frame
 * preprocessing and cutout calculations.
 */

/**
 * @brief The main function performing the registration of two frames - referential and another one.
 * @param[in] cap
 * @param[in] i_referential
 * @param[in] i_shifted
 * @param[in] i_translatedNo
 * @param[in] i_iteration
 * @param[in] i_areaMaximum
 * @param[in] i_angleLimit
 * @param[in] i_cutoutExtra
 * @param[in] i_cutoutStandard
 * @param[in] i_scaleChanged
 * @param[in] i_completelyRegistrated
 * @param[out] i_pocX
 * @param[out] i_pocY
 * @param[out] i_maxAngles
 * @param[in] video
 * @return False if registration fails, otherwise true.
 */
bool completeRegistration(cv::VideoCapture cap,
                          cv::Mat &i_referential,
                          cv::Mat &i_shifted,
                          int i_translatedNo,
                          double i_iteration,
                          double i_areaMaximum,
                          double i_angleLimit,
                          cv::Rect& i_cutoutExtra,
                          cv::Rect& i_cutoutStandard,
                          bool i_scaleChanged,
                          cv::Mat& i_completelyRegistrated,
                          QVector<double> &i_pocX,
                          QVector<double> &i_pocY,
                          QVector<double> &i_maxAngles,
                          bool video = true);

/**
 * @brief Preprocesses the input frame before main registration function is called.
 *
 * Populates cv::Rect objects
 * according to the changes/selections made by a user. If the extra cutout was changed, it is necessary to recalculate
 * standard cutout, if this parameter was modified too.
 * @param[in] i_referential
 * @param[in] i_frangi_point
 * @param[in] i_standardCutout
 * @param[in] i_frangiParameters
 * @param[in] frangiRatios
 * @param[in] frangiMargins
 * @return False if preprocessing fails, otherwise true.
 */
bool preprocessingCompleteRegistration(cv::Mat &i_referential,
                                       cv::Point3d& i_frangi_point,
                                       cv::Rect &i_standardCutout,
                                       QMap<QString, double> i_frangiParameters,
                                       QMap<QString,double> frangiRatios,
                                       QMap<QString,int> frangiMargins);

/**
 * @brief Calculates the standard cutout, using maximum frangi coordinates and ratios.
 * @param[in] i_frangi - frangi coordinates
 * @param[in] i_ratios - selected ratios
 * @param[in] i_rows
 * @param[in] i_cols
 * @return Calculated standard cutout cv::Rect object.
 */
cv::Rect calculateStandardCutout(cv::Point3d i_frangi, QMap<QString, double> i_ratios, int i_rows,
                                 int i_cols);

/**
 * @brief Recalculates the standard cutout from the extra cutout.
 *
 * makeItSmaller variable indicates, if the standard cutout should be adjusted against extra cutout (it will be smaller)
 * or if it should be calculated against the coordinates origin (will be bigger)
 * @param[in] i_extraCutoutParameters
 * @param[in] i_originalStandardCutout
 * @param[in] i_rows
 * @param[in] i_cols
 * @param[in] makeItSmaller
 * @return Recalculated standard cutout.
 */
cv::Rect adjustStandardCutout(cv::Rect i_extraCutoutParameters, cv::Rect i_originalStandardCutout,
                              int i_rows, int i_cols,bool makeItSmaller=true);

/**
 * @brief Recalculates the standard cutout from the extra cutout.
 *
 * makeItSmaller variable indicates, if the standard cutout should be adjusted against extra cutout (it will be smaller)
 * or if it should be calculated against the coordinates origin (will be bigger)
 * @param[in] i_extraCutoutParameters
 * @param[in] i_originalStandardCutout
 * @param[in] i_rows
 * @param[in] i_cols
 * @param[in] makeItSmaller
 * @return Recalculated standard cutout.
 */
QRect adjustStandardCutout(QRect i_extraCutoutParameters, QRect i_originalStandardCutout,
                              int i_rows, int i_cols,bool makeItSmaller=true);

#endif // MULTIPOC_AI1_H_INCLUDED
