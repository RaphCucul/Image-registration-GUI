#ifndef QTHREADFOURTHPART_H
#define QTHREADFOURTHPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>
#include <QMap>

class qThreadFourthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFourthPart(QStringList i_videos,
                               QVector<QString> i_badVideos,
                               QMap<QString,QVector<int>> i_framesFirstEvaluationDefinitive,
                               QMap<QString,QVector<int>> i_evalCompl,
                               QMap<QString,QVector<double>> i_CCproblematic,
                               QMap<QString,QVector<double>> i_FWHMproblematic,
                               QMap<QString,QVector<double>> i_POCX,
                               QMap<QString,QVector<double>> i_POCY,
                               QMap<QString,QVector<double>> i_Angles,
                               QMap<QString,QVector<double>> i_Fr_X,
                               QMap<QString,QVector<double>> i_Fr_Y,
                               QMap<QString,QVector<double>> i_Fr_Eu,
                               QMap<QString,double> i_averageCC,
                               QMap<QString,double> i_averageFWHM,
                               QObject* parent=nullptr);
    void run() override;
    QMap<QString,QVector<int>> framesSecondEvaluation();
    QMap<QString,QVector<int>> framesUpdateEvaluationComplete();
    QMap<QString,QVector<double>> framesFrangiXestimated();
    QMap<QString,QVector<double>> framesFrangiYestimated();
    QMap<QString,QVector<double>> framesFrangiEuklidestimated();
    QMap<QString,QVector<double>> framesPOCXestimated();
    QMap<QString,QVector<double>> framesPOCYestimated();
    QMap<QString,QVector<double>> framesAngleestimated();
    QVector<QString> unprocessableVideos();
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
    void fillEmpty(QString i_videoName, int i_frameCount);

    QStringList videoList;
    QMap<QString,QVector<int>> framesFirstEvaluationComplete,framesSecondEvaluationComplete,framesCompleteEvaluation;
    QMap<QString,QVector<double>> computedCC,computedFWHM,POC_x,POC_y,angle,frangi_x,
    frangi_y,frangi_euklid;
    QVector<QString> notProcessThese;
    QMap<QString,double> averageCCcomplete,averageFWHMcomplete;
    double videoCount;
    double framesToAnalyse;
};

#endif // QTHREADFOURTHPART_H
