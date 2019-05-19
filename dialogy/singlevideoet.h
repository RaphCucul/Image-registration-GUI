#ifndef SINGLEVIDEOET_H
#define SINGLEVIDEOET_H

#include "util/etanalysisparent.h"
#include "dialogy/errordialog.h"

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QJsonObject>
#include <QMap>

namespace Ui {
class SingleVideoET;
}

class SingleVideoET : public ETanalysisParent
{
    Q_OBJECT
public:
    explicit SingleVideoET(QWidget *parent = nullptr);
    virtual ~SingleVideoET();

private slots:
    void on_chooseVideoPB_clicked();
    void on_chosenVideoLE_textChanged(const QString &arg1);
    void on_horizontalAnomalyCB_stateChanged(int arg1);
    void on_verticalAnomalyCB_stateChanged(int arg1);
    void on_calculateET_clicked();
    void on_showGraphET_clicked();

    /**
     * @brief Display the name of actually processed video.
     * @param index
     */
    void newVideoProcessed(int index);

    /**
     * @brief Display the name of actually processed calculation.
     * @param method
     */
    void movedToMethod(int method);

    /**
     * @brief When one of thread is unexpectedly terminated, this function is called to raise an error message
     * and to terminate all calculations.
     * @param message
     * @param threadNumber
     * @param errorType
     */
    void onUnexpectedTermination(int videoIndex, QString errorType);

    /**
     * @brief Function calls protected done() function and another thread is initiated.
     */
    void onDone(int thread);

    void on_savePB_clicked();
    void on_areaMaximum_editingFinished();
    void on_rotationAngle_editingFinished();
    void on_iterationCount_editingFinished();

    /**
     * @brief Function enables the main push button if all parameters - area, iteration and angle - are set
     * correctly.
     */
    void evaluateCorrectValues();

    /**
     * @brief Function creates a new dialog window with the referencial image as a content - clicking into the image
     * marks new upper/lower or left/right border and corresponding extra cutout is generated later in algorithm.
     */
    void showDialog();

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
signals:
    /**
     * @brief If the value is added and if it is correct, the signal is emitted and evaluateCorrectValues
     * slot responds.
     */
    void checkValuesPass();
private:
    Ui::SingleVideoET *ui;
    int formerIndex = 0;
    bool runStatus = true;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // SINGLEVIDEOET_H
