#ifndef MULTIPOC_AI1_H_INCLUDED
#define MULTIPOC_AI1_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QCheckBox>
#include <QObject>

/**
 * @brief The main function performing the registration of two frames - referencial and another one.
 * @param cap
 * @param referencialImage
 * @param numberTranslated
 * @param iteration
 * @param areaMaximum
 * @param angle
 * @param cutoutExtra
 * @param cutoutStandard
 * @param scaleChange
 * @param fullyRegistratedImage
 * @param definitiveTranslation
 * @param finalRotationAngle
 * @return
 */
bool completeRegistration(cv::VideoCapture& cap,
                          cv::Mat& i_referencial,
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
                          QVector<double> &i_maxAngles);

/**
 * @brief Function does preprocessing before main registration function is called. It populates cv::Rect objects
 * according to the userly made changes. If the change in extra cutout was made, it is necessary to recalculate
 * standard cutout, if this parameter was to modified too.
 * @param reference
 * @param image
 * @param parFrang
 * @param i_standardCutout
 * @param i_extraCutout
 * @param fraMax
 * @param anomalyArea
 * @param cutoutStandard
 * @param cutoutExtra
 * @param cap
 * @param anomalyPresence
 * @param scalingNeeded
 */
bool preprocessingCompleteRegistration(cv::Mat &i_referencial,
                                       cv::Point3d& i_frangi_point,
                                       cv::Rect &i_standardCutout,
                                       QVector<double> i_frangiParameters,
                                       QMap<QString,double> frangiRatios,
                                       QMap<QString,int> frangiMargins);

cv::Rect calculateStandardCutout(cv::Point3d i_frangi, QMap<QString, double> i_ratios, int i_rows,
                                 int i_cols);
cv::Rect adjustStandardCutout(cv::Rect i_extraCutoutParameters, cv::Rect i_originalStandardCutout,
                              int i_rows, int i_cols);
#endif // MULTIPOC_AI1_H_INCLUDED
