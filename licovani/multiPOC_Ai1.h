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
                        cv::Mat& referencialImage,
                        int numberTranslated,
                        double iteration,
                        double areaMaximum,
                        double angle,
                        cv::Rect& cutoutExtra,
                        cv::Rect& cutoutStandard,
                        bool scaleChange,
                        cv::Mat& fullyRegistratedImage,
                        cv::Point3d& definitiveTranslation,
                        double& finalRotationAngle);

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
bool preprocessingCompleteRegistration(cv::Mat &reference,
                                       cv::Mat &image,
                                       QVector<double> parFrang,
                                       cv::Point2d verticalAnomaly,
                                       cv::Point2d horizontalAnomaly,
                                       cv::Point3d &fraMax,
                                       cv::Rect &anomalyArea,
                                       cv::Rect &cutoutStandard,
                                       cv::Rect &cutoutExtra,
                                       cv::VideoCapture &cap,
                                       bool& anomalyPresence,
                                       bool &scalingNeeded);
#endif // MULTIPOC_AI1_H_INCLUDED
