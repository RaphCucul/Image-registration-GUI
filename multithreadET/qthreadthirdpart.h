#ifndef QTHREADTHIRDPART_H
#define QTHREADTHIRDPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

#include <opencv2/opencv.hpp>

class qThreadThirdPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadThirdPart(QStringList& sV, QVector<QVector<int>>& sSprOhKomplet,QVector<QVector<int>>& ohodKomp,
                     QVector<int>& refKomplet,QVector<double>& RK, QVector<double>& FWHMK,cv::Rect& VK_standard, cv::Rect& VK_extra,
                     bool zM, QObject* parent=nullptr);
    void run() override;
    QVector<QVector<int>> framesUpdateEvaluation();
    QVector<QVector<int>> framesFirstEvaluationComplete();
    QVector<QVector<double>> framesProblematic_CC();
    QVector<QVector<double>> framesProblematic_FWHM();
    QVector<QVector<double>> framesFrangiXestimated();
    QVector<QVector<double>> framesFrangiYestimated();
    QVector<QVector<double>> framesFrangiEuklidestimated();
    QVector<QVector<double>> framesPOCXestimated();
    QVector<QVector<double>> framesPOCYestimated();
    QVector<QVector<double>> framesUhelestimated();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination(QString,int,QString);
private:
    QStringList videoList;
    QVector<double> averageCCcomplete;
    QVector<double> averageFWHMcomplete;
    QVector<QVector<int>> badFrames_firstEvaluation;
    QVector<QVector<int>> framesEvaluationCompelte;
    QVector<QVector<int>> framesFirstCompleteEvaluation;
    QVector<QVector<double>> framesComputedCC,framesComputedFWHM;
    QVector<int> referencialFrames;
    cv::Rect obtainedCutoffStandard;//ziskany_VK_standard;
    cv::Rect obtainedCutoffExtra;//ziskany_VK_extra;
    bool scaleChanged;
    QVector<QVector<double>> framesFrangiX;
    QVector<QVector<double>> framesFrangiY;
    QVector<QVector<double>> framesFrangiEuklid;
    QVector<QVector<double>> framesPOCX;
    QVector<QVector<double>> framesPOCY;
    QVector<QVector<double>> framesUhel;
    double videoCount;
    double frameCount;
};

#endif // QTHREADTHIRDPART_H
