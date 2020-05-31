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
    void run() override;
    QVector<double> vectorForFWHM(QVector<int> &badFrames, int frameCount);
    QMap<QString,double> computedCC();
    QMap<QString, double> computedFWHM();
    QVector<QString> unprocessableVideos();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination(int,QString);
    void readyForFinish();
private:
    /**
     * @brief If error, fill the vectors with zeros.
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
    void onDataObtained();
};

#endif // QTHREADSECONDPART_H
