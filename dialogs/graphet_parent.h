#ifndef GRAPHET_PARENT_H
#define GRAPHET_PARENT_H

#include <QWidget>
#include <QDialog>
#include <QJsonObject>
#include <QMap>

namespace Ui {
class GraphET_parent;
}

class GraphET_parent : public QDialog
{
    Q_OBJECT

public:
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
     * @brief saveDataForGivenVideo
     * @param i_videoName
     */
    void saveDataForGivenVideo(QString i_videoName,QJsonObject i_newData);
private:
    /**
     * @brief Function analyses the length of all given video names.
     */
    void analyseNames(namesFlag parameter);

    /**
     * @brief Function loads data from chosen *.dat files, creates GraphET class object and adds a new tab
     * to the tabWidget.
     */
    void loadAndShow();

    /**
     * @brief prepareData
     * @param i_videoName
     * @param loadedData
     * @return
     */
    bool prepareData(QString i_videoName, QJsonObject &loadedData);

    /**
     * @brief Function corresponds to loadAndShow() function but it does not load data from the *.dat file,
     * it creates the grap directly from given data. Used for SingleVideoET and MultipleVideoET classes.
     */
    void processAndShow(QMap<QString,QVector<double>> i_entropy,
                        QMap<QString,QVector<double>> i_tennengrad,
                        QMap<QString,QVector<double>> i_thresholds,
                        QMap<QString,QVector<int>> i_FirstEvalEntropy,
                        QMap<QString,QVector<int>> i_FirstEvalTennengrad,
                        QMap<QString,QVector<int>> i_FirstDecisionResults,
                        QMap<QString,QVector<int>> i_SecondDecisionResults,
                        QMap<QString,QVector<int>> i_CompleteEvaluation);

    Ui::GraphET_parent *ui;
    QStringList fileList,filenameList;
    QStringList neededParameters = {"entropy","tennengrad","thresholds","evaluation","firstEvalEntropy",
                                    "firstEvalTennengrad","firstEval","secondEval"};
    QStringList videoParameters = {"entropy","tennengrad","FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "angle","thresholds","evaluation","firstEvalEntropy","firstEvalTennengrad",
                                 "firstEval","secondEval","standard","extra"};
    QMap<QString,QString> videoReferences;
};

#endif // GRAPHET_PARENT_H
