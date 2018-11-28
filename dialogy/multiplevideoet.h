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

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    ~MultipleVideoET();
    void aktualizujProgBar(int procento);
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_nekolikVideiPB_clicked();
    void on_celaSlozkaPB_clicked();
    void on_ETanalyzaVideiPB_clicked();
    void on_zobrazVysledkyPB_clicked();
    void on_vymazatZVyberuPB_clicked();
    void zpracovano(int dokonceno);
    void newVideoProcessed(int index);
    void movedToMethod(int metoda);
    void terminatedByError(int where);
    void on_ulozeni_clicked();
    void on_oblastMaxima_textChanged(const QString &arg1);
    void on_uhelRotace_textChanged(const QString &arg1);
    void on_pocetIteraci_textChanged(const QString &arg1);

private:
    Ui::MultipleVideoET *ui;
    QStringList sezVid;
    qThreadFirstPart* TFirstP;
    qThreadSecondPart* TSecondP;
    qThreadThirdPart* TThirdP;
    qThreadFourthPart* TFourthP;
    qThreadFifthPart* TFifthP;
    double pocetSnimkuVidea;
    int analyzaETdokoncena = 0;

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
    int cisloReference;
};

#endif // MULTIPLEVIDEOET_H
