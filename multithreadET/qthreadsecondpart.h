#ifndef QTHREADSECONDPART_H
#define QTHREADSECONDPART_H

#include <QThread>
#include <QObject>
#include <QVector>
#include <opencv2/opencv.hpp>
#include <QStringList>

class qThreadSecondPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadSecondPart(QStringList& _videosForAnalysis, cv::Rect& _cutoutStandard,
                               cv::Rect& _cutoutExtra,
                               QVector<QVector<int>>& _badFramesCompleteList, QVector<int>& _videoReferencialFramesList,
                               bool _scaleChange);
    void run() override;
    QVector<double> vectorForFWHM(QVector<int> &badFrames, int frameCount);
    QVector<double> computedCC();
    QVector<double> computedFWHM();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination(QString,int,QString);
private:
    QStringList videoList;
    double videoCount;
    double frameCount;
    cv::Rect obtainedCutoffStandard;//ziskany_VK_standard;
    cv::Rect obtainedCutoffExtra;//ziskany_VK_extra;
    QVector<QVector<int>> badFramesComplete;
    bool scaleChanged;
    QVector<double> CC;
    QVector<double> FWHM;
    QVector<int> referencialFrames;
};

#endif // QTHREADSECONDPART_H
