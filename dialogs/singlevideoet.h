#ifndef SINGLEVIDEOET_H
#define SINGLEVIDEOET_H

#include "util/etanalysisparent.h"
#include "dialogs/errordialog.h"
#include "shared_staff/sharedvariables.h"

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QJsonObject>
#include <QMap>

using namespace clickImageEnums;

namespace Ui {
class SingleVideoET;
}

/**
 * @class SingleVideoET
 * @brief The SingleVideoET class is derived from the ETanalysisParent class and provides the option to select one video
 * for E+T analysis and start the analysis with chosen parameters.
 *
 * Unlike in the MultipleVideoET, this class enables to edit the name of the video to load it quickly. If the selected name
 * refers to a loadable video, it is automatically loaded and a user can immediately continue to work.
 */
class SingleVideoET : public ETanalysisParent
{
    Q_OBJECT
public:
    explicit SingleVideoET(QWidget *parent = nullptr);
    virtual ~SingleVideoET();

private slots:
    /**
     * @brief Opens QFileDialog so a user can choose a video for analysis.
     *
     * The function also checks if *.dat file for chosen video exists and tries to load info about cutouts. It also helps to
     * check if a referential frame is known.
     * @sa processFilePath(QString i_wholePaht, QString& o_folder, QString& o_onlyFilename, QString& o_suffix)
     * @sa evaluateVideoImageInput(QString i_path)
     */
    void on_chooseVideoPB_clicked();
    /**
     * @brief Called when a user changes the name of the video in the QLineEdit widget.
     * @param arg1 - new video name
     */
    void on_chosenVideoLE_textChanged(const QString &arg1);
    /**
     * @brief Starts E+T analysis.
     */
    void on_calculateET_clicked();
    /**
     * @brief Displays computed results.
     */
    void on_showGraphET_clicked();

    /**
     * @brief Display the name of actually processed video.
     * @param index - index of the video in the list
     */
    void newVideoProcessed(int index);

    /**
     * @brief Display the name of actually processed calculation.
     * @param method - informs about the progress
     */
    void movedToMethod(int method);

    /**
     * @brief When one of thread is unexpectedly terminated, this function is called to terminate all calculations.
     * @param videoIndex - unused
     * @param errorType - "hardError" expected
     */
    void onUnexpectedTermination(int videoIndex, QString errorType);

    /**
     * @brief Calls protected done() function and another thread is initiated.
     */
    void onDone(int thread);

    /**
     * @brief Saves complete calculated information.
     */
    void on_savePB_clicked();

    /**
     * @brief Handles the command to save data if it comes from the GraphET class object.
     * @param i_videoName - video name to identify data to be saved
     * @param i_object - JSON object with information changed in the GraphET class object
     */
    void onSaveFromGraphET(QString i_videoName, QJsonObject i_object);

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
     * @brief Checks if the path to the video folder exists.
     *
     * If so, the first video from the list of files
     * found in the folder is placed into the QLineEdit widget.
     */
    void checkPaths();
signals:
    /**
     * @brief If the value is added and if it is correct, the signal is emitted and evaluateCorrectValues
     * slot responds.
     * @sa evaluateCorrectValues()
     */
    void checkValuesPass();
private:
    /**
     * @brief Checks if input video or images can be loaded for next processing step.
     * @param i_path
     */
    bool evaluateVideoImageInput(QString i_path);

    /**
     * @brief Checks if userly selected cutout fulfils all necessary criteria to be accepted.
     *
     * Basically, the modified cutout cannot be empty and its width and height must be greated than zero.
     */
    void checkSelectedCutout();

    Ui::SingleVideoET *ui;
    int formerIndex = 0;
    bool runStatus = true;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    bool canProceed,standardLoaded=false,extraLoaded=false;
    QMap<QString,bool> ETthresholdsFound;
    cutoutType selectedCutout = cutoutType::NO_CUTOUT;
};

#endif // SINGLEVIDEOET_H
