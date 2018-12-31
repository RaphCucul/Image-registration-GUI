#ifndef SINGLEVIDEOET_H
#define SINGLEVIDEOET_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QJsonObject>
#include <QMap>

#include "multithreadET/qThreadFirstPart.h"
#include "multithreadET/qthreadsecondpart.h"
#include "multithreadET/qthreadthirdpart.h"
#include "multithreadET/qthreadfourthpart.h"
#include "multithreadET/qthreadfifthpart.h"
namespace Ui {
class SingleVideoET;
}

class SingleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit SingleVideoET(QWidget *parent = nullptr);
    ~SingleVideoET();    
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
    void terminatedByError(int where);
    void on_ulozeni_clicked();
    void on_oblastMaxima_textChanged(const QString &arg1);
    void on_uhelRotace_textChanged(const QString &arg1);
    void on_pocetIteraci_textChanged(const QString &arg1);
    void on_cisloReferencnihosnimku_textChanged(const QString &arg1);

private:
    Ui::SingleVideoET *ui;
    qThreadFirstPart* TFirstP;
    qThreadSecondPart* TSecondP;
    qThreadThirdPart* TThirdP;
    qThreadFourthPart* TFourthP;
    qThreadFifthPart* TFifthP;
    double pocetSnimkuVidea;
    int analyzaETdokoncena = 0;
    QStringList analysedVideos;
    QVector<QString> vybraneVideoETSingle;
    bool spravnostVideaETSingle = false;
    bool volbaCasZnackyETSingle = false;
    bool volbaSvetAnomETSingle = false;
    QVector<double> aktualniEntropie, aktualniTennengrad;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<int>> snimkyDruheRozhodovaniKomplet;
    QVector<QVector<int>> snimkyOhodnoceniKomplet;
    QVector<QVector<int>> spatneSnimkyKomplet;
    QVector<QVector<double>> snimkyFrangiX;
    QVector<QVector<double>> snimkyFrangiY;
    QVector<QVector<double>> snimkyFrangiEuklid;
    QVector<QVector<double>> snimkyPOCX;
    QVector<QVector<double>> snimkyPOCY;
    QVector<QVector<double>> snimkyUhel;
    QVector<int> snimkyReferencni;
    QVector<double> prumerneRKomplet;
    QVector<double> prumerneFWHMKomplet;
    /// Parametries of anomalies
    /// Parametries for Frangi filter and phase correlation
    int iterace = -1;
    double oblastMaxima = -1;
    double uhel = 0.1;
    QVector<double> parametry_frangi;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_casZnac;
    cv::Rect ziskany_VK_standard;
    cv::Rect ziskany_VK_extra;
    cv::Point3d maximum_frangi;
    int predchozi_index = 0;
    QJsonObject parametryFrangiJson;
    QJsonObject videoParametersJson;
    int cisloReference;
    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;
    QMap<QString,QVector<float>> mapAnomalies;
    QVector<float> PritomnostCasoveZnacky;
    QVector<float> PritomnostSvetelneAnomalie;
    QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiX","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad"
                                 "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};
};

#endif // SINGLEVIDEOET_H
