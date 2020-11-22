#ifndef QTHREADTHIRDPART_H
#define QTHREADTHIRDPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <opencv2/opencv.hpp>

#include "shared_staff/sharedvariables.h"
using namespace clickImageEnums;

/**
 * @class qThreadThirdPart
 * @brief The qThreadThirdPart class contains algorithm evaluating frames using FWHM and correlation coefficient.
 *
 * All conditions used to evaluate frames were found empirically.
 */
class qThreadThirdPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadThirdPart(QStringList i_videoList,
                              QVector<QString> i_badVideos,
                              QMap<QString,QVector<int>> i_badFramesFirstEval,
                              QMap<QString,QVector<int>> i_framesCompleteEval,
                              QMap<QString,int> i_framesreferential,
                              QMap<QString,double> i_averageCC,
                              QMap<QString,double> i_averageFWHM,
                              QMap<QString,cv::Rect> i_standardCutout,
                              QMap<QString,cv::Rect> i_extraCutout,
                              cutoutType i_cutoutType,
                              double i_areaMaximum,
                              QObject *parent=nullptr);
    /**
     * @brief Main class function initiating all calculations.
     */
    void run() override;
    /**
     * @brief Returns updated evaluation indexes of videoframes.
     */
    QMap<QString,QVector<int>> framesUpdateEvaluation();
    /**
     * @brief Returns evaluation indexes changed in this thread.
     */
    QMap<QString,QVector<int>> framesFirstEvaluationComplete();
    /**
     * @brief Returns calculated correlation coefficients of videoframes.
     */
    QMap<QString, QVector<double> > frames_CC();
    /**
     * @brief Returns calculated FWHM coefficient.
     */
    QMap<QString,QVector<double>> frames_FWHM();
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
    QMap<QString,QVector<double>> framesAngleEstimated();
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

    QStringList videoList;
    QMap<QString,double> averageCCcomplete, averageFWHMcomplete;
    QMap<QString,QVector<int>> badFrames_firstEvaluation, framesEvaluationCompelte, framesFirstCompleteEvaluation;
    QMap<QString,QVector<double>> framesComputedCC,framesComputedFWHM;
    QMap<QString,int> referentialFrames;
    QVector<QString> notProcessThese;
    QMap<QString,cv::Rect> obtainedCutoffStandard, obtainedCutoffExtra;
    cutoutType selectedCutout = cutoutType::STANDARD;
    QMap<QString,QVector<double>> framesFrangiX;
    QMap<QString,QVector<double>> framesFrangiY;
    QMap<QString,QVector<double>> framesFrangiEuklid;
    QMap<QString,QVector<double>> framesPOCX;
    QMap<QString,QVector<double>> framesPOCY;
    QMap<QString,QVector<double>> framesUhel;
    double videoCount,areaMaximum;
};

#endif // QTHREADTHIRDPART_H
