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
 * @brief The VideoWriter class provides functions to write frames into a new video in a separate QThread class object.
 */
class VideoWriter : public QObject
{
    Q_OBJECT
public:
    VideoWriter(QString i_videoFullPath,QMap<QString,QVector<double>> i_data, QString i_writePath);
    ~VideoWriter();

    friend class RegistrationParent;
public slots:
    /**
     * @brief Slot function starts to write frames into a new video when a QThread class object is ready for use.
     */
    void writeVideo();
signals:
    void errorOccured(int errorIndex); /**< Emits index of the error text which should be raised */
    void errorOccured(QString errorMessage); /**< Emits error message which should appear in the error label */
    void finishedSuccessfully(); /**< Emitted when the operation is finished */
private:
    QString videoReadPath,videoWritePath;
    QMap<QString,QVector<double>> obtainedData;
};

/**
 * @class RegistrationParent
 * @brief The RegistrationParent class is a parent class for the registration classes.
 */
class RegistrationParent : public QWidget
{
    Q_OBJECT
public:
    explicit RegistrationParent(QWidget *parent = nullptr);
    virtual ~RegistrationParent() { }
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
     * @brief When the thread emits all calculations are done, the thread is terminated
     * in this function.
     */
    void onFinishThread(int);
protected:
    /**
     * @brief It initialises QMaps with vectors of vectors, filled by values during analysis.
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
    int numberOfThreads = QThread::idealThreadCount()-1;
    QHash<int,RegistrationThread*> threadPool;

    QStringList videoParameters = {"FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "angle","evaluation","standard","extra"};
    QMap<QString,QVector<double>> videoParametersDouble;
    QMap<QString,QVector<int>> videoParametersInt;
    QMap<QString,QVector<int>> videoAnomalies;
};

#endif // REGISTRATIONPARENT_H
