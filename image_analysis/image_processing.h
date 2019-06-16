#ifndef IMAGE_PROCESSING_H_INCLUDED
#define IMAGE_PROCESSING_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
/**
 * @brief Function shifts the original frame in new coordinates.
 * @param i_shifted_orig
 * @param i_shift
 * @param i_rows
 * @param i_cols
 * @return
 */
cv::Mat frameTranslation(const cv::Mat& i_shifted_orig,
                         const cv::Point3d& i_shift,int i_rows,int i_cols);

/**
 * @brief Function rotates shifted frame.
 * @param i_frameAfterTranslation
 * @param i_angle
 * @return
 */
cv::Mat frameRotation(const cv::Mat& i_frameAfterTranslation, const double i_angle);

/**
 * @brief Displays given frame, in the window with defined name.
 * @param i_windowName
 * @param i_frameToShow
 */
void showMat(std::string i_windowName, cv::Mat i_frameToShow);

void transformMatTypeTo32C1(cv::Mat& i_MatToCheck);
void transformMatTypeTo8C1(cv::Mat& i_MatToCheck);
void transformMatTypeTo8C3(cv::Mat& i_MatToCheck);
cv::Mat transformMatTypeTo64C1(cv::Mat& i_MatToCheck);
#endif // IMAGE_PROCESSING_H_INCLUDED
