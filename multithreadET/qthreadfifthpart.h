#ifndef QTHREADFIFTHPART_H
#define QTHREADFIFTHPART_H

#include <QThread>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>
#include <QMap>

#include "shared_staff/sharedvariables.h"
using namespace clickImageEnums;

/**
 * @class qThreadFifthPart
 * @brief The qThreadFifthPart class contains complete frame registration algorithm.
 *
 * This class is used as a final step to evaluate those frames marked as unsuitable for the registration process.
 */
class qThreadFifthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFifthPart(QStringList i_videos,
                              QVector<QString> i_badVideos,
                              QMap<QString,cv::Rect> i_standardCutout,
                              QMap<QString,cv::Rect> i_extraCutout,
                              QMap<QString,QVector<double>> i_POCX,
                              QMap<QString,QVector<double>> i_POCY,
                              QMap<QString,QVector<double>> i_Angle,
                              QMap<QString,QVector<double>> i_Fr_X,
                              QMap<QString,QVector<double>> i_Fr_Y,
                              QMap<QString,QVector<double>> i_Fr_E,
                              cutoutType i_cutoutType,
                              QMap<QString,QVector<int>> i_EvaluationComplete,
                              QMap<QString,QVector<int>> i_frEvalSec,
                              QMap<QString,int> i_referFrames,                              
                              int i_iteration,
                              double i_areaMaximum,
                              double i_maximalAngle,                              
                              QObject* parent = nullptr);
    /**
     * @brief Main class function initiating all calculations.
     */
    void run() override;
    /**
     * @brief Returns updated evaluation indexes of videoframes.
     */
    QMap<QString,QVector<int>> framesUpdateEvaluationComplete();
    /**
     * @brief Returns a vector with frangi X coordinate (999) of those frames where an analysis problem/error occured
     * or a frame is not suitable for the registration. Otherwise, there is zero value.
     */
    QMap<QString,QVector<double>> framesFrangiXestimated();
    /**
     * @brief Returns a vector with frangi Y coordinates (999) of those frames where an analysis problem/error occured
     * or a frame is not suitable for the registration. Otherwise, there is zero value.
     */
    QMap<QString,QVector<double>> framesFrangiYestimated();
    /**
     * @brief Returns a vector with euklidean distance (999) of those frames where an analysis problem/error occured
     * or a frame is not suitable for the registration. Otherwise, there is zero value.
     */
    QMap<QString,QVector<double>> framesFrangiEuklidestimated();
    /**
     * @brief Returns a vector with X axis translation (999) of those frames where an analysis problem/error occured
     * or a frame is not suitable for the registration. Otherwise, there is zero value.
     */
    QMap<QString,QVector<double>> framesPOCXestimated();
    /**
     * @brief Returns a vector with Y axis translation (999) of those frames where an analysis problem/error occured
     * or a frame is not suitable for the registration. Otherwise, there is zero value.
     */
    QMap<QString,QVector<double>> framesPOCYestimated();
    /**
     * @brief Returns a vector with rotation angle (999) of those frames where an analysis problem/error occured
     * or a frame is not suitable for the registration. Otherwise, there is zero value.
     */
    QMap<QString,QVector<double>> framesAngleestimated();
    /**
     * @brief Returns names of unprocessed videos. A video can become unprocessed, if an error occurs during loading of this video.
     */
    QVector<QString> unprocessableVideos();
signals:
    /**
     * @brief Provides integer value for the progress bar in the main thread. Takes in consideration number of videos which were chosen
     * for the analysis.
     */
    void percentageCompleted(int);
    /**
     * @brief Informs about current algorithm process.
     */
    void typeOfMethod(int);
    /**
     * @brief Informs about currently terminated thread (index).
     */
    void done(int);
    /**
     * @brief Informs about a video (index) which is currently processed.
     */
    void actualVideo(int);
    /**
     * @brief If an error occurs, message is sent to the main thread to be displayed.
     */
    void unexpectedTermination(int,QString);
    /**
     * @brief Informs the main thread calculated data was downloaded and the thread can be destroyed.
     */
    void readyForFinish();
private slots:
    /**
     * @brief When calculated data is downloaded, fires corresponding signal to init deletion of the thread.
     */
    void onDataObtained();
private:
    /**
     * @brief If an error occurs, a video is marked as unprocessable.
     */
    void fillEmpty(QString i_videoName);

    double videoCount, framesToAnalyse, maximalAngle, areaMaximum;
    int iteration;
    QStringList videoList;
    QMap<QString,QVector<int>> framesSecondEval,framesCompleteEvaluation;
    QMap<QString,QVector<double>> POC_x,POC_y,angle,frangi_x,frangi_y,frangi_euklid;
    QVector<double> average_CC,average_FWHM;
    QMap<QString,int> referentialFrames;
    QVector<QString> notProcessThese;
    QMap<QString,cv::Rect> obtainedCutoffStandard, obtainedCutoffExtra;
    cutoutType selectedCutout = cutoutType::STANDARD;
    bool scaleChanged = false;
};

#endif // QTHREADFIFTHPART_H
