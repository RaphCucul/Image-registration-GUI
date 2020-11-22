#ifndef QTHREADFIRSTPART_H
#define QTHREADFIRSTPART_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>
#include <QMap>

#include "shared_staff/sharedvariables.h"
using namespace clickImageEnums;

/**
 * @class qThreadFirstPart
 * @brief The qThreadFirstPart class contains functions to calculate entropy and tennengrad values for each frame of a video.
 *
 * During the calculation, thresholds for entropy and tennengrad are also estimated. But, it is also possible to use previously
 * calculated thresholds. Extra and standard cutouts are also used.
 */
class qThreadFirstPart : public QThread
{
    Q_OBJECT
public:
   explicit qThreadFirstPart(QStringList i_videosForAnalysis,
                             cutoutType i_cutoutType,
                             QMap<QString, QVector<double>> ETthresholds,
                             QMap<QString, bool> ETthresholdsFound,
                             bool i_previousThresholdsUsageAllowed,
                             QObject* parent=nullptr);
    /**
     * @brief Returns calculated entropy values.
     */
    QMap<QString, QVector<double> > computedEntropy();
    /**
     * @brief Returns calculated tennengrad values.
     */
    QMap<QString, QVector<double> > computedTennengrad();
    /**
     * @brief Returns frames marked as unsuitable for registration process, based on calculated entropy values.
     */
    QMap<QString, QVector<int> > computedFirstEntropyEvaluation();
    /**
     * @brief Returns frames marked as unsuitable for registration process, based on calculated tennengrad values.
     */
    QMap<QString, QVector<int> > computedFirstTennengradEvalueation();
    /**
     * @brief Returns evaluations of all videoframes.
     */
    QMap<QString, QVector<int> > computedCompleteEvaluation();
    /**
     * @brief Returns frame indexes of all bad frames (eval index 1, 4 and 5)
     */
    QMap<QString, QVector<int> > computedBadFrames();
    /**
     * @brief Returns calculated standard cutouts of referential frame for each video.
     */
    QMap<QString,cv::Rect> computedCOstandard();
    /**
     * @brief Returns calculated extra cutouts of referential frame for each video.
     */
    QMap<QString, cv::Rect> computedCOextra();
    /**
     * @brief Returns computed thresholds (upper and lower) for entropy and tennengrad for each video.
     */
    QMap<QString,QVector<double>> computedThresholds();
    /**
     * @brief Returns indexes of detected referential frames for all videos.
     */
    QMap<QString,int> estimatedreferentialFrames();

    /**
     * @brief Returns coordinates of frangi maximum for each detected referential frame for each video.
     */
    QMap<QString,cv::Point3d> estimatedReferentialFrangiCoordinates() { return maximum_frangi; };

    /**
     * @brief Returns names of unprocessed videos. A video can become unprocessed, if an error occurs during loading of this video.
     */
    QVector<QString> unprocessableVideos();

    /**
     * @brief Main class function initiating all calculations.
     */
    void run() override;

private:
    /**
     * @brief If an error occurs, a video is marked as unprocessable.
     */
    void fillEmpty(QString i_videoName);

    QMap<QString,QVector<double>> entropyComplete,tennengradComplete, previousThresholds;
    QMap<QString,QVector<int>> framesFirstFullCompleteEntropyEvaluation;
    QMap<QString,QVector<int>> framesFirstFullCompleteTennengradEvaluation;
    QMap<QString,QVector<int>> framesFullCompleteDecision;
    QMap<QString,QVector<int>> badFramesComplete;
    QMap<QString,int> referentialFrames;
    QMap<QString, bool> videosWithThresholdsFound;
    //QVector<double> FrangiParameters;
    QVector<QString> doNotProcessThis;
    QStringList processVideos;
    int videoCount;
    double percent;
    QMap<QString,cv::Rect> obtainedCutoffStandard;
    QMap<QString,cv::Rect> obtainedCutoffExtra;
    QMap<QString,QVector<double>> calculatedETthresholds;
    bool usePreviousThresholds = false;
    cutoutType selectedCutout = cutoutType::STANDARD;

    QMap<QString,cv::Point3d> maximum_frangi;
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
};

#endif // QTHREADFIRSTPART_H
