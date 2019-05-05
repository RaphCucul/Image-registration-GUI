#ifndef SINGLEVIDEOLICOVANI_H
#define SINGLEVIDEOLICOVANI_H

#include "util/licovaniparent.h"
#include "licovani/registrationthread.h"
#include "fancy_staff/sharedvariables.h"
#include "dialogy/errordialog.h"
#include <QWidget>
#include <QVector>
#include <QList>
#include <QJsonObject>
#include <QTableWidget>

namespace Ui {
class SingleVideoLicovani;
}

class SingleVideoLicovani : public LicovaniParent
{
    Q_OBJECT

public:
    explicit SingleVideoLicovani(QWidget *parent = nullptr);
    ~SingleVideoLicovani();
    void processVideoParameters(QJsonObject& videoData);
    bool writeToVideo();

private slots:
    void on_chooseVideoLE_textChanged(const QString &arg1);
    void on_chooseVideoPB_clicked();
    void registrateVideoframes();
    void totalFramesCompleted(int frameCounter);

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

    void errorHandler(int indexOfThread,QString errorMessage);
    void processAnother(int indexOfThread);
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
private:
    /**
     * @brief Function process computed translations of frames in specific range the thread was
     * initialized for.
     * @param analysedThread
     */
    void processResuluts(int analysedThread);

    void populateLists(QVector<QString> _file);

    void createAndRunThreads(int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
    Ui::SingleVideoLicovani *ui;
    QString fullVideoPath;
    QVector<QString> chosenVideo;
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

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // SINGLEVIDEOLICOVANI_H
