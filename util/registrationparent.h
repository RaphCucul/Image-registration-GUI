#ifndef REGISTRATIONPARENT_H
#define REGISTRATIONPARENT_H

#include "registration/registrationthread.h"

#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QProgressBar>
#include <QMap>
#include <QString>
#include <QLabel>
#include <QPointer>
#include <QThread>

/**
 * @class VideoWriter
 * @brief Provides functions to write frames into a new video in a separate QThread class object.
 *
 * It also reflects a user choice to write only those frames with best evaluation index.
 */
class VideoWriter : public QObject
{
    Q_OBJECT
public:
    VideoWriter(QString i_videoFullPath,QMap<QString,QVector<double>> i_data,
                QVector<int> i_evaluationInformation, QString i_writePath, bool onlyBest);
    ~VideoWriter();

    friend class RegistrationParent;
public slots:
    /**
     * @brief Starts to write frames into a new video when a QThread class object is ready for use.
     */
    void writeVideo();
signals:
    /**
     * @brief Emits index of the error text which should be raised (from errors.h).
     * @param errorIndex
     */
    void errorOccured(int errorIndex);
    /**
     * @brief Emits error message which should appear in the error label.
     * @param errorMessage - custom error text
     */
    void errorOccured(QString errorMessage);
    /**
     * @brief Emitted when the operation is finished.
     */
    void finishedSuccessfully();
private:
    QString videoReadPath,videoWritePath;
    QMap<QString,QVector<double>> obtainedData;
    QVector<int> evaluation;
    bool onlyBestFrames = false;
};

/**
 * @class RegistrationParent
 * @brief It is a parent class for the video registration classes.
 */
class RegistrationParent : public QWidget
{
    Q_OBJECT
public:
    explicit RegistrationParent(QWidget *parent = nullptr);
    virtual ~RegistrationParent() { }
signals:
    /**
     * @brief Informs the GUI to block all tabs except the current one
     * to prevent the program from unexpected behaviour.
     */
    void calculationStarted();

    /**
     * @brief Informs the GUI all procedures have stopped. All actions are enabled.
     */
    void calculationStopped();
public slots:
    /**
     * @brief When the thread emits all calculations are done, the thread is terminated
     * in this function.
     */
    void onFinishThread(int);
protected:
    /**
     * @brief Initialises QMaps with vectors of vectors, filled by values during analysis.
     */
    void initMaps();

    /**
     * @brief Clears the content of all thread QMaps.
     */
    void cancelAllCalculations();

    QStringList videoList;
    QStringList videoListFull;
    QStringList videoListNames;

    bool runStatus = true;
    bool canProceed = true;
    int processedVideoNo = 0;
    int numberOfThreads = 1;
    int finishedThreadCounter = 0;
    QHash<int,RegistrationThread*> threadPool;

    QStringList videoParameters = {"FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "angle","evaluation","standard","extra"};
    QMap<QString,QVector<double>> videoParametersDouble;
    QMap<QString,QVector<int>> videoParametersInt;
    QMap<QString,QVector<int>> videoAnomalies;
};

#endif // REGISTRATIONPARENT_H
