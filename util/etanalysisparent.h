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
signals:
    void calculationStarted();
    void calculationStopped();
    void dataObtained_first();
    void dataObtained_second();
    void dataObtained_third();
    void dataObtained_fourth();
    void dataObtained_fifth();
protected:
    /**
     * @brief Function checks, if the numerical input is in the set range. If the input is out of range,
     * font color is red, otherwise green.
     * @param input
     * @param lower
     * @param upper
     * @param editWidget
     * @param finalValue
     * @param evaluation
     */
    void checkInputNumber(double i_input,double i_lower,double i_upper,
                          QLineEdit* i_editWidget,double& i_finalValue,bool& i_evaluation);

    /**
     * @brief Function clears the content of all thread QMaps.
     */
    void cancelAllCalculations();

    /**
     * @brief Function initialise QMaps with vectors of vectors, filled by values during analysis.
     */
    void initMaps();

    /**
     * @brief Function where output values of individual threads are processed.
     * @param done
     */
    void done(int done);

    // QMaps store pointers to threads - original idea was to delete content of each QMap
    // when the calculation is done -> because of problems with termination, the content is
    // not deleted
    QMap<int,qThreadFirstPart*> First;
    QMap<int,qThreadSecondPart*> Second;
    QMap<int,qThreadThirdPart*> Third;
    QMap<int,qThreadFourthPart*> Fourth;
    QMap<int,qThreadFifthPart*> Fifth;

    double frameCountActualVideo;
    int analysisCompleted = 0;
    QStringList analysedVideos;
    QVector<QString> chosenVideoETSingle;
    bool videoETScorrect = false;
    bool horizontalAnomalySelected = false;
    bool verticalAnomalySelected = false;
    QVector<double> actualEntropy, actualTennengrad;
    QVector<int> badVideos;

    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;
    QMap<QString,QVector<int>> mapAnomalies;
    QVector<QVector<double>> CC_problematicFrames,FWHM_problematicFrames;
    QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad",
                                 "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};

    QVector<int> framesReferencial;
    QVector<double> averageCCcomplete;
    QVector<double> averageFWHMcomplete;
    QVector<QVector<int>> badFramesComplete;

    double iterationCount = -99.0;
    double areaMaximum = -99.0;
    double rotationAngle = -99.0;
    bool areaMaximumCorrect = false;
    bool rotationAngleCorrect = false;
    bool iterationCountCorrect = false;
    bool canProceed = true;

    QVector<cv::Rect> obtainedCutoffStandard;
    QVector<cv::Rect> obtainedCutoffExtra;
    cv::Point3d maximum_frangi;

    QJsonObject videoParametersJson;
    int referencialNumber;
};

#endif // ETANALYSISPARENT_H
