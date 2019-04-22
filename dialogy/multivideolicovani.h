#ifndef MULTIVIDEOLICOVANI_H
#define MULTIVIDEOLICOVANI_H

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
#include "util/licovaniparent.h"
#include "licovani/registrationthread.h"
#include "dialogy/errordialog.h"

namespace Ui {
class MultiVideoLicovani;
}

class MultiVideoLicovani : public LicovaniParent
{
    Q_OBJECT

public:
    explicit MultiVideoLicovani(QWidget *parent = nullptr);
    ~MultiVideoLicovani();

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    /**
     * @brief Function deactivates specific widgets when the calculations start.
     */
    //void disableWidgets();

    /**
     * @brief Function re-enables specific widgets when the calculations stop.
     */
    //void enableWidgets();

    void on_chooseMultiVPB_clicked();
    void on_chooseFolderPB_clicked();
    void on_deleteChosenPB_clicked();
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

    void errorHandler(int indexOfThread,QString errorMessage);
    void processAnother(int indexOfThread);
    void on_listOfVideos_cellClicked(int row, int column);

private:
    void processResuluts(int analysedThread);

    void createAndRunThreads(int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
    void processVideoParameters(QJsonObject& videoData,
                                QMap<QString, QVector<double>> &inputMMdouble,
                                QMap<QString, QVector<int>> &inputMMint,
                                QMap<QString, QVector<int>> &inputMManomaly);
    int writeToVideo();
    void saveTheResults(QMap<QString,QVector<double>> input,int from, int to);

    bool checkPath(QString filenameToAnalyse);
    bool checkVideo(cv::VideoCapture& capture);

    void populateProperties(QStringList chosenVideos);

    void displayStatus(QString status);

    void startCalculations(cv::VideoCapture& capture);

    Ui::MultiVideoLicovani *ui;
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

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;

    QHash<QString,QMap<QString,QVector<double>>> videoPropertiesDouble;
    QHash<QString,QMap<QString,QVector<int>>> videoPropertiesInt;
    QHash<QString,QMap<QString,QVector<int>>> videoPropertiesAnomaly;

    QHash<QString,QTableWidget*> videoPool;
    QHash<QString,QString> videoStatus;
    QJsonObject videoParametersJson;
    QStringList columnHeaders = {"X","Y",tr("Angle"),"Status"};
};

#endif // MULTIVIDEOLICOVANI_H
