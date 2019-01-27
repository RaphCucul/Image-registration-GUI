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
    void on_vyberVidea_clicked();
    void on_vybraneVideo_textChanged(const QString &arg1);
    void on_casovaZnacka_stateChanged(int arg1);
    void on_svetelnaAnomalie_stateChanged(int arg1);
    void on_vypocetET_clicked();
    void on_zobrazGrafET_clicked();
    void zpracovano(int dokonceno);
    void newVideoProcessed(int index);
    void movedToMethod(int metoda);
    void on_ulozeni_clicked();
    void on_oblastMaxima_textChanged(const QString &arg1);
    void on_uhelRotace_textChanged(const QString &arg1);
    void on_pocetIteraci_textChanged(const QString &arg1);
    void on_cisloReferencnihosnimku_textChanged(const QString &arg1);

private:
    Ui::SingleVideoET *ui;
    int formerIndex = 0;
    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;
    QMap<QString,QVector<int>> mapAnomalies;
    QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiX","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad"
                                 "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};
};

#endif // SINGLEVIDEOET_H
