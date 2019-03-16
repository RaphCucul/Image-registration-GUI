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
#include <QLineEdit>

class ETanalysisParent : public QWidget
{
    Q_OBJECT
public:
    ETanalysisParent(QWidget *parent);
protected:
    void checkInputNumber(double input,double lower,double upper,QLineEdit* editWidget,double& finalValue,bool& evaluation);
    void evaluateCorrectValues();

    qThreadFirstPart* TFirstP;
    qThreadSecondPart* TSecondP;
    qThreadThirdPart* TThirdP;
    qThreadFourthPart* TFourthP;
    qThreadFifthPart* TFifthP;

    double frameCountActualVideo;
    int analysisCompleted = 0;
    QStringList analysedVideos;
    QVector<QString> chosenVideoETSingle;
    bool videoETScorrect = false;
    bool horizontalAnomalySelected = false;
    bool verticalAnomalySelected = false;
    QVector<double> actualEntropy, actualTennengrad;
    /*QVector<QVector<double>> entropy;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<int>> framesFirstFullCompleteEntropyEvaluation; //snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> framesFirstFullCompleteTennengradEvaluation; //snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> framesFirstFullCompleteDecision; //snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<int>> framesSecondFullCompleteDecision; //snimkyDruheRozhodovaniKomplet;
    QVector<QVector<int>> framesFinalCompleteDecision; //snimkyOhodnoceniKomplet;    
    QVector<QVector<double>> framesFrangiX; //snimkyFrangiX;
    QVector<QVector<double>> framesFrangiY; //snimkyFrangiY;
    QVector<QVector<double>> framesFrangiEuklid; //snimkyFrangiEuklid;
    QVector<QVector<double>> framesPOCX; //snimkyPOCX;
    QVector<QVector<double>> framesPOCY; //snimkyPOCY;
    QVector<QVector<double>> framesAngle; //snimkyUhel;*/
    QVector<int> framesReferencial;
    QVector<double> averageCCcomplete; //prumerneRkomplet
    QVector<double> averageFWHMcomplete; //prumerneFWHMkomplet
    QVector<QVector<int>> badFramesComplete; //spatnesnimkyohodnocenikomplet

    //QVector<int> horizontalAnomalyPresent; //PritomnostCasoveZnacky;
    //QVector<int> verticalAnomalyPresent; //PritomnostSvetelneAnomalie;
    double iterationCount = -99.0;
    double areaMaximum = -99.0;
    double rotationAngle = -99.0;
    bool areaMaximumCorrect = false;
    bool rotationAngleCorrect = false;
    bool iterationCountCorrect = false;

    cv::Rect obtainedCutoffStandard;//ziskany_VK_standard;
    cv::Rect obtainedCutoffExtra;//ziskany_VK_extra;
    cv::Point3d maximum_frangi;

    QJsonObject videoParametersJson;
    int referencialNumber;
};

#endif // ETANALYSISPARENT_H
