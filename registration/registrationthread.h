#ifndef REGISTRATIONTHREAD_H
#define REGISTRATIONTHREAD_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QTableWidgetItem>

#include <opencv2/opencv.hpp>

/**
 * @class RegistrationThread
 * @brief The RegistrationThread class is derived from QThread class. An object of this class is used to perform frames registration
 * in an independent thread.
 *
 * The object receives information about the range of frames which should be registrated because the registration
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
                                cv::Rect& i_cutoutExtra,
                                cv::Rect& i_cutoutStandard,
                                bool i_scaleChange,
                                QMap<QString,int> i_margins,
                                QMap<QString,double> i_ratios,
                                QObject *parent = nullptr);
    /**
     * @brief Returns calculated data. Maximum frangi coordinates and POC shift is calculated for each frame from the
     * range, if possible.
     */
    QMap<QString,QVector<double>> provideResults();

    /**
     * @brief Returns start and stop frame of the analysis.
     */
    QVector<int> threadFrameRange();

    /**
     * @brief Emits a signal to indicate the class object can be destroyed.
     */
    void dataObtained();

signals:
    /**
     * @brief The algorithm has finished, data can be "downloaded".
     *
     * Integer variable represents the numeric identificator of the thread with RegistrationThread object.
     */
    void allWorkDone(int);
    /**
     * @brief An error occured in the critical part of the algorithm - calculations stopped, error message is sent to
     * the main thread.
     */
    void errorDetected(int,QString);
    /**
     * @brief X coordinates info.
     */
    void x_coordInfo(int,int,QString);
    /**
     * @brief Y coordinates info.
     */
    void y_coordInfo(int,int,QString);
    /**
     * @brief Rotation angle info.
     */
    void angleInfo(int,int,QString);
    /**
     * @brief Frame processing status (done/error).
     */
    void statusInfo(int,int,QString);
    /**
     * @brief Ready to be deleted.
     */
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
    int iteration;
    int startingFrame;
    int stoppingFrame;
    double maximalArea;
    double angle;
    cv::Rect standardCutout;
    cv::Rect extraCutout;
    double frameCount;
    bool scaling=false;
    double totalAngle=0.0;
    int threadIndex = -1;
    QString videoName;
    QString videoPath;
    QMap<QString,int> margins;
    QMap<QString,double> ratios;
};

#endif // REGISTRATIONTHREAD_H
