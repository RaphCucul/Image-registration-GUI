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

/**
 * @class SingleVideoRegistration
 * @brief The SingleVideoRegistration class is derived from the RegistrationParent class and provides the option to select
 * one video for the registration process defined by chosen parameters.
 *
 * Unlike in the MultiVideoRegistration class, this class enables to edit the name of the video to load it quickly. If the selected name
 * refers to a loadable video, it is automatically loaded and a user can immediately continue to work.
 * It is absolutely vital that a video chosen for the registration process was already analysed and its *.dat file
 * can be located. Video without its *.dat file will never be processed.
 */
class SingleVideoRegistration : public RegistrationParent
{
    Q_OBJECT

public:
    explicit SingleVideoRegistration(QWidget *parent = nullptr);
    ~SingleVideoRegistration();

    /**
     * @brief Processes QJsonObject object with loaded data from a *.dat file.
     * @param videoData - loaded data
     */
    void processVideoParameters(QJsonObject& videoData);

    /**
     * @brief Applies results of registration on original frames and produces video with
     * registrated frames. Produced video is not compressed.
     * @return
     */
    void writeToVideo();
signals:
    /**
     * @brief If a problem occurs during writing procedure, the videowriter thread is terminated and deleted.
     */
    void terminateWriter();
private slots:
    /**
     * @brief Called when a user changes the name of the video in the QLineEdit widget.
     * @param arg1 - new video name
     */
    void on_chooseVideoLE_textChanged(const QString &arg1);

    /**
     * @brief Opens QFileDialog so a user can choose a video for analysis.
     * @sa processFilePath(QString i_wholePaht, QString& o_folder, QString& o_onlyFilename, QString& o_suffix)
     */
    void on_chooseVideoPB_clicked();

    /**
     * @brief Main registration function where the registration process starts.
     */
    void registrateVideoframes();

    /**
     * @brief Adds given string into the corresponding table.
     * @param row - table row
     * @param column - table column
     * @param parameter - a value represented by a string
     */
    void addItem(int row,int column,QString parameter);

    /**
     * @brief Adds frame processing status to the table.
     * @param row - table row
     * @param column - table column
     * @param status - a status represented by a string
     */
    void addStatus(int row, int column, QString status);

    /**
     * @brief Handles error signals from the RegistrationThread.
     * @param indexOfThread
     * @param errorMessage - custom error message
     */
    void errorHandler(int indexOfThread,QString errorMessage);

    /**
     * @brief Starts processing of another video if available. All imported function are invoked here
     * before moving to another video.
     * @param indexOfThread
     */
    void processAnother(int indexOfThread);

    /**
     * @brief Saves complete calculated information.
     */
    void on_savePB_clicked();

    /**
     * @brief Deactivates specific widgets when the calculations start.
     */
    void disableWidgets();

    /**
     * @brief Enables specific widgets when the calculations stop.
     */
    void enableWidgets();

    /**
     * @brief Checks if the path to the video folder exists. If so, the first video from the list of files
     * found in the folder is placed into the line edit.
     */
    void checkPaths();

    /**
     * @brief When a registration process is finished and video is written, the function starts to process another video.
     */
    void continueAlgorithm();

    /**
     * @brief Displays a predefined error message when an error occurs during the video writing process.
     * @param errorNumber - index reference to a predefined error message
     */
    void processWriterError(int errorNumber);

    /**
     * @brief Displays a custom error message when an error occurs during the video writing process.
     * @param errorMessage - custom message
     */
    void processWriterError(QString errorMessage);

    /**
     * @brief When a video is written successfully, a signal is emitted to terminate and delete the thread.
     * @sa terminateWriter()
     */
    void processSuccess();

    /**
     * @brief When a registration is done and video with POC results is created, it is possible to check the results immediately.
     */
    void showRegistrationResult();
private:
    /**
     * @brief Processes computed translations of frames in specific range the thread was
     * initialized for.
     * @param analysedThread
     */
    void processResults(int analysedThread);

    /**
     * @brief Creates registration thread.
     *
     * The range of processed frames is represented by lower and upper limit. The more parts the video is divided,
     * the faster the registration process is finished.
     * Before the registration process is started, standard and extra cutouts are extracted (if possible) to be applied in the
     * algorithm.
     * @param indexThread - internal index
     * @param cap - processed video
     * @param lowerLimit - lowest video frame index to be processed
     * @param upperLimit - highest video frame index to be processed
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
