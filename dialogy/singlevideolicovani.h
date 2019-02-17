#ifndef SINGLEVIDEOLICOVANI_H
#define SINGLEVIDEOLICOVANI_H

#include "util/licovaniparent.h"
#include "licovani/registrationthread.h"
#include "fancy_staff/sharedvariables.h"
#include <QWidget>
#include <QVector>
#include <QList>
#include <QJsonObject>
#include <QTableWidget>

namespace Ui {
class SingleVideoLicovani;
}

class SingleVideoLicovani : public LicovaniParent
{
    Q_OBJECT

public:
    explicit SingleVideoLicovani(QWidget *parent = nullptr);
    ~SingleVideoLicovani();
    void processVideoParameters(QJsonObject& videoData);
    void checkPaths();
    /*void licuj(cv::VideoCapture& cap,
               QVector<double> frangiParam,
               cv::Mat& referencni_snimek,
               int startFrame,
               int stopFrame,
               int iterace,
               double oblastMaxima,
               double angle,
               float timeStamp,
               float lightAnomaly);
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
                          QVector<double> &parametry_frangi);
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
                           bool &zmeMer);
    int registrationCorrection(cv::Mat& slicovany_snimek,
                               cv::Mat& obraz,
                               cv::Mat& snimek_korigovany,
                               cv::Rect& vyrez_korelace_standard,
                               cv::Point3d& korekce_bod);*/
    int writeToVideo();

    void createAndRunThreads(int indexProcVid, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
private slots:
    void on_vybraneVideoLE_textChanged(const QString &arg1);
    void on_vyberVideaPB_clicked();
    void slicovatSnimkyVidea();
    void totalFramesCompleted(int frameCounter);
    void addItem(int row,int column,QTableWidgetItem* item);
private:
    Ui::SingleVideoLicovani *ui;
    RegistrationThread *regThread;
    QString fullVideoPath;
    QTableWidget* widgetForScrollArea;
    QVector<QString> chosenVideo;
    QVector<QString> chosenJson;
    QList<QPointer<RegistrationThread>> registrationThreadsList;
    QJsonObject videoParametersJson;
    QVector<double> frangiX,frangiY,frangiEuklidean,finalPOCx,finalPOCy,maximalAngles;
    cv::Rect correl_standard;
    cv::Rect correl_extra;
    cv::Rect anomalyCutoff;
    double angle = 0.0;
    int internalCounter = 0;
    double actualFrameCount = 0.0;
};

#endif // SINGLEVIDEOLICOVANI_H
