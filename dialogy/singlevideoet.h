#ifndef SINGLEVIDEOET_H
#define SINGLEVIDEOET_H

#include "util/etanalysisparent.h"

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
    //void temp(QStringList pomList);
    void checkPaths();
private slots:
    void on_chooseVideoPB_clicked();
    void on_chosenVideoLE_textChanged(const QString &arg1);
    void on_horizontalAnomalyCB_stateChanged(int arg1);
    void on_verticalAnomalyCB_stateChanged(int arg1);
    void on_calculateET_clicked();
    void on_showGraphET_clicked();
    void done(int done);
    void newVideoProcessed(int index);
    void movedToMethod(int method);
    void on_savePB_clicked();
    void on_areaMaximum_editingFinished();
    void on_rotationAngle_editingFinished();
    void on_iterationCount_editingFinished();
    void on_referencialNumberLE_textChanged(const QString &arg1);
    void evaluateCorrectValues();
    void showDialog();
signals:
    void checkValuesPass();
private:
    Ui::SingleVideoET *ui;
    int formerIndex = 0;
    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;
    QMap<QString,QVector<int>> mapAnomalies;
    QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiX","FrangiEuklid","POCX","POCY",
                                 "Angle","Rating","PrvotOhodEntropie","PrvotOhodTennengrad"
                                 "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};
};

#endif // SINGLEVIDEOET_H
