#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QJsonObject>
#include <opencv2/opencv.hpp>
#include <QTableWidgetItem>

#include "multithreadET/qThreadFirstPart.h"
#include "multithreadET/qthreadsecondpart.h"
#include "multithreadET/qthreadthirdpart.h"
#include "multithreadET/qthreadfourthpart.h"
#include "multithreadET/qthreadfifthpart.h"
#include "dialogs/errordialog.h"
#include "util/etanalysisparent.h"
#include "shared_staff/sharedvariables.h"

using namespace clickImageEnums;

namespace Ui {
class MultipleVideoET;
}

/**
 * @class MultipleVideoET
 * @brief The MultipleVideoET class is derived from the ETanalysisParent class and provides the option to select multiple videos
 * for the E+T analysis.
 *
 * It is possible to load the content of a folder (but only *.avi videos are selected), or it is possible to pick a limited
 * number of videos from a folder. The class also supports drag&drop function so the content of a folder can be added to the program
 * very quickly.
 * It is absolutely vital that all videos chosen for the registration process were already analysed and theirs *.dat files
 * can be located. Videos without their *.dat file will never be processed.
 */
class MultipleVideoET : public ETanalysisParent
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    virtual ~MultipleVideoET() override;
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
private slots:
    /**
     * @brief Opens a QFileDialogue allowing to select multiple *.avi files.
     */
    void on_afewVideosPB_clicked();

    /**
     * @brief Opens a QFileDialogue allowing to select a folder with *.avi file(s).
     */
    void on_wholeFolderPB_clicked();

    /**
     * @brief Starts E+T analysis of videos.
     */
    void on_analyzeVideosPB_clicked();

    /**
     * @brief When an analysis of all videos is completed, it is possible to go through calculated results
     * and change a few parameters - thresholds, evaluation indexes.
     */
    void on_showResultsPB_clicked();

    /**
     * @brief Helper function for keyPressEvent function handles the deletion of selected QListWidget
     * items.
     */
    void deleteSelectedFiles();

    /**
     * @brief The stop and start of individual threads is processed here.
     * @param thread - index of the thread
     */
    void onDone(int thread);

    /**
     * @brief Displays the name of actually processed video.
     * @param index - index of the video in the list
     */
    void newVideoProcessed(int index);

    /**
     * @brief Displays the name of actually processed calculation.
     * @param method - internal index of the method
     */
    void movedToMethod(int method);

    /**
     * @brief When one of thread is unexpectedly terminated, this function is called to raise an error message
     * and to terminate all calculations.
     * @param videoIndex
     * @param errorType - expected "hardError" type
     */
    void onUnexpectedTermination(int videoIndex, QString errorType);

    /**
     * @brief Saves computed information to corresponding video *.dat file.
     */
    void on_savePB_clicked();

    /**
     * @brief When editing of "areaMaximum" parameter is finished, the function checks corectness.
     * @sa checkValuesPass()
     */
    void on_areaMaximum_editingFinished();

    /**
     * @brief When editing of "rotationAngle" parameter is finished, the function checks corectness.
     * @sa checkValuesPass()
     */
    void on_rotationAngle_editingFinished();

    /**
     * @brief When editing of "iterationCount" parameter is finished, the function checks corectness.
     * @sa checkValuesPass()
     */
    void on_iterationCount_editingFinished();

    /**
     * @brief Enables the main push button if all parameters - areaMaximum, iterationCount and rotationAngle - are set
     * correctly.
     * @sa on_areaMaximum_editingFinished()
     * @sa on_rotationAngle_editingFinished()
     * @sa on_iterationCount_editingFinished()
     */
    void evaluateCorrectValues();

    /**
     * @brief Creates a new ClickImageEvent class object so a user can mark standard/extra cutout.
     */
    void showDialog();

    /**
     * @brief Disables specific widgets when the E+T analysis starts.
     */
    void disableWidgets();

    /**
     * @brief Enables specific widgets when the E+T analysis stop.
     */
    void enableWidgets();

    /**
     * @brief Handles the command to save data if it comes from the GraphET class object.
     * @param i_videoName - video name to identify data to be saved
     * @param i_object - JSON object with information changed in the GraphET class object
     */
    void onSaveFromGraphET(QString i_videoName, QJsonObject i_object);
signals:
    /**
     * @brief If the value is added and if it is correct, the signal is emitted and evaluateCorrectValues
     * slot responds.
     */
    void checkValuesPass();
private:
    /**
     * @brief Checks if a video is loadable.
     * @return - true if everything is correct, otherwise false
     */
    bool checkVideo(QString i_video);

    /**
     * @brief Overridden QWidget keyPressEvent function. The reaction on "Delete" key press
     * is handled here.
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief Checks if *.dat file for analysed video exists and tries to load "thresholds" parameter from the file.
     * @param i_videoName
     * @return - true if thresholds were found, otherwise false.
     */
    bool checkFileAndLoadThresholds(QString i_videoName);

    /**
     * @brief Fills QTableWidget with obtained data. If "fillInternalVariables" parameter is "true", specific internal
     * variables are filled with additional information obtained (if possible) from a *.dat file of a video.
     * @param fillInternalVariables - indicates, if additional parameters should be loaded (if possible)
     */
    void fillTable(bool fillInternalVariables);

    /**
     * @brief Helper function creates and returns a QLabel containing a pixmap with an icon.
     * @param i_icon - name of the icon loaded from program resources
     */
    QLabel *createIconTableItem(QString i_icon);

    /**
     * @brief Enables/disables three main inputs widgets (area maximum, rotation angle, iteration count).
     * @param status - "true" or "false"
     */
    void inputWidgetsEnabledStatus(bool status);

    /**
     * @brief Checks if userly selected cutout fulfils all necessary criteria to be accepted.
     *
     * Basically, the modified cutout cannot be empty and its width and height must be greated than zero.
     */
    void checkSelectedCutout(cutoutType i_type);

    Ui::MultipleVideoET *ui;
    bool runStatus = true,canProceed = true,standardLoaded=false,extraLoaded=false;
    QMap<QString,bool> ETthresholdsFound;
    cutoutType selectedCutout = cutoutType::NO_CUTOUT;
    QMap<QString,bool> readyToProcess;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // MULTIPLEVIDEOET_H
