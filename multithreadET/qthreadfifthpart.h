#ifndef QTHREADFIFTHPART_H
#define QTHREADFIFTHPART_H

#include <QThread>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>

class qThreadFifthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFifthPart(QStringList& videos,
                              cv::Rect& CO_s,
                              cv::Rect& CO_e,
                              QVector<QVector<double>>& POCX,
                              QVector<QVector<double>>& POCY,
                              QVector<QVector<double>>& Angle,
                              QVector<QVector<double>>& Fr_X,
                              QVector<QVector<double>>& Fr_Y,
                              QVector<QVector<double>>& Fr_E,
                              bool scaleChanged,
                              QVector<QVector<int>> &EvaluationComplete,
                              QVector<QVector<int>>& frEvalSec,
                              QVector<int>& referFrames,
                              QVector<double> FrangiParams,
                              QObject* parent = nullptr);
    void run() override;
    QVector<QVector<int>> framesUpdateEvaluationComplete();
    QVector<QVector<double>> framesFrangiXestimated();
    QVector<QVector<double>> framesFrangiYestimated();
    QVector<QVector<double>> framesFrangiEuklidestimated();
    QVector<QVector<double>> framesPOCXestimated();
    QVector<QVector<double>> framesPOCYestimated();
    QVector<QVector<double>> framesAngleestimated();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination();
private:
    double videoCount;
    double frameCount;
    QStringList videoList;
    QVector<QVector<int>> framesSecondEval,framesCompleteEvaluation;
    QVector<QVector<double>> POC_x,POC_y,angle,frangi_x,frangi_y,frangi_euklid;
    QVector<double> average_CC,average_FWHM,FrangiParameters;
    QVector<int> referencialFrames;
    cv::Rect obtainedCutoffStandard;//ziskany_VK_standard;
    cv::Rect obtainedCutoffExtra;//ziskany_VK_extra;
    bool scaleCh;
};

#endif // QTHREADFIFTHPART_H
