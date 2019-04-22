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
    explicit qThreadFourthPart(QStringList i_videos,
                               QVector<int> i_badVideos,
                               QVector<QVector<int>> i_framesFirstEvaluationDefinitive,
                               QVector<QVector<int>> i_evalCompl,
                               QVector<QVector<double>> i_CCproblematic,
                               QVector<QVector<double>> i_FWHMproblematic,
                               QVector<QVector<double>> i_POCX,
                               QVector<QVector<double>> i_POCY,
                               QVector<QVector<double>> i_Angles,
                               QVector<QVector<double>> i_Fr_X,
                               QVector<QVector<double>> i_Fr_Y,
                               QVector<QVector<double>> i_Fr_Eu,
                               QVector<double> i_averageCC,
                               QVector<double> i_averageFWHM,
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
    void readyForFinish();
private slots:
    void onDataObtained();
private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty(int i_frameCount);

    QStringList videoList;
    QVector<QVector<int>> framesFirstEvaluationComplete,framesSecondEvaluationComplete,framesCompleteEvaluation;
    QVector<QVector<double>> computedCC,computedFWHM,POC_x,POC_y,angle,frangi_x,
    frangi_y,frangi_euklid;
    QVector<int> notProcessThese;
    QVector<double> averageCCcomplete,averageFWHMcomplete;
    double videoCount;
    double framesToAnalyse;
};

#endif // QTHREADFOURTHPART_H
