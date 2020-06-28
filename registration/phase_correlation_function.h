#ifndef PHASE_CORRELATION_FUNCTIONS_H_INCLUDED
#define PHASE_CORRELATION_FUNCTIONS_H_INCLUDED
#include <opencv2/opencv.hpp>

/**
 * @file phase_correlation_function.h
 * The file contains vital phase correlation function wrappers. These function wrappers are calling the cv::PhaseCorrelate
 * function directly and process the results automatically for another use, if necessary.
 */

/**
 * @brief It calculates the shift between referencial and shifted frame.
 * Hann window is used in this calculation.
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @return The coordinates of the shift in x and y axis. z-value represents the value of the phase correlation peak.
 */
cv::Point3d pc_translation_hann(const cv::Mat& i_referencialFrame,
                                const cv::Mat& i_shiftedFrame,
                                double i_calcAreaSize);

/**
 * @brief It calculates the shift between referencial and shifted frame.
 * Hann window is not used.
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @return The coordinates of the shift in x and y axis. z-value represents the value of the phase correlation peak.
 */
cv::Point3d pc_translation(const cv::Mat& i_referencialFrame,
                           const cv::Mat& i_shiftedFrame,
                           double i_calcAreaSize);

/**
 * @brief It calculates the angle the shifted frame must be rotated to get best translation
 * results.
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @param i_maximalAngle
 * @param i_fkTranslation_maximumValue
 * @param i_translation
 * @return y-value contains calculated angle.
 */
cv::Point3d pc_rotation(const cv::Mat& i_referencialFrame,
                      const cv::Mat& i_shiftedFrame,
                      double i_maximalAngle,
                      const double& i_fkTranslation_maximumValue,
                      cv::Point3d& i_translation);
#endif // PHASE_CORRELATION_FUNCTIONS_H_INCLUDED
