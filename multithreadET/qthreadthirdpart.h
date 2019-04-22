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
    explicit qThreadThirdPart(QStringList i_videoList,
                              QVector<int> i_badVideos,
                              QVector<QVector<int> > i_badFramesFirstEval,
                              QVector<QVector<int> > i_framesCompleteEval,
                              QVector<int> i_framesReferencial,
                              QVector<double> i_averageCC,
                              QVector<double> i_averageFWHM,
                              QVector<cv::Rect> i_standardCutout,
                              QVector<cv::Rect> i_extraCutout,
                              bool i_scaleChange, QObject *parent=nullptr);
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
    void unexpectedTermination(int,QString);
    void readyForFinish();
private slots:
    void onDataObtained();
private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty(int i_frameCount);

    QStringList videoList;
    QVector<double> averageCCcomplete;
    QVector<double> averageFWHMcomplete;
    QVector<QVector<int>> badFrames_firstEvaluation;
    QVector<QVector<int>> framesEvaluationCompelte;
    QVector<QVector<int>> framesFirstCompleteEvaluation;
    QVector<QVector<double>> framesComputedCC,framesComputedFWHM;
    QVector<int> referencialFrames,notProcessThese;
    QVector<cv::Rect> obtainedCutoffStandard;
    QVector<cv::Rect> obtainedCutoffExtra;
    bool scaleChanged;
    QVector<QVector<double>> framesFrangiX;
    QVector<QVector<double>> framesFrangiY;
    QVector<QVector<double>> framesFrangiEuklid;
    QVector<QVector<double>> framesPOCX;
    QVector<QVector<double>> framesPOCY;
    QVector<QVector<double>> framesUhel;
    double videoCount;
};

#endif // QTHREADTHIRDPART_H
