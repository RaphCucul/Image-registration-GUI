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

class VideoWriter : public QObject
{
    Q_OBJECT
public:
    VideoWriter(QString i_videoFullPath,QMap<QString,QVector<double>> i_data, QString i_writePath);
    ~VideoWriter();

    friend class LicovaniParent;
public slots:
    void writeVideo();
signals:
    void errorOccured(int errorIndex);
    void errorOccured(QString errorMessage);
    void finished();
private:
    QString videoReadPath,videoWritePath;
    QMap<QString,QVector<double>> obtainedData;
};

class LicovaniParent : public QWidget
{
    Q_OBJECT
public:
    explicit LicovaniParent(QWidget *parent = nullptr);
signals:
    /**
     * @brief Signal informs the GUI to block all tabs except the current one
     * to prevent the program from unexpected behaviour.
     */
    void calculationStarted();

    /**
     * @brief Signal informs the GUI all procedures have stopped. All actions are enabled.
     */
    void calculationStopped();
public slots:
    /**
     * @brief When the thread emits the finish of all calculations, the thread is terminated
     * in this function.
     */
    void onFinishThread(int);

    /**
     * @brief Function handles the end of video write.
     */
    void onVideoWriterFinished();
protected:
    /**
     * @brief Function initialise QMaps with vectors of vectors, filled by values during analysis.
     */
    void initMaps();

    /**
     * @brief Function clears the content of all thread QMaps.
     */
    void cancelAllCalculations();

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
