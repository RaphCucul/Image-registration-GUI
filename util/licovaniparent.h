#ifndef LICOVANIPARENT_H
#define LICOVANIPARENT_H

#include "licovani/registrationthread.h"

#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QProgressBar>
#include <QMap>
#include <QString>
#include <QLabel>
#include <QPointer>
#include <QThread>

class LicovaniParent : public QWidget
{
    Q_OBJECT
public:
    explicit LicovaniParent(QWidget *parent = nullptr);
signals:
    void calculationStarted();
    void calculationStopped();
    void dataObtained();
public slots:
    void onFinishThread(int);
protected:
    /**
     * @brief Function initialise QMaps with vectors of vectors, filled by values during analysis.
     */
    void initMaps();

    /**
     * @brief Function clears the content of all thread QMaps.
     */
    void cancelAllCalculations();

    int licovaniDokonceno = 0;

    QStringList videoList;
    QStringList videoListFull;
    QStringList videoListNames;

    bool runStatus = true;
    bool canProceed = true;
    int processedVideoNo = 0;
    int numberOfThreads = QThread::idealThreadCount()-1;
    QHash<int,RegistrationThread*> threadPool;

    QStringList videoParameters = {"FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","VerticalAnomaly","HorizontalAnomaly"};
    QMap<QString,QVector<double>> videoParametersDouble;
    QMap<QString,QVector<int>> videoParametersInt;
    QMap<QString,QVector<int>> videoAnomalies;
};

#endif // LICOVANIPARENT_H
