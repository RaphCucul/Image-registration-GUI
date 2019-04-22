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
   explicit qThreadFirstPart(QStringList i_videosForAnalysis,
                             cv::Point2d i_verticalAnomalyCoords,
                             cv::Point2d i_horizontalAnomalyCoords,
                             QVector<double> i_FrangiParametersValues,
                             QObject* parent=nullptr);
    QVector<QVector<double>> computedEntropy();
    QVector<QVector<double>> computedTennengrad();
    QVector<QVector<int>> computedFirstEntropyEvaluation();
    QVector<QVector<int>> computedFirstTennengradEvalueation();
    QVector<QVector<int>> computedCompleteEvaluation();
    QVector<QVector<int>> computedBadFrames();
    QVector<cv::Rect> computedCOstandard();
    QVector<cv::Rect> computedCOextra();
    QVector<int> estimatedReferencialFrames();
    void run() override;

private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty(int i_frameCount);

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
    QVector<cv::Rect> obtainedCutoffStandard;
    QVector<cv::Rect> obtainedCutoffExtra;
    bool anomalyPresence = false;

signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination(int,QString);
    void readyForFinish();
private slots:
    void onDataObtained();
};

#endif // QTHREADFIRSTPART_H
