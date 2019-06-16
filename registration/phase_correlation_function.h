#ifndef PHASE_CORRELATION_FUNCTIONS_H_INCLUDED
#define PHASE_CORRELATION_FUNCTIONS_H_INCLUDED
#include <opencv2/opencv.hpp>

/**
 * @brief Function calculates the shift between referencial and shifted frame.
 * Hann window is used in this calculation.
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @return
 */
cv::Point3d pc_translation_hann(const cv::Mat& i_referencialFrame,
                              const cv::Mat& i_shiftedFrame,
                              double i_calcAreaSize);

/**
 * @brief Function calculates the shift between referencial and shifted frame.
 * Hann window is not used.
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @return
 */
cv::Point3d pc_translation(const cv::Mat& i_referencialFrame,
                         const cv::Mat& i_shiftedFrame,
                         double i_calcAreaSize);

/**
 * @brief Function calculates the angle the shifted frame must be rotated for the best translation
 * results.
 * @param i_referencialFrame
 * @param i_shiftedFrame
 * @param i_maximalAngle
 * @param i_fkTranslation_maximumValue
 * @param i_translation
 * @return
 */
cv::Point3d pc_rotation(const cv::Mat& i_referencialFrame,
                      const cv::Mat& i_shiftedFrame,
                      double i_maximalAngle,
                      const double& i_fkTranslation_maximumValue,
                      cv::Point3d& i_translation);
#endif // PHASE_CORRELATION_FUNCTIONS_H_INCLUDED
