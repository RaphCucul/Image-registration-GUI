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

/**
 * @class ETanalysisParent
 * @brief The ETanalysisParent class is a parent class for entropy and tennegrad analysis classes. Signal functions
 * are emitted when the corresponding thread is finished and calculated parameters are obtained.
 */
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
     * @brief It checks if the numerical input is in the given range. If the input is out of range,
     * font color is set red, otherwise green.
     * @param input - user input
     * @param lower - minimum
     * @param upper - maximum
     * @param editWidget - affected QLineEdit
     * @param finalValue - -99 returned if a problem is discovered
     * @param evaluation - true if input in the range, otherwise false
     */
    void checkInputNumber(double i_input,double i_lower,double i_upper,
                          QLineEdit* i_editWidget,double& i_finalValue,bool& i_evaluation);

    /**
     * @brief It terminates all running threads and clears the content of QMap objects.
     */
    void cancelAllCalculations();

    /**
     * @brief Initialises QMaps with empty QMaps prepared for analysed video(s).
     */
    void initMaps();

    /**
     * @brief Template function initialises QMap object for given video(s).
     * @tparam T - type of the vector in the QMap pair (double, int)
     * @tparam parameters - a list of parameters of analysed video(s)
     * @tparam i_videoList - a list of video names
     * @tparam start_pos - index of the parameter in the list
     * @tparam end_pos - index of the parameter in the list
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
     * @brief It separates initialisation function for anomalies QMap object (standard and extra cutout defined by a user).
     * Because a QMap is storing cv::Rect object not a vector, it is easier to do the necessary things in a separate
     * function.
     * @param parameters - a list of parameters of analysed video(s)
     * @param i_videoList - a list of video names
     * @param start_pos - index of the parameter in the list
     * @param end_pos - index of the parameter in the list
     */
    QMap<QString,QMap<QString,cv::Rect>> initAnomaly(QStringList parameters,
                                                     QStringList i_videoList,
                                                     int start_pos,
                                                     int end_pos);

    /**
     * @brief Template function fills input map variable with a key and a value. If the key
     * already exists in the map variable, the value is just changed. If it does not exist, then new pair
     * key-value is inserted into the map variable.
     * @tparam i_key - a QMap key of type T
     * @tparam i_value - a QMap value of type S
     * @tparam i_map - a QMap with keys of type T and values of type S
     */
    template<class T, class S>
    void fillMap(T i_key, S i_value, QMap<T,S>& i_map){
        if (i_map.contains(i_key))
            i_map[i_key] = i_value;
        else
            i_map.insert(i_key,i_value);
    }

    /**
     * @brief It processes calculated parameters from a thread and initialises its termination.
     * @param done
     */
    void done(int done);

    /**
     * @brief Saves complete information of all analysed videos into corresponding *.dat file.
     */
    void saveVideoAnalysisResults();

    /**
     * @brief Saves all available information about a video to its *.dat file together with probably updated parameters
     * from GrafET class object. These parameters are updated or added if missing to the file. If the file already exists,
     * its content is just updated and original data is preserved.
     * @param i_videoName - name of the video
     * @param i_object
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
    QMap<QString,QString> chosenVideoETSingle;
    bool videoETScorrect = false;
    bool horizontalAnomalySelected = false;
    bool verticalAnomalySelected = false;
    QVector<double> actualEntropy, actualTennengrad;
    QVector<QString> badVideos;

    QMap<QString,QMap<QString,QVector<double>>> mapDouble;
    QMap<QString,QMap<QString,QVector<int>>> mapInt;
    QMap<QString,QMap<QString,cv::Rect>> mapAnomalies;
    QMap<QString,QVector<double>> temporalySavedThresholds;
    //QMap<QString,QMap<QString,QVector<double>>> mapThresholds;
    QMap<QString, QVector<double>> CC_problematicFrames,FWHM_problematicFrames;
    QStringList videoParameters = {"entropy","tennengrad","FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "angle","thresholds","evaluation","firstEvalEntropy","firstEvalTennengrad",
                                 "firstEval","secondEval","standard","extra"};
    QMap<QString,int> framesReferencial;
    QMap<QString,cv::Point3d> framesReferentialFrangiCoordinates;
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
