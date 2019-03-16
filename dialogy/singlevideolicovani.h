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
    void checkPaths();
    bool writeToVideo();

    void createAndRunThreads(int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
private slots:
    void on_chooseVideoLE_textChanged(const QString &arg1);
    void on_chooseVideoPB_clicked();
    void registrateVideoframes();
    void totalFramesCompleted(int frameCounter);
    //void addItem(int row,int column,QTableWidgetItem* item);
    void addItem(int row,int column,QString parameter);
    void addStatus(int row, int column, QString status);
    void errorHandler(int indexOfThread,QString errorMessage);
    void processAnother(int indexOfThread);
    void on_savePB_clicked();
signals:
    void calculationStarted();
    void calculationStopped();
private:
    void processResuluts(int analysedThread);
    void terminateThreads();
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
    int numberOfThreads = 1;//QThread::idealThreadCount()-1;
    int threadProcessed = 0;

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QHash<int,RegistrationThread*> threadPool;
};

#endif // SINGLEVIDEOLICOVANI_H
