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
 * The file contains functions vital for the post-processing of phase correlation results - translation, rotation, type conversion.
 */

/**
 * @brief It shifts the original frame to new coordinates.
 * @param i_shifted_orig
 * @param i_shift
 * @param i_rows
 * @param i_cols
 * @return Registrated frame.
 */
cv::Mat frameTranslation(const cv::Mat& i_shifted_orig,
                         const cv::Point3d& i_shift,int i_rows,int i_cols);

/**
 * @brief It rotates shifted frame.
 * @param i_frameAfterTranslation
 * @param i_angle
 * @return Rotated frame.
 */
cv::Mat frameRotation(const cv::Mat& i_frameAfterTranslation, const double i_angle);

/**
 * @brief It displays given frame in the window with defined name.
 * @param i_windowName
 * @param i_frameToShow
 */
void showMat(std::string i_windowName, cv::Mat i_frameToShow);

/**
 * @brief It transforms given cv::Mat frame to 32C1 format.
 * @param i_MatToCheck
 */
void transformMatTypeTo32C1(cv::Mat& i_MatToCheck);

/**
 * @brief It transforms given cv::Mat frame to 32C3 format.
 * @param i_MatToCheck
 */
cv::Mat convertMatFrom32FTo8U(cv::Mat& i_MatToCheck);

/**
 * @brief It transforms given cv::Mat frame to 8C1 format.
 * @param i_MatToCheck
 */
void transformMatTypeTo8C1(cv::Mat& i_MatToCheck);

/**
 * @brief It transforms given cv::Mat frame to 8C3 format.
 * @param i_MatToCheck
 */
void transformMatTypeTo8C3(cv::Mat& i_MatToCheck);

/**
 * @brief It transforms given cv::Mat frame to 64C1 format.
 * @param i_MatToCheck
 */
cv::Mat transformMatTypeTo64C1(cv::Mat& i_MatToCheck);

/**
 * @brief MatType2String
 * @param type
 * @return
 */
QString MatType2String(int type);

/**
 * @brief It transforms given cv::Rect object to QRect object.
 * @param i_rect
 */
QRect transform_CV_RECT_to_QRect(cv::Rect i_rect);

/**
 * @brief It transforms given QRect object to cv::Rect object.
 * @param i_rect
 */
cv::Rect transform_QRect_to_CV_RECT(QRect i_rect);

/**
 * @brief It transforms given cv::Point object to QPoint object.
 * @param i_point
 */
QPoint transform_CV_POINT_to_QPoint(cv::Point i_point);

/**
 * @brief It transforms given cv::Point3d object to QPointF object.
 * @param i_point
 */
QPointF transform_CV_POINTF_to_QPointF(cv::Point3d i_point);

/**
 * @brief It transforms given QPoint object to cv::Point3d object.
 * @param i_point
 */
cv::Point3d transform_QPoint_to_CV_Point3d(QPoint i_point);
#endif // IMAGE_PROCESSING_H_INCLUDED
