#ifndef QTHREADFOURTHPART_H
#define QTHREADFOURTHPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

class qThreadFourthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFourthPart(QStringList& videos,
                               QVector<QVector<int>>& framesFirstEvaluationDefinitive,
                               QVector<QVector<int>>& evalCompl,
                               QVector<QVector<double>>& CCproblematic,
                               QVector<QVector<double>>& FWHMproblematic,
                               QVector<QVector<double>>& POCX,
                               QVector<QVector<double>>& POCY,
                               QVector<QVector<double>>& Angles,
                               QVector<QVector<double>>& Fr_X,
                               QVector<QVector<double>>& Fr_Y,
                               QVector<QVector<double>>& Fr_Eu,
                               QVector<double>& averageCC,
                               QVector<double>& averageFWHM,
                               QObject* parent=nullptr);
    void run() override;
    QVector<QVector<int>> framesSecondEvaluation();
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
    QStringList videoList;
    QVector<QVector<int>> framesFirstEvaluationComplete,framesSecondEvaluationComplete,framesCompleteEvaluation;
    QVector<QVector<double>> computedCC,computedFWHM,POC_x,POC_y,angle,frangi_x,
    frangi_y,frangi_euklid;
    QVector<double> averageCCcomplete,averageFWHMcomplete;
    double videoCount;
    double frameCount;
};

#endif // QTHREADFOURTHPART_H
