#ifndef REGISTRATIONTHREAD_H
#define REGISTRATIONTHREAD_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QTableWidgetItem>

#include <opencv2/opencv.hpp>

/**
 * @class RegistrationThread
 * @brief The RegistrationThread class is derived from QThread class. The object of RregistrationThread class is used to perform frames registration
 * in an independent thread. The object receives information about the range of frames which should be registrated because the registration
 * process enables to register more frames at one time. It is then possible divide a video into segments and registrate frames in each segment
 * independently on other segments.
 */
class RegistrationThread : public QThread
{
    Q_OBJECT
public:
    explicit RegistrationThread(int& i_indexOfThread,
                                QString &i_fullVideoPath,
                                QString& i_nameOfVideo,
                                QMap<QString,double> i_frangiParameters,
                                QVector<int>& i_frameEvaluation,
                                cv::Mat& i_referencialFrame,
                                int& startFrame,
                                int& stopFrame,
                                int& i_iteration,
                                double& i_areaMaximum,
                                double& i_angle,
                                int &i_horizAnomaly,
                                int &i_vertAnomaly,
                                bool i_scaleChange,
                                QMap<QString,int> i_margins,
                                QMap<QString,double> i_ratios,
                                QObject *parent = nullptr);
    /**
     * @brief It returns calculated data. Maximum frangi coordinates and POC shift is calculated for each frame from the
     * range, if possible.
     */
    QMap<QString,QVector<double>> provideResults();

    /**
     * @brief It returns start and stop frame of the analysis.
     */
    QVector<int> threadFrameRange();

    /**
     * @brief It emits a signal to indicate the class object can be destroyed.
     */
    void dataObtained();

signals:
    void allWorkDone(int); /**< the algorithm has finished, data can be downloaded */
    void errorDetected(int,QString); /**< an error occured in the critical part of the algorithm - calculations stopped */
    // "Info" signals are providing basic info about the progress of the algorithm
    void x_coordInfo(int,int,QString);
    void y_coordInfo(int,int,QString);
    void angleInfo(int,int,QString);
    void statusInfo(int,int,QString);
    void readyForFinish(int);
private slots:

private:
    /**
     * @brief Main registration function. Calls helper functions to get expected results.
     */
    void run();

    cv::VideoCapture capture;
    cv::Mat referencialImage;
    QMap<QString,double> frangiParameters;
    QVector<double> frangiX,frangiY,frangiEuklidean,finalPOCx,finalPOCy,maximalAngles;
    QMap<QString,QVector<double>> vectors;
    QVector<int> framesEvaluation;
    int noMoved;
    int iteration;
    int startingFrame;
    int stoppingFrame;
    int pomCounter=0;
    double maximalArea;
    double angle;
    float horizontalAnomaly;
    float verticalAnomaly;
    double frameCount;
    cv::Rect correl_standard;
    cv::Rect correl_extra;
    cv::Rect anomalyCutoff;
    bool scaling=false;
    double totalAngle=0.0;
    cv::Point3d totalTranslation;
    cv::Point2f obtainedVerticalAnomalyCoords;
    cv::Point2f obtainedHorizontalAnomalyCoords;
    int threadIndex = -1;
    QString videoName;
    QString videoPath;
    QMap<QString,int> margins;
    QMap<QString,double> ratios;
};

#endif // REGISTRATIONTHREAD_H
