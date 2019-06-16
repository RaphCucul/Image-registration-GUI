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

class MultiVideoRegistration : public RegistrationParent
{
    Q_OBJECT

public:
    explicit MultiVideoRegistration(QWidget *parent = nullptr);
    virtual ~MultiVideoRegistration() override;

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
signals:
private slots:
    /**
     * @brief Function deactivates specific widgets when the calculations start.
     */
    void disableWidgets();

    /**
     * @brief Function re-enables specific widgets when the calculations stop.
     */
    void enableWidgets();

    void on_chooseMultiVPB_clicked();
    void on_chooseFolderPB_clicked();
    void on_registratePB_clicked();
    void on_saveResultsPB_clicked();

    /**
     * @brief Function adds given string into the corresponding table.
     * @param row
     * @param column
     * @param parameter
     */
    void addItem(int row,int column,QString parameter);

    /**
     * @brief Function adds frame processing status to the table.
     * @param row
     * @param column
     * @param status
     */
    void addStatus(int row, int column, QString status);

    /**
     * @brief Function handles an error from registration threads.
     * @param indexOfThread
     * @param errorMessage
     */
    void errorHandler(int indexOfThread,QString errorMessage);

    /**
     * @brief Function starts to process another video in the video list and calls function to write the
     * registration results into the new video.
     * @param indexOfThread
     */
    void processAnother(int indexOfThread);

    /**
     * @brief Clicking on any row of the QListWidget will show, if possible, the table with computed
     * parameters
     * @param row
     * @param column
     */
    void on_listOfVideos_cellClicked(int row, int column);

    void continueAlgorithm();
    void processWriterError(int errorNumber, VideoWriter* w);
    void processWriterError(QString errorMessage, VideoWriter* w);
    void processSuccess();
private:
    /**
     * @brief Function process computed translations of frames in specific range the thread was
     * initialized for.
     * @param analysedThread
     */
    void processResuluts(int analysedThread);

    /**
     * @brief Function creates registration threads in for loop.
     * @param indexThread
     * @param cap
     * @param lowerLimit
     * @param upperLimit
     */
    void createAndRunThreads(int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);

    /**
     * @brief Function loads data from *.dat files of selected videos.
     * @param videoData
     * @param inputMMdouble
     * @param inputMMint
     * @param inputMManomaly
     */
    void processVideoParameters(QJsonObject& videoData,
                                QMap<QString, QVector<double>> &inputMMdouble,
                                QMap<QString, QVector<int>> &inputMMint,
                                QMap<QString, QVector<int>> &inputMManomaly);

    /**
     * @brief Function apply the results of registration on original frames and produces video with
     * registrated frames. Produced video is not compressed.
     * @return
     */
    void writeToVideo();
    void saveTheResults(QMap<QString,QVector<double>> input,int from, int to);

    bool checkPath(QString filenameToAnalyse);
    bool checkVideo(cv::VideoCapture& capture);

    /**
     * @brief Function calls processVideoParameters() in the for loop for all chosen videos.
     * @param chosenVideos
     */
    void populateProperties(QStringList chosenVideos);

    /**
     * @brief Function is overridden QWidget keyPressEvent function. The reaction on Delete key press
     * is handled here.
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief Helper function for keyPressEvent function handles the deletion of selected QListWidget
     * items.
     */
    void deleteSelectedFiles();

    void displayStatus(QString status);

    void startCalculations(cv::VideoCapture& capture);

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
};

#endif // MULTIVIDEOREGISTRATION_H
