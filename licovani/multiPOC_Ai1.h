#ifndef MULTIPOC_AI1_H_INCLUDED
#define MULTIPOC_AI1_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QCheckBox>

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
 * @brief Function does preprocessing before main registration function. It populates cutouts cv::Rect variables
 * according to the existing anomalies - if at least one anomaly exists, area of the image the final translation
 * will be computed from must be adjusted.
 * @param reference
 * @param image
 * @param parFrang
 * @param verticalAnomaly
 * @param horizontalAnomaly
 * @param fraMax
 * @param anomalyArea
 * @param cutoutStandard
 * @param cutoutExtra
 * @param cap
 * @param anomalyPresence
 * @param scalingNeeded
 */
bool preprocessingCompleteRegistration(cv::Mat &i_referencial,
                                       cv::Mat &i_newreferencial,
                                       QVector<double> i_frangiParameters,
                                       cv::Point2d i_verticalAnomalyCoords,
                                       cv::Point2d i_horizontalAnomalyCoords,
                                       cv::Rect &i_anomalyArea,
                                       cv::Rect &i_cutoutExtra,
                                       cv::Rect &i_cutoutStandard,
                                       cv::VideoCapture &i_cap,
                                       bool &i_scaleChange,
                                       QMap<QString,double> frangiRatios,
                                       QMap<QString,int> frangiMargins);
#endif // MULTIPOC_AI1_H_INCLUDED
