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
void firstDecisionAlgorithm(QVector<double>& i_badFrames_FirstEval,
                            QVector<double>& i_allFrameCompleteEval,
                            QVector<double>& i_POC_x,
                            QVector<double>& i_POC_y,
                            QVector<double>& i_Angle,
                            QVector<double>& i_frangi_x,
                            QVector<double>& i_frangi_y,
                            QVector<double>& i_frangi_euklid,
                            double i_averageCC,
                            double i_averageFWHM,
                            cv::VideoCapture& i_cap,
                            cv::Mat& i_referImage,
                            cv::Rect& i_COStandard,
                            cv::Rect& i_COExtra,
                            bool i_scaleChanged,
                            QVector<double>& i_framesFirstEval,
                            QVector<double>& i_computedCC,
                            QVector<double>& i_computedFWHM);

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
void secondDecisionAlgorithm(QVector<double>& i_framesFirstEval,
                             QVector<double>& i_allFrameCompleteEval,
                             QVector<double>& i_computedCC,
                             QVector<double>& vcomputedFWHM,
                             QVector<double>& i_POC_x,
                             QVector<double>& i_POC_y,
                             QVector<double>& i_Angle,
                             QVector<double>& i_frangi_x,
                             QVector<double>& i_frangi_y,
                             QVector<double>& i_frangi_euklid,
                             double i_averageCC,
                             double i_averageFWHM,
                             QVector<double>& i_framesSecondEval);

/**
 * @brief Third decision algorithm registrates the problematic frame and then calculates
 * coordinates of Frangi filter maximum. The euclidean distance of Frangi filter maximum of this
 * shifted->registrated frame and referencial frame is the last criterium for frame evaluation.
 * @param image
 * @param coutouExtra
 * @param cutoutStandard
 * @param frangi_x
 * @param frangi_y
 * @param frangi_euklid
 * @param POC_x
 * @param POC_y
 * @param Angle
 * @param scaleChanged
 * @param cap
 * @param allFrameCompleteEval
 * @param framesSecondEval
 * @param FrangiParameters
 */
void thirdDecisionAlgorithm(cv::Mat& i_frame,
                            cv::Rect& i_coutouExtra,
                            cv::Rect& i_cutoutStandard,
                            QVector<double>& i_frangi_x,
                            QVector<double>& i_frangi_y,
                            QVector<double>& i_frangi_euklid,
                            QVector<double>& i_POC_x,
                            QVector<double>& i_POC_y,
                            QVector<double>& i_Angle,
                            bool i_scaleChanged,
                            cv::VideoCapture& i_cap,
                            QVector<double>& i_allFrameCompleteEval,
                            QVector<double>& i_framesSecondEval,
                            QVector<double>& i_FrangiParameters,
                            QMap<QString, int> i_margins);

#endif // ROZHODOVACI_ALGORITMY_H_INCLUDED
