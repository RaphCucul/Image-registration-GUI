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
#include <QTableWidgetItem>
#include <QMovie>
#include "util/licovaniparent.h"
#include "licovani/registrationthread.h"

namespace Ui {
class MultiVideoLicovani;
}

class MultiVideoLicovani : public LicovaniParent
{
    Q_OBJECT

public:
    explicit MultiVideoLicovani(QWidget *parent = nullptr);
    ~MultiVideoLicovani();
    void populateMM(QHash<QString, QMap<QString, QVector<double> > > &inputMM);
    void populateMM(QHash<QString,QMap<QString,QVector<int>>>& inputMM);
    void processVideoParameters(QJsonObject& videoData,
                                QHash<QString, QMap<QString, QVector<double> > > &inputMMdouble,
                                QHash<QString,QMap<QString,QVector<int>>>& inputMMint);
    void createAndRunThreads(int indexProcVid, int indexThread, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
    void saveTheResults(QMap<QString,QVector<double>> input,int from, int to);
    int writeToVideo();
    void checkPaths();
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_chooseMultiVPB_clicked();
    void on_chooseFolderPB_clicked();
    void on_deleteChosenPB_clicked();
    void on_registratePB_clicked();
    void on_saveResultsPB_clicked();
    void registrateVideoFrames(int number);
    void addItem(int row,int column, QTableWidgetItem* item);
    void on_listOfVideos_cellDoubleClicked(int row, int column);

private:
    Ui::MultiVideoLicovani *ui;
    RegistrationThread *regThread;
    QHash<QString,QMap<QString,QVector<double>>> videoPropertiesDouble;
    QHash<QString,QMap<QString,QVector<int>>> videoPropertiesInt;
    QMap<int,RegistrationThread*> existingThreads;
    QVector<QVector<int>> threadRange;
    QMovie *movie;
    QTableWidgetItem* progressIconItem;

    double actualFrameCount;
    QString actualVideoName;
    int controlSum = 0;
    int totalSum = 0;
    int internalCounter = 0;
    int TableRowCounter=0;

};

#endif // MULTIVIDEOLICOVANI_H
