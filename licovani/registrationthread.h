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
    RegistrationThread(cv::VideoCapture& cap,
                       int indexOfThread,
                       QString nameOfVideo,
                       QVector<double> frangiParam,
                       QVector<int> frameEvaluation,
                       cv::Mat& referencni_snimek,
                       int startFrame,
                       int stopFrame,
                       int iterace,
                       double oblastMaxima,
                       double uhel,
                       float timeStamp,
                       float lightAnomaly,
                       bool nutnost_zmenit_velikost_snimku);
    void run() override;
    QMap<QString,QVector<double>> provideResults();
    bool registrateTheBest(cv::VideoCapture& i_cap,
                          cv::Mat& i_referencialFrame,
                          cv::Point3d i_coordsFrangiStandardReferencialReverse,
                          int i_index_translated,
                          int i_iteration,
                          double i_areaMaximum,
                          double i_angle,
                          cv::Rect& i_cutoutExtra,
                          cv::Rect& i_cutoutStandard,
                          bool i_scaleChanged,
                          QVector<double> &parametry_frangi);
    bool fullRegistration(cv::VideoCapture& cap,
                         cv::Mat& referencni_snimek,
                         int cislo_posunuty,
                         int iterace,
                         double oblastMaxima,
                         double uhel,
                         cv::Rect& korelacni_vyrez_navic,
                         cv::Rect& korelacni_vyrez_standardni,
                         bool nutnost_zmenit_velikost_snimku,
                         cv::Mat& slicovany_kompletne,
                         cv::Point3d& mira_translace,
                         double& celkovy_uhel);
    bool imagePreprocessing(cv::Mat &reference,
                           cv::Mat &obraz,
                           QVector<double> &parFrang,
                           cv::Point3d& frangiMaxEstimated,
                           cv::Point2f &hraniceAnomalie,
                           cv::Point2f &hraniceCasu,
                           cv::Rect &oblastAnomalie,
                           cv::Rect &vyrezKoreEx,
                           cv::Rect &vyrezKoreStand,
                           cv::VideoCapture &cap,
                           bool &zmeMer);
    bool registrationCorrection(cv::Mat& slicovany_snimek,
                               cv::Mat& obraz,
                               cv::Mat& snimek_korigovany,
                               cv::Rect& vyrez_korelace_standard,
                               cv::Point3d& korekce_bod);
signals:
    void allWorkDone(int);
    void errorDetected(QString);
    void x_coordInfo(int,int,QTableWidgetItem*);
    void y_coordInfo(int,int,QTableWidgetItem*);
    void angleInfo(int,int,QTableWidgetItem*);
    void statusInfo(int,int,QString);
private:
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
};

#endif // REGISTRATIONTHREAD_H
