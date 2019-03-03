#ifndef QTHREADFIRSTPART_H
#define QTHREADFIRSTPART_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>

class qThreadFirstPart : public QThread
{
    Q_OBJECT
public:
   explicit qThreadFirstPart(QStringList _videosForAnalysis, cv::Point2d _verticalAnomalyCoords,
                             cv::Point2d _horizontalAnomalyCoords,QVector<double>_FrangiParametersValues,
                             QObject* parent=nullptr);
    QVector<QVector<double>> computedEntropy();
    QVector<QVector<double>> computedTennengrad();
    QVector<QVector<int>> computedFirstEntropyEvaluation();
    QVector<QVector<int>> computedFirstTennengradEvalueation();
    QVector<QVector<int>> computedCompleteEvaluation();
    QVector<QVector<int>> computedBadFrames();
    cv::Rect computedCOstandard();
    cv::Rect computedCOextra();
    QVector<int> estimatedReferencialFrames();
    void run() override;

private:
    QVector<QVector<double>> entropyComplete;
    QVector<QVector<double>> tennengradComplete;
    QVector<QVector<int>> framesFirstFullCompleteEntropyEvaluation;
    QVector<QVector<int>> framesFirstFullCompleteTennengradEvaluation;
    QVector<QVector<int>> framesFullCompleteDecision;
    QVector<QVector<int>> badFramesComplete;
    QVector<int> referencialFrames;
    QVector<double> FrangiParameters;
    QStringList processVideos;
    int videoCount;
    double percent;
    cv::Point2d horizontalAnomaly;
    cv::Point2d verticalAnomaly;
    cv::Rect obtainedCutoffStandard;//ziskany_VK_standard;
    cv::Rect obtainedCutoffExtra;//ziskany_VK_extra;
    bool anomalyPresence = false;

signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination(QString,int,QString);
};

#endif // QTHREADFIRSTPART_H
