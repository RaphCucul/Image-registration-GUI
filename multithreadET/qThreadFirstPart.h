#ifndef QTHREADFIRSTPART_H
#define QTHREADFIRSTPART_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>
#include <QMap>

#include "shared_staff/sharedvariables.h"
using namespace clickImageEnums;
class qThreadFirstPart : public QThread
{
    Q_OBJECT
public:
   explicit qThreadFirstPart(QStringList i_videosForAnalysis,
                             cutoutType i_cutoutType,
                             QMap<QString, QVector<double>> ETthresholds,
                             QMap<QString, bool> ETthresholdsFound,
                             bool i_previousThresholdsUsageAllowed,
                             QObject* parent=nullptr);
    /**
     * @brief computedEntropy
     * @return
     */
    QMap<QString, QVector<double> > computedEntropy();
    /**
     * @brief computedTennengrad
     * @return
     */
    QMap<QString, QVector<double> > computedTennengrad();
    /**
     * @brief computedFirstEntropyEvaluation
     * @return
     */
    QMap<QString, QVector<int> > computedFirstEntropyEvaluation();
    /**
     * @brief computedFirstTennengradEvalueation
     * @return
     */
    QMap<QString, QVector<int> > computedFirstTennengradEvalueation();
    /**
     * @brief computedCompleteEvaluation
     * @return
     */
    QMap<QString, QVector<int> > computedCompleteEvaluation();
    /**
     * @brief computedBadFrames
     * @return
     */
    QMap<QString, QVector<int> > computedBadFrames();
    /**
     * @brief computedCOstandard
     * @return
     */
    QMap<QString,cv::Rect> computedCOstandard();
    /**
     * @brief computedCOextra
     * @return
     */
    QMap<QString, cv::Rect> computedCOextra();
    /**
     * @brief computedThresholds
     * @return
     */
    QMap<QString,QVector<double>> computedThresholds();
    /**
     * @brief estimatedReferencialFrames
     * @return
     */
    QMap<QString,int> estimatedReferencialFrames();

    /**
     * @brief estimatedReferentialFrangiCoordinates
     * @return
     */
    QMap<QString,cv::Point3d> estimatedReferentialFrangiCoordinates() { return maximum_frangi_reverse; };

    /**
     * @brief unprocessableVideos
     * @return
     */
    QVector<QString> unprocessableVideos();

    /**
     * @brief run
     */
    void run() override;

private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty(QString i_videoName);

    QMap<QString,QVector<double>> entropyComplete,tennengradComplete, previousThresholds;
    QMap<QString,QVector<int>> framesFirstFullCompleteEntropyEvaluation;
    QMap<QString,QVector<int>> framesFirstFullCompleteTennengradEvaluation;
    QMap<QString,QVector<int>> framesFullCompleteDecision;
    QMap<QString,QVector<int>> badFramesComplete;
    QMap<QString,int> referencialFrames;
    QMap<QString, bool> videosWithThresholdsFound;
    //QVector<double> FrangiParameters;
    QVector<QString> doNotProcessThis;
    QStringList processVideos;
    int videoCount;
    double percent;
    QMap<QString,cv::Rect> obtainedCutoffStandard;
    QMap<QString,cv::Rect> obtainedCutoffExtra;
    QMap<QString,QVector<double>> calculatedETthresholds;
    bool usePreviousThresholds = false;
    cutoutType selectedCutout = cutoutType::STANDARD;

    QMap<QString,cv::Point3d> maximum_frangi_reverse;
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
