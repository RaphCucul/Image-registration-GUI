#ifndef REGISTRATIONTHREAD_H
#define REGISTRATIONTHREAD_H

#include <QObject>
#include <QThread>
#include <QVector>

#include <opencv2/opencv.hpp>

class RegistrationThread : public QThread
{
    Q_OBJECT
public:
    RegistrationThread(cv::VideoCapture& cap,
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
    int registrateTheBest(cv::VideoCapture& cap,
                          cv::Mat& referencni_snimek,
                          cv::Point3d bod_RefS_reverse,
                          int index_posunuty,
                          int iterace,
                          double oblastMaxima,
                          double uhel,
                          cv::Rect& vyrez_korelace_extra,
                          cv::Rect& vyrez_korelace_standard,
                          bool zmena_meritka,
                          QVector<double> &parametry_frangi,
                          QVector<double>& frangi_x,
                          QVector<double>& frangi_y,
                          QVector<double>& frangi_euklid,
                          QVector<double>& POC_x,
                          QVector<double>& POC_y,
                          QVector<double>& angleList);
    int fullRegistration(cv::VideoCapture& cap,
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
    int imagePreprocessing(cv::Mat &reference,
                           cv::Mat &obraz,
                           QVector<double> &parFrang,
                           cv::Point3d& frangiMaxEstimated,
                           cv::Point2f &hraniceAnomalie,
                           cv::Point2f &hraniceCasu,
                           cv::Rect &oblastAnomalie,
                           cv::Rect &vyrezKoreEx,
                           cv::Rect &vyrezKoreStand,
                           cv::VideoCapture &cap,
                           bool pritomnostAnomalie,
                           bool casZnacka,
                           bool &zmeMer);
    int registrationCorrection(cv::Mat& slicovany_snimek,
                               cv::Mat& obraz,
                               cv::Rect& vyrez_korelace_standard,
                               cv::Point3d& korekce_bod,
                               int index_posunuty,
                               QVector<double>& finalni_POC_x,
                               QVector<double>& finalni_POC_y);
signals:
    void allWorkDone(int);

private:
    cv::VideoCapture capture;
    cv::Mat referencialImage;
    QVector<double> frangiParameters;
    QVector<double> frangiX,frangiY,frangiEuklidean,POCx,POCy,finalPOCx,finalPOCy,maximalAngles;
    QVector<int> ohodnoceniSnimku;
    int noMoved;
    int iteration;
    int startingFrame;
    int stoppingFrame;
    double maximalArea;
    double angle;
    float casovaZnacka;
    float svetelAnomalie;
    cv::Rect correl_standard;
    cv::Rect correl_extra;
    cv::Rect anomalyCutoff;
    bool scaling=false;
    double totalAngle=0.0;
    cv::Point3d totalTranslation;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_CasZnac;
};

#endif // REGISTRATIONTHREAD_H
