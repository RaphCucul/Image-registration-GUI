#ifndef IMAGE_PROCESSING_H_INCLUDED
#define IMAGE_PROCESSING_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <QPoint>
#include <QRect>
#include <QString>

/**
 * @file image_processing.h
 * Functions vital for the post-processing of phase correlation results - translation, rotation, type conversion.
 */

/**
 * @brief Shifts the original frame to new coordinates.
 * @param[in] i_orig - original frame
 * @param[in] i_shift - calculated translation
 * @param[in] i_rows
 * @param[in] i_cols
 * @return translated frame
 */
cv::Mat frameTranslation(const cv::Mat& i_orig,
                         const cv::Point3d& i_shift,int i_rows,int i_cols);

/**
 * @brief Rotates shifted frame.
 * @param[in] i_frameAfterTranslation
 * @param[in] i_angle - calculated rotation
 * @return rotated frame
 */
cv::Mat frameRotation(const cv::Mat& i_frameAfterTranslation, const double i_angle);

/**
 * @brief Transforms given cv::Mat frame to 32C1 format.
 * @param[in] i_MatToCheck
 */
void transformMatTypeTo32C1(cv::Mat& i_MatToCheck);

/**
 * @brief Transforms given cv::Mat frame to 32C3 format.
 * @param[in] i_MatToCheck
 */
cv::Mat convertMatFrom32FTo8U(cv::Mat& i_MatToCheck);

/**
 * @brief Transforms given cv::Mat frame to 8C1 format.
 * @param[in] i_MatToCheck
 */
void transformMatTypeTo8C1(cv::Mat& i_MatToCheck);

/**
 * @brief Transforms given cv::Mat frame to 8C3 format.
 * @param[in] i_MatToCheck
 */
void transformMatTypeTo8C3(cv::Mat& i_MatToCheck);

/**
 * @brief Transforms given cv::Mat frame to 64C1 format.
 * @param[in] i_MatToCheck
 */
cv::Mat transformMatTypeTo64C1(cv::Mat& i_MatToCheck);

/**
 * @brief Converts numeric representation of the cv::Mat type into the
 * clear string representation.
 * @param[in] type
 */
QString MatType2String(int type);

/**
 * @brief Transforms given cv::Rect object to QRect object.
 * @param[in] i_rect
 */
QRect transform_CV_RECT_to_QRect(cv::Rect i_rect);

/**
 * @brief Transforms given QRect object to cv::Rect object.
 * @param[in] i_rect
 */
cv::Rect transform_QRect_to_CV_RECT(QRect i_rect);

/**
 * @brief Transforms given cv::Point object to QPoint object.
 * @param[in] i_point
 */
QPoint transform_CV_POINT_to_QPoint(cv::Point i_point);

/**
 * @brief Transforms given cv::Point3d object to QPointF object.
 * @param[in] i_point
 */
QPointF transform_CV_POINTF_to_QPointF(cv::Point3d i_point);

/**
 * @brief Transforms given QPoint object to cv::Point3d object.
 * @param[in] i_point
 */
cv::Point3d transform_QPoint_to_CV_Point3d(QPoint i_point);
#endif // IMAGE_PROCESSING_H_INCLUDED
