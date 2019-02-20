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
#include "dialogy/errordialog.h"
#include "util/etanalysisparent.h"

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public ETanalysisParent
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    ~MultipleVideoET();
    void aktualizujProgBar(int procento);
    //void checkPaths();
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_afewVideosPB_clicked();
    void on_wholeFolderPB_clicked();
    void on_analyzeVideosPB_clicked();
    void on_showResultsPB_clicked();
    void on_deleteChosenFromListPB_clicked();
    void done(int finished);
    void newVideoProcessed(int index);
    void movedToMethod(int method);
    void on_savePB_clicked();
    void on_areaMaximum_editingFinished();
    void on_rotationAngle_editingFinished();
    void on_iterationCount_editingFinished();
    void evaluateCorrectValues();
signals:
    void checkValuesPass();
private:
    Ui::MultipleVideoET *ui;
    QStringList videoList;
    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;
    QMap<QString,QVector<int>> mapAnomalies;
    QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiX","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad",
                                 "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};
};

#endif // MULTIPLEVIDEOET_H
