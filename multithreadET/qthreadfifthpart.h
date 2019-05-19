#ifndef QTHREADFIFTHPART_H
#define QTHREADFIFTHPART_H

#include <QThread>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>
#include <QMap>

class qThreadFifthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFifthPart(QStringList i_videos,
                              QVector<int> i_badVideos,
                              QVector<cv::Rect> i_standardCutout,
                              QVector<cv::Rect> i_extraCutout,
                              QVector<QVector<double>> i_POCX,
                              QVector<QVector<double>> i_POCY,
                              QVector<QVector<double>> i_Angle,
                              QVector<QVector<double>> i_Fr_X,
                              QVector<QVector<double>> i_Fr_Y,
                              QVector<QVector<double>> i_Fr_E,
                              bool i_scaleChanged,
                              QVector<QVector<int>> i_EvaluationComplete,
                              QVector<QVector<int>> i_frEvalSec,
                              QVector<int> i_referFrames,
                              QVector<double> i_FrangiParams,
                              int i_iteration,
                              double i_areaMaximum,
                              double i_maximalAngle,
                              QMap<QString, int> i_margins,
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
    void unexpectedTermination(int,QString);
    void readyForFinish();
private slots:
    void onDataObtained();
private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty(int i_frameCount);

    double videoCount, framesToAnalyse, maximalAngle, areaMaximum;
    int iteration;
    QStringList videoList;
    QVector<QVector<int>> framesSecondEval,framesCompleteEvaluation;
    QVector<QVector<double>> POC_x,POC_y,angle,frangi_x,frangi_y,frangi_euklid;
    QVector<double> average_CC,average_FWHM,FrangiParameters;
    QVector<int> referencialFrames,notProcessThese;
    QVector<cv::Rect> obtainedCutoffStandard, obtainedCutoffExtra;
    bool scaleCh;

    QMap<QString, int> margins;
};

#endif // QTHREADFIFTHPART_H
