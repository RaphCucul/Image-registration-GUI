#ifndef REGISTRATIONTHREAD_H
#define REGISTRATIONTHREAD_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QTableWidgetItem>

#include <opencv2/opencv.hpp>

class RegistrationThread : public QThread
{
    Q_OBJECT
public:
    explicit RegistrationThread(int& i_indexOfThread,
                                QString &i_fullVideoPath,
                                QString& i_nameOfVideo,
                                QVector<double>& i_frangiParameters,
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
    QMap<QString,QVector<double>> provideResults();
    QVector<int> threadFrameRange();
    void dataObtained();

signals:
    void allWorkDone(int);
    void errorDetected(int,QString);
    void x_coordInfo(int,int,QString);
    void y_coordInfo(int,int,QString);
    void angleInfo(int,int,QString);
    void statusInfo(int,int,QString);
    void readyForFinish(int);
private slots:

private:
    void run();



    cv::VideoCapture capture;
    cv::Mat referencialImage;
    QVector<double> frangiParameters;
    QVector<double> frangiX,frangiY,frangiEuklidean,finalPOCx,finalPOCy,maximalAngles;
    QMap<QString,QVector<double>> vectors;
    QVector<int> ohodnoceniSnimku;
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
