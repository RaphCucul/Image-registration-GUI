#ifndef REGISTRATION_CORRECTION_H_INCLUDED
#define REGISTRATION_CORRECTION_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QVector>

/**
 * @file registration_correction.h
 */

/**
 * @brief It registrates the referential frame with a frame with the evaluatio index 0 which means
 * no error is expected.
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
                          cv::Mat &i_shiftedFrame,
                          cv::Point3d i_coordsFrangiStandardReferencialReverse,
                          int i_index_moved,
                          double i_iteration,
                          double i_areaMaximum,
                          double i_angle,
                          cv::Rect i_cutoutExtra,
                          cv::Rect i_cutoutStandard,
                          bool i_scaleChanged,
                          QMap<QString, double> i_frangiParameters,
                          QVector<double> &_pocX,
                          QVector<double> &_pocY,
                          QVector<double> &_frangiX,
                          QVector<double> &_frangiY,
                          QVector<double> &_frangiEucl,
                          QVector<double> &_maxAngles,
                          QMap<QString,int> i_margins,
                          bool video = true);
/**
 * @brief Depricated.
 * @param i_cap
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @param i_index_moved
 * @param i_scaleChanged
 * @param i_cutoutStandard
 * @param i_cutoutExtra
 * @param i_adjustedCutoutStandard
 * @param fullyRegistratedFrame
 * @param i_areaMaximum
 * @param i_coordsFrangiStandardReferencialReverse
 * @param _tempPOCX
 * @param _tempPOCY
 * @param _frangiX
 * @param _frangiY
 * @param _frangiEucl
 * @param _pocX
 * @param _pocY
 * @param i_frangiParameters
 * @param _tempAngles
 * @param _maxAngles
 * @param i_margins
 * @param video
 * @return
 */
bool postProcessing(cv::VideoCapture i_cap,
                    cv::Mat& i_referencialFrame,
                    cv::Mat &i_shiftedFrame,
                    int i_index_moved,
                    bool i_scaleChanged,
                    cv::Rect i_cutoutStandard,
                    cv::Rect i_cutoutExtra,
                    cv::Rect i_adjustedCutoutStandard,
                    cv::Mat& fullyRegistratedFrame,
                    double i_areaMaximum,
                    cv::Point3d i_coordsFrangiStandardReferencialReverse,
                    QVector<double>& _tempPOCX,
                    QVector<double>& _tempPOCY,
                    QVector<double> &_frangiX,
                    QVector<double> &_frangiY,
                    QVector<double> &_frangiEucl,
                    QVector<double> &_pocX,
                    QVector<double> &_pocY,
                    QMap<QString, double> i_frangiParameters,
                    QVector<double>& _tempAngles,
                    QVector<double> &_maxAngles,
                    QMap<QString,int> i_margins,
                    bool video = true);

/**
 * @brief It is possible that after full registration process a translation correction can be necessary. The
 * function tests the translation results and it can provide the correction of shift coordinates for
 * better registration result.
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
