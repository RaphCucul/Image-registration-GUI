#ifndef ETANALYSISPARENT_H
#define ETANALYSISPARENT_H

#include "multithreadET/qThreadFirstPart.h"
#include "multithreadET/qthreadsecondpart.h"
#include "multithreadET/qthreadthirdpart.h"
#include "multithreadET/qthreadfourthpart.h"
#include "multithreadET/qthreadfifthpart.h"

#include <QThread>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QMap>
#include <QWidget>

class ETanalysisParent : public QWidget
{
    Q_OBJECT
public:
    ETanalysisParent(QWidget *parent);
protected:
    qThreadFirstPart* TFirstP;
    qThreadSecondPart* TSecondP;
    qThreadThirdPart* TThirdP;
    qThreadFourthPart* TFourthP;
    qThreadFifthPart* TFifthP;

    double frameCountActualVideo;
    int analysisCompleted = 0;
    QStringList analysedVideos;
    QVector<QString> vybraneVideoETSingle;
    bool spravnostVideaETSingle = false;
    bool volbaCasZnackyETSingle = false;
    bool volbaSvetAnomETSingle = false;
    QVector<double> actualEntropy, actualTennengrad;
    QVector<QVector<double>> entropy;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<int>> framesFirstFullCompleteEntropyEvaluation; //snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> framesFirstFullCompleteTennengradEvaluation; //snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> framesFirstFullCompleteDecision; //snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<int>> framesSecondFullCompleteDecision; //snimkyDruheRozhodovaniKomplet;
    QVector<QVector<int>> framesFinalCompleteDecision; //snimkyOhodnoceniKomplet;
    QVector<QVector<int>> badFramesComplete; //spatnesnimkyohodnocenikomplet
    QVector<QVector<double>> framesFrangiX; //snimkyFrangiX;
    QVector<QVector<double>> framesFrangiY; //snimkyFrangiY;
    QVector<QVector<double>> framesFrangiEuklid; //snimkyFrangiEuklid;
    QVector<QVector<double>> framesPOCX; //snimkyPOCX;
    QVector<QVector<double>> framesPOCY; //snimkyPOCY;
    QVector<QVector<double>> framesAngle; //snimkyUhel;
    QVector<int> framesReferencial;
    QVector<double> averageCCcomplete; //prumerneRkomplet
    QVector<double> averageFWHMcomplete; //prumerneFWHMkomplet

    QVector<int> horizontalAnomalyPresent; //PritomnostCasoveZnacky;
    QVector<int> verticalAnomalyPresent; //PritomnostSvetelneAnomalie;
    int iteration = -1;
    double areaMaximum = -1;
    double angle = 0.1;
    QVector<double> FrangiParametersVector;
    QJsonObject FrangiParametersFile;

    cv::Point2f obtainedVerticalAnomaly;//ziskane_hranice_anomalie;
    cv::Point2f obtainedHorizontalAnomaly;//ziskane_hranice_casZnac;
    cv::Rect obtainedCutoffStandard;//ziskany_VK_standard;
    cv::Rect obtainedCutoffExtra;//ziskany_VK_extra;
    cv::Point3d maximum_frangi;

    QJsonObject videoParametersJson;
    int referencialNumber;
};

#endif // ETANALYSISPARENT_H
