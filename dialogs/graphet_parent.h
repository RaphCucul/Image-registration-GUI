#ifndef GRAPHET_PARENT_H
#define GRAPHET_PARENT_H

#include <QWidget>
#include <QDialog>
#include <QJsonObject>
#include <QMap>

#include <opencv2/opencv.hpp>

namespace Ui {
class GraphET_parent;
}

/**
 * @class GraphET_parent
 * @brief The GraphET_parent class creates GrafET objects for each video from the list. GrafET objects are represented
 * by individual tabs.
 */
class GraphET_parent : public QDialog
{
    Q_OBJECT

public:
    /**
     * @enum namesFlag
     * @brief The namesFlag enum helps to identify, what should appear in the tab.
     */
    enum class namesFlag {
        Fullname,FilenameOnly
    };

    explicit GraphET_parent(QStringList i_chosenList, QWidget *parent = nullptr);
    explicit GraphET_parent(QStringList i_chosenList,
                            QMap<QString,QVector<double>> i_entropy,
                            QMap<QString,QVector<double>> i_tennengrad,
                            QMap<QString,QVector<double>> i_thresholds,
                            QMap<QString,QVector<int>> i_FirstEvalEntropy,
                            QMap<QString,QVector<int>> i_FirstEvalTennengrad,
                            QMap<QString,QVector<int>> i_FirstDecisionResults,
                            QMap<QString,QVector<int>> i_SecondDecisionResults,
                            QMap<QString,QVector<int>> i_CompleteEvaluation,
                            QWidget *parent = nullptr);
    ~GraphET_parent();
signals:
    void saveCalculatedData(QString videoName, QJsonObject object);
public slots:
    /**
     * @brief Saves all parameters which can be changed by a user.
     * @param i_videoName
     * @param i_newData - JSON object with complete information
     */
    void saveDataForGivenVideo(QString i_videoName,QJsonObject i_newData);
private:
    /**
     * @brief Analyses the length of all given video names to calculate proper width of tabs.
     * May extract the name of videos.
     * @param parameter
     */
    void analyseNames(namesFlag parameter);

    /**
     * @brief Loads data from chosen *.dat files, creates GraphET object and adds a new tab
     * to the tabWidget.
     */
    void loadAndShow();

    /**
     * @brief Loads data from the file of the given video and merges original data (if found) with actual data.
     * @param i_videoName
     * @param loadedData
     * @return True if loading was successful.
     */
    bool prepareData(QString i_videoName, QJsonObject &loadedData);

    /**
     * @brief Creates the grap directly from given data. Used for SingleVideoET and MultipleVideoET classes.
     */
    void processAndShow(QMap<QString,QVector<double>> i_entropy,
                        QMap<QString,QVector<double>> i_tennengrad,
                        QMap<QString,QVector<double>> i_thresholds,
                        QMap<QString,QVector<int>> i_FirstEvalEntropy,
                        QMap<QString,QVector<int>> i_FirstEvalTennengrad,
                        QMap<QString,QVector<int>> i_FirstDecisionResults,
                        QMap<QString,QVector<int>> i_SecondDecisionResults,
                        QMap<QString,QVector<int>> i_CompleteEvaluation);

    /**
     * @brief Sets size of the dialogue.
     */
    void sizeSettings();
    Ui::GraphET_parent *ui;
    QStringList fileList,filenameList;
    QStringList neededParameters = {"entropy","tennengrad","thresholds","evaluation","firstEvalEntropy",
                                    "firstEvalTennengrad","firstEval","secondEval"};
    QStringList videoParameters = {"entropy","tennengrad","FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "angle","thresholds","evaluation","firstEvalEntropy","firstEvalTennengrad",
                                 "firstEval","secondEval","standard","extra"};
    QMap<QString,QString> videoReferences;
    QMap<QString,QMap<QString,cv::Rect>> videosAnomalies;
};

#endif // GRAPHET_PARENT_H
