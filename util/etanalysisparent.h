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
    /**
     * @brief Function checks, if the numerous input is in the set range. If the input is out of range,
     * font color is red, otherwise green.
     * @param input
     * @param lower
     * @param upper
     * @param editWidget
     * @param finalValue
     * @param evaluation
     */
    void checkInputNumber(double input,double lower,double upper,QLineEdit* editWidget,double& finalValue,bool& evaluation);
    void evaluateCorrectValues();

    /**
     * @brief Function initialise QMaps with vectors of vectors, filled by values during analysis.
     */
    void initMaps();

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

    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;
    QMap<QString,QVector<int>> mapAnomalies;
    QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad",
                                 "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};

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
