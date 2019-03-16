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
    explicit RegistrationThread(int& indexOfThread,
                                QString &fullVideoPath,
                                QString& nameOfVideo,
                                QVector<double>& frangiParam,
                                QVector<int>& frameEvaluation,
                                cv::Mat& referencni_snimek,
                                int& startFrame,
                                int& stopFrame,
                                int& iterace,
                                double& oblastMaxima,
                                double& uhel,
                                int &timeStamp,
                                int &lightAnomaly,
                                bool nutnost_zmenit_velikost_snimku,
                                QObject *parent = nullptr);
    QMap<QString,QVector<double>> provideResults();
    QVector<int> threadFrameRange();

signals:
    void allWorkDone(int);
    void errorDetected(int,QString);
    /*void x_coordInfo(int,int,QTableWidgetItem*);
    void y_coordInfo(int,int,QTableWidgetItem*);
    void angleInfo(int,int,QTableWidgetItem*);*/
    void x_coordInfo(int,int,QString);
    void y_coordInfo(int,int,QString);
    void angleInfo(int,int,QString);
    //void finalParameter(int,int,double);
    void statusInfo(int,int,QString);
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
    float casovaZnacka;
    float svetelAnomalie;
    double frameCount;
    cv::Rect correl_standard;
    cv::Rect correl_extra;
    cv::Rect anomalyCutoff;
    bool scaling=false;
    double totalAngle=0.0;
    cv::Point3d totalTranslation;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_CasZnac;
    int threadIndex = -1;
    QString videoName;
    QString videoPath;
};

#endif // REGISTRATIONTHREAD_H
