#ifndef ROZHODOVACI_ALGORITMY_H_INCLUDED
#define ROZHODOVACI_ALGORITMY_H_INCLUDED
#include <QVector>
#include <opencv2/opencv.hpp>
/**
 * @brief First decision algorithm uses average corelation coefficient and average FWHM value obtained from
 * frames having entropy and tennengrad values in the correct range. All theresholds in this algorithm were
 * found empirically based on the testing.
 * @param badFrames_FirstEval
 * @param allFrameCompleteEval
 * @param POC_x
 * @param POC_y
 * @param Angle
 * @param frangi_x
 * @param frangi_y
 * @param frangi_euklid
 * @param averageCC
 * @param averageFWHM
 * @param cap
 * @param referImage
 * @param COStandard
 * @param COExtra
 * @param scaleChanged
 * @param framesFirstEval
 * @param computedCC
 * @param computedFWHM
 */
void firstDecisionAlgorithm(QVector<double>& badFrames_FirstEval,
                            QVector<double>& allFrameCompleteEval,
                            QVector<double>& POC_x,
                            QVector<double>& POC_y,
                            QVector<double>& Angle,
                            QVector<double>& frangi_x,
                            QVector<double>& frangi_y,
                            QVector<double>& frangi_euklid,
                            double averageCC,
                            double averageFWHM,
                            cv::VideoCapture& cap,
                            cv::Mat& referImage,
                            cv::Rect& COStandard,
                            cv::Rect& COExtra,
                            bool scaleChanged,
                            QVector<double>& framesFirstEval,
                            QVector<double>& computedCC,
                            QVector<double>& computedFWHM);

/**
 * @brief Second decision algorithm takes frames marked as not good for registration and checks if the first
 * decision algorithm made adequate decision. Average values of correlation coefficient and FWHM are used again,
 * but decision rules are not so restrictive and some frames can be send to the third algorithm for definitive
 * evaluation.
 * @param framesFirstEval
 * @param allFrameCompleteEval
 * @param computedCC
 * @param computedFWHM
 * @param POC_x
 * @param POC_y
 * @param Angle
 * @param frangi_x
 * @param frangi_y
 * @param frangi_euklid
 * @param averageCC
 * @param averageFWHM
 * @param framesSecondEval
 */
void secondDecisionAlgorithm(QVector<double>& framesFirstEval,
                             QVector<double>& allFrameCompleteEval,
                             QVector<double>& computedCC,
                             QVector<double>& computedFWHM,
                             QVector<double>& POC_x,
                             QVector<double>& POC_y,
                             QVector<double>& Angle,
                             QVector<double>& frangi_x,
                             QVector<double>& frangi_y,
                             QVector<double>& frangi_euklid,
                             double averageCC,
                             double averageFWHM,
                             QVector<double>& framesSecondEval);


void thirdDecisionAlgorithm(cv::Mat& image,
                            cv::Rect& coutouExtra,
                            cv::Rect& cutoutStandard,
                            QVector<double>& frangi_x,
                            QVector<double>& frangi_y,
                            QVector<double>& frangi_euklid,
                            QVector<double>& POC_x,
                            QVector<double>& POC_y,
                            QVector<double>& Angle,
                            bool scaleChanged,
                            cv::VideoCapture& cap,
                            QVector<double>& allFrameCompleteEval,
                            QVector<double>& framesSecondEval,
                            QVector<double>& FrangiParameters);

#endif // ROZHODOVACI_ALGORITMY_H_INCLUDED
