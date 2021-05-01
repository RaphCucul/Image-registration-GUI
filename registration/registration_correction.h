#ifndef REGISTRATION_CORRECTION_H_INCLUDED
#define REGISTRATION_CORRECTION_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QVector>

/**
 * @file registration_correction.h
 * Main registration function and translation correction function.
 */

/**
 * @brief Registrates the referential frame with a frame with the evaluatio index 0 which means
 * no error is expected.
 *
 * Vectors are used as designated output of the function. These results can be saved and they are vital
 * when creating a new video.
 * @param[in] i_cap - actual video
 * @param[in] i_referentialFrame - referential frame of the video
 * @param[in] i_coordsFrangiStandardReferencialReverse
 * @param[in] i_index_moved - actual frame, supposed to be shifted
 * @param[in] i_iteration - maximum iteration
 * @param[in] i_areaMaximum - maximum counted area
 * @param[in] i_angle - maximum angle
 * @param[in] i_cutoutExtra - extra cutout
 * @param[in] i_cutoutStandard - standard cutout
 * @param[in] i_scaleChanged - if extra cutout selected, scale is true
 * @param[in] i_frangiParameters - global or video specific parameters
 * @param[out] _pocX - POC X-coordinate of videoframes
 * @param[out] _pocY - POC y-coordinate of videoframes
 * @param[out] _frangiX - Frangi X-coordinate of videoframes
 * @param[out] _frangiY - Frangi Y-coordinate of videoframes
 * @param[out] _frangiEucl - euclidean distance of frangi maximum coordinates
 * @param[out] _maxAngles - detected rotation angles
 * @return true if successfully processed, otherwise false
 */
bool registrateBestFrames(cv::VideoCapture& i_cap,
                          cv::Mat& i_referentialFrame,
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
 * @brief It is possible that after full registration process a translation correction can be necessary. The
 * function tests translation results and it can provide the correction of shift coordinates for
 * better registration result.
 * @param i_frameRegistrated
 * @param i_frame
 * @param i_frameCorrected
 * @param i_cutoutStandard
 * @param i_correction
 * @param i_areaMaximum
 * @return
 */
bool translationCorrection(cv::Mat& i_frameRegistrated,
                           cv::Mat& i_frame,
                           cv::Mat& i_frameCorrected,
                           cv::Rect& i_cutoutStandard,
                           cv::Point3d& i_correction,
                           double i_areaMaximum);

#endif // REGISTRATION_CORRECTION_H_INCLUDED
