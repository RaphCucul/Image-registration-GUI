#ifndef QTHREADFOURTHPART_H
#define QTHREADFOURTHPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>
#include <QMap>

/**
 * @class qThreadFourthPart
 * @brief The qThreadFourthPart class contains
 */
class qThreadFourthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFourthPart(QStringList i_videos,
                               QVector<QString> i_badVideos,
                               QMap<QString,QVector<int>> i_framesFirstEvaluationDefinitive,
                               QMap<QString,QVector<int>> i_evalCompl,
                               QMap<QString,QVector<double>> i_CCproblematic,
                               QMap<QString,QVector<double>> i_FWHMproblematic,
                               QMap<QString,QVector<double>> i_POCX,
                               QMap<QString,QVector<double>> i_POCY,
                               QMap<QString,QVector<double>> i_Angles,
                               QMap<QString,QVector<double>> i_Fr_X,
                               QMap<QString,QVector<double>> i_Fr_Y,
                               QMap<QString,QVector<double>> i_Fr_Eu,
                               QMap<QString,double> i_averageCC,
                               QMap<QString,double> i_averageFWHM,
                               QObject* parent=nullptr);
    /**
     * @brief Main class function initiating all calculations.
     */
    void run() override;
    /**
     * @brief Returns evaluation indexes changed in this thread.
     */
    QMap<QString,QVector<int>> framesSecondEvaluation();
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
    QMap<QString,QVector<int>> framesFirstEvaluationComplete,framesSecondEvaluationComplete,framesCompleteEvaluation;
    QMap<QString,QVector<double>> computedCC,computedFWHM,POC_x,POC_y,angle,frangi_x,
    frangi_y,frangi_euklid;
    QVector<QString> notProcessThese;
    QMap<QString,double> averageCCcomplete,averageFWHMcomplete;
    double videoCount;
    double framesToAnalyse;
};

#endif // QTHREADFOURTHPART_H
