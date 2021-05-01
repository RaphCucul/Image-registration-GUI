#ifndef MULTIVIDEOREGISTRATION_H
#define MULTIVIDEOREGISTRATION_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QHash>
#include <QMultiMap>
#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QMovie>
#include "util/registrationparent.h"
#include "registration/registrationthread.h"
#include "dialogs/errordialog.h"

namespace Ui {
class MultiVideoRegistration;
}

/**
 * @class MultiVideoRegistration
 * @brief The MultiVideoRegistration classs is derived from the RegistrationParent class and provides the option to select multiple videos
 * for the registration process defined by chosen parameters.
 *
 * It is possible to load the content of a folder (but only *.avi videos are selected), or it is possible to pick a limited
 * number of videos from a folder. The class also supports drag&drop function so the content of a folder can be added to the program
 * very quickly.
 * It is absolutely vital that all videos chosen for the registration process were already analysed and theirs *.dat files
 * can be located. Videos without their *.dat file will never be processed.
 */
class MultiVideoRegistration : public RegistrationParent
{
    Q_OBJECT

public:
    explicit MultiVideoRegistration(QWidget *parent = nullptr);
    virtual ~MultiVideoRegistration() override;

protected:
    /**
     * @brief Overriden drop event ensures only video files will be accepted.
     * @param event
     */
    void dropEvent(QDropEvent *event) override;
    /**
     * @brief Overriden drag event checks the data has mime format text/uri-list.
     * @param event
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
signals:
    /**
     * @brief If a problem occurs during writing procedure, the videowriter thread is terminated and deleted.
     */
    void terminateWriter();
private slots:
    /**
     * @brief Deactivates specific widgets when the calculations start.
     */
    void disableWidgets();

    /**
     * @brief Enables specific widgets when the calculations stop.
     */
    void enableWidgets();
    /**
     * @brief Opens a QFileDialogue allowing to select multiple *.avi files.
     */
    void on_chooseMultiVPB_clicked();
    /**
     * @brief Opens a QFileDialogue allowing to select a folder with *.avi file(s).
     */
    void on_chooseFolderPB_clicked();
    /**
     * @brief Main registration function where the registration process starts.
     */
    void on_registratePB_clicked();
    /**
     * @brief Saves complete calculated information.
     */
    void on_saveResultsPB_clicked();

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
     * @brief Clicking on any row of the QListWidget will show, if possible, the table with computed
     * parameters.
     * @param row - cell row
     * @param column - cell column
     */
    void on_listOfVideos_cellClicked(int row, int column);
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
private:
    /**
     * @brief Processes computed translations of frames in specific range the thread was
     * initialized for.
     * @param analysedThread
     */
    void processResuluts(int analysedThread);

    /**
     * @brief Creates registration thread.
     *
     * The range of processed frames is represented by lower and upper limit. The more parts the video is divided,
     * the faster the registration process is finished.
     *
     * Before the registration process is started, standard and extra cutouts are extracted (if possible) to be applied in the
     * algorithm.
     * @param indexThread - internal index
     * @param cap - processed video
     * @param lowerLimit - lowest video frame index to be processed
     * @param upperLimit - highest video frame index to be processed
     */
    void createAndRunThreads(int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);

    /**
     * @brief Processes QJsonObject object with loaded data from a *.dat file.
     * @param videoData - loaded data
     */
    void processVideoParameters(QJsonObject& videoData,
                                QMap<QString, QVector<double>> &inputMMdouble,
                                QMap<QString, QVector<int>> &inputMMint,
                                QMap<QString, QVector<int>> &inputMManomaly);

    /**
     * @brief Applies results of registration of original frames and produces video with
     * registrated frames. Produced video is not compressed.
     * @return
     */
    void writeToVideo();

    /**
     * @brief Checks if the *.dat file of the specific video exists and is usable.
     * @param filenameToAnalyse - analysed video
     * @return true if usable, otherwise false
     */
    bool checkPath(QString filenameToAnalyse);

    /**
     * @brief Checks if the specific video exists and is usable. If exists, the registration
     * process starts.
     * @param capture - specific video
     * @return true if usable, otherwise false
     */
    bool checkVideo(cv::VideoCapture& capture);

    /**
     * @brief Checks if added video (drag&drop, QFileDialog selection) exists and is usable.
     * @param i_video - specific video
     * @return true if usable, otherwise false
     */
    bool checkVideo(QString i_video);

    /**
     * @brief Calls processVideoParameters() in the for loop for all chosen videos.
     * @param chosenVideos
     */
    void populateProperties(QStringList chosenVideos);

    /**
     * @brief Overridden QWidget keyPressEvent function. The reaction on "Delete" key press
     * is handled here.
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief Helper function for keyPressEvent function handles the deletion of selected QListWidget
     * items.
     */
    void deleteSelectedFiles();

    /**
     * @brief Displays the processing status of the specific video.
     *
     * There are four possible states - no progress, currently processed, done, error.
     * @param status
     */
    void displayStatus(QString status);

    /**
     * @brief Initialises threads were the registration algorithm runs and
     * creates a range of frames for each initialised thread.
     * @param capture - specific video
     * @sa createIconTableItem(bool icon, QString information)
     */
    void startCalculations(cv::VideoCapture& capture);

    /**
     * @brief Fills QTableWidget with calculated data.
     *
     * If "fillInternalVariables" parameter is "true", specific internal
     * variables are filled with additional information obtained (if possible) from a *.dat file of a video.
     * @param fillInternalVariables - indicates, if additional parameters should be loaded (if possible)
     */
    void fillTable(bool fillInternalVariables);

    /**
     * @brief Helper function creates and returns a QLabel containing a pixmap with an icon.
     * @param icon - name of the icon loaded from program resources
     * @param information - name of the icon in the 'images' resources
     */
    QWidget *createIconTableItem(bool icon, QString information);

    Ui::MultiVideoRegistration *ui;
    RegistrationThread *regThread;
    QVector<QVector<int>> threadRange;

    double actualFrameCount;
    QString actualVideoName;
    int controlSum = 0;
    int totalSum = 0;
    int internalCounter = 0;
    int TableRowCounter=0;
    int threadProcessed = 0;
    int videoCounter = 0;
    bool readyToContinue = false;

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;

    QHash<QString,QMap<QString,QVector<double>>> videoPropertiesDouble;
    QHash<QString,QMap<QString,QVector<int>>> videoPropertiesInt;
    QHash<QString,QMap<QString,QVector<int>>> videoPropertiesAnomaly;

    QHash<QString,QTableWidget*> videoPool;
    QHash<QString,QString> videoStatus;
    QJsonObject videoParametersJson;
    QStringList columnHeaders = {"X","Y",tr("Angle"),"Status"};
    QMap<QString,bool> readyToProcess;
};

#endif // MULTIVIDEOREGISTRATION_H
