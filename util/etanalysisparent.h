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
    virtual ~ETanalysisParent() { }
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
    void cancelAllCalculations(QStringList i_videoNames);

    /**
     * @brief Function initialise QMaps with vectors of vectors, filled by values during analysis.
     */
    void initMaps(QStringList i_videoNames);

    /**
     * @brief Template function initMap initialises one specific map variable (mapDouble, mapInt)
     */
    template <typename T>
    QMap<QString,QMap<QString,QVector<T>>> initMap(QStringList parameters,
                                                          QStringList i_videoList,
                                                          int start_pos,
                                                          int end_pos) {
        QMap<QString,QMap<QString,QVector<T>>> outputMap;
        QVector<T> _v;
        for (int parameterIndex = start_pos; parameterIndex <= end_pos; parameterIndex++){
            QMap<QString,QVector<T>> _pom;
            foreach (QString name, i_videoList){                
                _pom.insert(name,_v);                
            }
            outputMap.insert(parameters.at(parameterIndex),_pom);
        }

        return outputMap;
    }

    /**
     * @brief initAnomaly
     * @param parameters
     * @param i_videoList
     * @param start_pos
     * @param end_pos
     * @return
     */
    QMap<QString,QMap<QString,cv::Rect>> initAnomaly(QStringList parameters,
                                                            QStringList i_videoList,
                                                            int start_pos,
                                                            int end_pos);

    /**
     * @brief Template function fillMap fills given map variable with key and corresponding value. If key
     * already exists in the map variable, the value is just changed. If it does not exist, then new pair
     * key-value is inserted into the map variable.
     */
    template<class T, class S>
    void fillMap(T i_key, S i_value, QMap<T,S>& i_map){
        if (i_map.contains(i_key))
            i_map[i_key] = i_value;
        else
            i_map.insert(i_key,i_value);
    }

    /**
     * @brief Function where output values of individual threads are processed.
     * @param done
     */
    void done(int done);

    /**
     * @brief saveVideoAnalysisResults
     */
    void saveVideoAnalysisResults();

    /**
     * @brief saveVideoAnalysisResultsFromGraphET
     */
    void saveVideoAnalysisResultsFromGraphET(QString i_videoName, QJsonObject i_object);

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
    QStringList analysedVideos, videoNamesList;
    QVector<QString> chosenVideoETSingle;
    bool videoETScorrect = false;
    bool horizontalAnomalySelected = false;
    bool verticalAnomalySelected = false;
    QVector<double> actualEntropy, actualTennengrad;
    QVector<QString> badVideos;

    QMap<QString,QMap<QString,QVector<double>>> mapDouble;
    QMap<QString,QMap<QString,QVector<int>>> mapInt;
    QMap<QString,QMap<QString,cv::Rect>> mapAnomalies;
    //QMap<QString,QMap<QString,QVector<double>>> mapThresholds;
    QMap<QString, QVector<double>> CC_problematicFrames,FWHM_problematicFrames;
    QStringList videoParameters = {"entropy","tennengrad","FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "angle","thresholds","evaluation","firstEvalEntropy","firstEvalTennengrad",
                                 "firstEval","secondEval","standard","extra"};
    QMap<QString,int> framesReferencial;
    QMap<QString,double> averageCCcomplete;
    QMap<QString,double> averageFWHMcomplete;
    QMap<QString,QVector<int>> badFramesComplete;

    double iterationCount = -99.0;
    double areaMaximum = -99.0;
    double rotationAngle = -99.0;
    bool areaMaximumCorrect = false;
    bool rotationAngleCorrect = false;
    bool iterationCountCorrect = false;

    cv::Point3d maximum_frangi;

    QJsonObject videoParametersJson;
    int referencialNumber;
};

#endif // ETANALYSISPARENT_H
