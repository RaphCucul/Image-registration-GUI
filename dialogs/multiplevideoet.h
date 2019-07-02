#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QJsonObject>
#include <opencv2/opencv.hpp>

#include "multithreadET/qThreadFirstPart.h"
#include "multithreadET/qthreadsecondpart.h"
#include "multithreadET/qthreadthirdpart.h"
#include "multithreadET/qthreadfourthpart.h"
#include "multithreadET/qthreadfifthpart.h"
#include "dialogs/errordialog.h"
#include "util/etanalysisparent.h"

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public ETanalysisParent
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    virtual ~MultipleVideoET() override;
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_afewVideosPB_clicked();
    void on_wholeFolderPB_clicked();
    void on_analyzeVideosPB_clicked();
    void on_showResultsPB_clicked();

    /**
     * @brief Helper function for keyPressEvent function handles the deletion of selected QListWidget
     * items.
     */
    void deleteSelectedFiles();

    /**
     * @brief Slot function where stop and start of individual threads is processed.
     * @param done
     */
    void onDone(int thread);

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
signals:
    /**
     * @brief If the value is added and if it is correct, the signal is emitted and evaluateCorrectValues
     * slot responds.
     */
    void checkValuesPass();
private:
    /**
     * @brief checkVideos
     * @return
     */
    bool checkVideos();

    /**
     * @brief Function is overridden QWidget keyPressEvent function. The reaction on Delete key press
     * is handled here.
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    Ui::MultipleVideoET *ui;
    QStringList videoList;
    bool runStatus = true;
    bool canProceed = true;

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // MULTIPLEVIDEOET_H
