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
                              QVector<QString> i_badVideos,
                              QMap<QString,cv::Rect> i_standardCutout,
                              QMap<QString,cv::Rect> i_extraCutout,
                              QMap<QString,QVector<double>> i_POCX,
                              QMap<QString,QVector<double>> i_POCY,
                              QMap<QString,QVector<double>> i_Angle,
                              QMap<QString,QVector<double>> i_Fr_X,
                              QMap<QString,QVector<double>> i_Fr_Y,
                              QMap<QString,QVector<double>> i_Fr_E,
                              bool i_scaleChanged,
                              QMap<QString,QVector<int>> i_EvaluationComplete,
                              QMap<QString,QVector<int>> i_frEvalSec,
                              QMap<QString,int> i_referFrames,
                              QVector<double> i_FrangiParams,
                              int i_iteration,
                              double i_areaMaximum,
                              double i_maximalAngle,
                              QMap<QString, int> i_margins,
                              QObject* parent = nullptr);
    void run() override;
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
    void unexpectedTermination(int,QString);
    void readyForFinish();
private slots:
    void onDataObtained();
private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty(QString i_videoName, int i_frameCount);

    double videoCount, framesToAnalyse, maximalAngle, areaMaximum;
    int iteration;
    QStringList videoList;
    QMap<QString,QVector<int>> framesSecondEval,framesCompleteEvaluation;
    QMap<QString,QVector<double>> POC_x,POC_y,angle,frangi_x,frangi_y,frangi_euklid;
    QVector<double> average_CC,average_FWHM,FrangiParameters;
    QMap<QString,int> referencialFrames;
    QVector<QString> notProcessThese;
    QMap<QString,cv::Rect> obtainedCutoffStandard, obtainedCutoffExtra;
    bool scaleCh;

    QMap<QString, int> margins;
};

#endif // QTHREADFIFTHPART_H
