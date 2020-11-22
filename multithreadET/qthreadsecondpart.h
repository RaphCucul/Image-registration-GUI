#ifndef QTHREADSECONDPART_H
#define QTHREADSECONDPART_H

#include <QThread>
#include <QObject>
#include <QVector>
#include <opencv2/opencv.hpp>
#include <QStringList>
#include <QMap>

#include "shared_staff/sharedvariables.h"
using namespace clickImageEnums;

/**
 * @class qThreadSecondPart
 * @brief The qThreadSecondPart class contains functions calculating FWHM and correlation coefficient for each frame.
 */
class qThreadSecondPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadSecondPart(QStringList i_videosForAnalysis,
                               QVector<QString> i_badVideos,
                               QMap<QString,cv::Rect> i_cutoutStandard,
                               QMap<QString,cv::Rect> i_cutoutExtra,
                               QMap<QString,QVector<int>> i_badFramesCompleteList,
                               QMap<QString, int> i_videoReferencialFramesList,
                               cutoutType i_cutoutType,
                               double i_areaMaximum);
    /**
     * @brief Main class function initiating all calculations.
     */
    void run() override;
    /**
     * @brief Creates a vector of frames which can be processed.
     *
     * The function uses information about bad frames of a video to not include unsuitable frames to the vector.
     * @param badFrames - a vector of bad frames for currently processed video
     * @param frameCount - number of frames of currently processed video
     * @return Vector containing indexes of those frames which can be used in this thread.
     */
    QVector<double> vectorForFWHM(QVector<int> &badFrames, int frameCount);
    /**
     * @brief Returns the vector with computed correlation coefficients.
     */
    QMap<QString,double> computedCC();
    /**
     * @brief Returns the vector with computed FWHM parameters.
     */
    QMap<QString, double> computedFWHM();
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
private:
    /**
     * @brief If an error occurs, a video is marked as unprocessable.
     */
    void fillEmpty(QString i_videoName);

    QStringList videoList;
    double videoCount;
    double frameCount;
    double areaMaximum;
    QMap<QString,cv::Rect> obtainedCutoffStandard;
    QMap<QString,cv::Rect> obtainedCutoffExtra;
    QMap<QString,QVector<int>> badFramesComplete;
    cutoutType selectedCutout = cutoutType::STANDARD;
    QMap<QString,double> CC;
    QMap<QString,double> FWHM;
    QMap<QString,int> referencialFrames;
    QVector<QString> notProcessThis;
private slots:
    /**
     * @brief When calculated data is downloaded, fires corresponding signal to init deletion of the thread.
     */
    void onDataObtained();
};

#endif // QTHREADSECONDPART_H
