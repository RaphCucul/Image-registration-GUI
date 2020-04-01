#ifndef QTHREADTHIRDPART_H
#define QTHREADTHIRDPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>
#include <QMap>

#include <opencv2/opencv.hpp>

class qThreadThirdPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadThirdPart(QStringList i_videoList,
                              QVector<QString> i_badVideos,
                              QMap<QString,QVector<int>> i_badFramesFirstEval,
                              QMap<QString,QVector<int>> i_framesCompleteEval,
                              QMap<QString,int> i_framesReferencial,
                              QMap<QString,double> i_averageCC,
                              QMap<QString,double> i_averageFWHM,
                              QMap<QString,cv::Rect> i_standardCutout,
                              QMap<QString,cv::Rect> i_extraCutout,
                              bool i_scaleChange,
                              double i_areaMaximum,
                              QObject *parent=nullptr);
    void run() override;
    QMap<QString,QVector<int>> framesUpdateEvaluation();
    QMap<QString,QVector<int>> framesFirstEvaluationComplete();
    QMap<QString, QVector<double> > framesProblematic_CC();
    QMap<QString,QVector<double>> framesProblematic_FWHM();
    QMap<QString,QVector<double>> framesFrangiXestimated();
    QMap<QString,QVector<double>> framesFrangiYestimated();
    QMap<QString,QVector<double>> framesFrangiEuklidestimated();
    QMap<QString,QVector<double>> framesPOCXestimated();
    QMap<QString,QVector<double>> framesPOCYestimated();
    QMap<QString,QVector<double>> framesUhelestimated();
    QVector<QString> unprocessableVideos();
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
    void fillEmpty(QString i_videoName, int i_frameCount);

    QStringList videoList;
    QMap<QString,double> averageCCcomplete, averageFWHMcomplete;
    QMap<QString,QVector<int>> badFrames_firstEvaluation, framesEvaluationCompelte, framesFirstCompleteEvaluation;
    QMap<QString,QVector<double>> framesComputedCC,framesComputedFWHM;
    QMap<QString,int> referencialFrames;
    QVector<QString> notProcessThese;
    QMap<QString,cv::Rect> obtainedCutoffStandard, obtainedCutoffExtra;
    bool scaleChanged;
    QMap<QString,QVector<double>> framesFrangiX;
    QMap<QString,QVector<double>> framesFrangiY;
    QMap<QString,QVector<double>> framesFrangiEuklid;
    QMap<QString,QVector<double>> framesPOCX;
    QMap<QString,QVector<double>> framesPOCY;
    QMap<QString,QVector<double>> framesUhel;
    double videoCount,areaMaximum;
};

#endif // QTHREADTHIRDPART_H
