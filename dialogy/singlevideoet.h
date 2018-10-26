#ifndef SINGLEVIDEOET_H
#define SINGLEVIDEOET_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QJsonObject>
#include "util/vicevlaknovezpracovani.h"
namespace Ui {
class SingleVideoET;
}

class SingleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit SingleVideoET(QWidget *parent = nullptr);
    ~SingleVideoET();

private slots:
    void on_vyberVidea_clicked();
    void on_vybraneVideo_textChanged(const QString &arg1);
    void on_casovaZnacka_stateChanged(int arg1);
    void on_svetelnaAnomalie_stateChanged(int arg1);
    void on_vypocetET_clicked();
    void on_zobrazGrafET_clicked();
    void zpracovano();
    void on_pushButton_clicked();

    void on_oblastMaxima_textChanged(const QString &arg1);

    void on_uhelRotace_textChanged(const QString &arg1);

    void on_pocetIteraci_textChanged(const QString &arg1);

    void on_cisloReferencnihosnimku_textChanged(const QString &arg1);

private:
    Ui::SingleVideoET *ui;
    VicevlaknoveZpracovani* vlaknoET;
    double pocetSnimkuVidea;
    int analyzaETdokoncena = 0;
    QVector<QString> vybraneVideoETSingle;
    bool spravnostVideaETSingle = false;
    bool volbaCasZnackyETSingle = false;
    bool volbaSvetAnomETSingle = false;
    QVector<double> aktualniEntropie, aktualniTennengrad;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    /// Parametries of anomalies
    /// Parametries for Frangi filter and phase correlation
    int iterace = -1;
    double oblastMaxima = -1;
    double uhel = 0.1;
    QVector<double> parametry_frangi;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_casZnac;
    cv::Point3d maximum_frangi;
    int predchozi_index = 0;
    QJsonObject parametryFrangiJson;
    int cisloReference;
};

#endif // SINGLEVIDEOET_H
