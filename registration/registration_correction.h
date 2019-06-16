#ifndef REGISTRATION_CORRECTION_H_INCLUDED
#define REGISTRATION_CORRECTION_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QVector>

/**
 * @brief Function registrates the frames which were marked as ideal frames for registration process,
 * where an error in the process of registration is not expected.
 * @param i_cap
 * @param i_referencialFrame
 * @param i_coordsFrangiStandardReferencialReverse
 * @param i_index_noved
 * @param i_iteration
 * @param i_areaMaximum
 * @param i_angle
 * @param i_cutoutExtra
 * @param i_cutoutStandard
 * @param i_scaleChanged
 * @param i_frangiParameters
 * @param _pocX
 * @param _pocY
 * @param _frangiX
 * @param _frangiY
 * @param _frangiEucl
 * @param _maxAngles
 * @return
 */
bool registrateBestFrames(cv::VideoCapture& i_cap,
                                    cv::Mat& i_referencialFrame,
                                    cv::Point3d i_coordsFrangiStandardReferencialReverse,
                                    int i_index_noved,
                                    double i_iteration,
                                    double i_areaMaximum,
                                    double i_angle,
                                    cv::Rect& i_cutoutExtra,
                                    cv::Rect& i_cutoutStandard,
                                    bool i_scaleChanged,
                                    QVector<double> i_frangiParameters,
                                    QVector<double> &_pocX,
                                    QVector<double> &_pocY,
                                    QVector<double> &_frangiX,
                                    QVector<double> &_frangiY,
                                    QVector<double> &_frangiEucl,
                                    QVector<double> &_maxAngles,
                                    QMap<QString,int> i_margins);

/**
 * @brief It is possible that after full registration process a translation correction can be necessary. This
 * function therefore tests the translation results and can provide the correction of shift coordinates for
 * better registration.
 * @param i_frameRegistrated
 * @param i_frame
 * @param i_frameCorrected
 * @param i_cutoutStandard
 * @param i_correction
 * @return
 */
bool translationCorrection(cv::Mat& i_frameRegistrated,
                                  cv::Mat& i_frame,
                                  cv::Mat& i_frameCorrected,
                                  cv::Rect& i_cutoutStandard,
                                  cv::Point3d& i_correction,
                                  double i_areaMaximum);

#endif // REGISTRATION_CORRECTION_H_INCLUDED
