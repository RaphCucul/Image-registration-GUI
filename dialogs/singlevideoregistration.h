#ifndef SINGLEVIDEOREGISTRATION_H
#define SINGLEVIDEOREGISTRATION_H

#include "util/registrationparent.h"
#include "registration/registrationthread.h"
#include "shared_staff/sharedvariables.h"
#include "dialogs/errordialog.h"
#include <QWidget>
#include <QVector>
#include <QList>
#include <QJsonObject>
#include <QTableWidget>

namespace Ui {
class SingleVideoRegistration;
}

class SingleVideoRegistration : public RegistrationParent
{
    Q_OBJECT

public:
    explicit SingleVideoRegistration(QWidget *parent = nullptr);
    ~SingleVideoRegistration();

    /**
     * @brief processVideoParameters
     * @param videoData
     */
    void processVideoParameters(QJsonObject& videoData);

    /**
     * @brief writeToVideo
     * @return
     */
    bool writeToVideo();
signals:
    void terminateWriter();
private slots:
    /**
     * @brief on_chooseVideoLE_textChanged
     * @param arg1
     */
    void on_chooseVideoLE_textChanged(const QString &arg1);

    /**
     * @brief on_chooseVideoPB_clicked
     */
    void on_chooseVideoPB_clicked();

    /**
     * @brief registrateVideoframes
     */
    void registrateVideoframes();

    /**
     * @brief totalFramesCompleted
     * @param frameCounter
     */
    void totalFramesCompleted(int frameCounter);

    /**
     * @brief Adds given string into the corresponding table.
     * @param row
     * @param column
     * @param parameter
     */
    void addItem(int row,int column,QString parameter);

    /**
     * @brief Adds frame processing status to the table.
     * @param row
     * @param column
     * @param status
     */
    void addStatus(int row, int column, QString status);

    /**
     * @brief Handes the error signals from the RegistrationThread.
     * @param indexOfThread
     * @param errorMessage
     */
    void errorHandler(int indexOfThread,QString errorMessage);

    /**
     * @brief Starts processing of another video if available. All imported function are invoked in this
     * function before moving to another video.
     * @param indexOfThread
     */
    void processAnother(int indexOfThread);

    /**
     * @brief VideoWriterErrorHandler
     * @param errorIndex
     */
    void VideoWriterErrorHandler(int errorIndex);

    /**
     * @brief VideoWriterErrorHandler
     * @param errorMessage
     */
    void VideoWriterErrorHandler(QString errorMessage);

    /**
     * @brief on_savePB_clicked
     */
    void on_savePB_clicked();

    /**
     * @brief Function deactivates specific widgets when the calculations start.
     */
    void disableWidgets();

    /**
     * @brief Function re-enables specific widgets when the calculations stop.
     */
    void enableWidgets();

    /**
     * @brief The function checks, if the path to the video folder exists. If so, the first video from the list of files
     * found in the folder is placed into the line edit.
     */
    void checkPaths();

    /**
     * @brief continueAlgorithm
     */
    void continueAlgorithm();

    /**
     * @brief processWriterError
     * @param errorNumber
     */
    void processWriterError(int errorNumber);

    /**
     * @brief processWriterError
     * @param errorMessage
     */
    void processWriterError(QString errorMessage);

    /**
     * @brief processSuccess
     */
    void processSuccess();

    /**
     * @brief showRegistrationResult
     */
    void showRegistrationResult();
private:
    /**
     * @brief Function process computed translations of frames in specific range the thread was
     * initialized for.
     * @param analysedThread
     */
    void processResults(int analysedThread);

    /**
     * @brief populateLists
     * @param _file
     */
    void populateLists(QVector<QString> _file);

    /**
     * @brief createAndRunThreads
     * @param indexThread
     * @param cap
     * @param lowerLimit
     * @param upperLimit
     */
    void createAndRunThreads(int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
    Ui::SingleVideoRegistration *ui;
    QString fullVideoPath;
    QMap<QString,QString> chosenVideo;
    QVector<QString> chosenJson;
    QJsonObject videoParametersJson;
    cv::Rect correl_standard;
    cv::Rect correl_extra;
    cv::Rect anomalyCutoff;
    double angle = 0.0;
    int internalCounter = 0;
    double actualFrameCount = 0.0;
    int videoCounter = 0;
    int threadProcessed = 0;
    bool readyToContinue = false;

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // SINGLEVIDEOREGISTRATION_H
