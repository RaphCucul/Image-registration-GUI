#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QJsonObject>
#include <opencv2/opencv.hpp>
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
    void zpracovano();
    void on_ulozeni_clicked();

private:
    Ui::MultipleVideoET *ui;
    QStringList sezVid;

    double pocetSnimkuVidea;
    int analyzaETdokoncena = 0;
    bool spravnostVideaETSingle = false;
    bool volbaCasZnackyETSingle = false;
    bool volbaSvetAnomETSingle = false;

    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<double>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<double>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<double>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<double>> snimkyDruheRozhodovaniKomplet;
    QVector<QVector<double>> snimkyOhodnoceniKomplet;
    QVector<QVector<double>> snimkyFrangiX;
    QVector<QVector<double>> snimkyFrangiY;
    QVector<QVector<double>> snimkyFrangiEuklid;
    QVector<QVector<double>> snimkyPOCX;
    QVector<QVector<double>> snimkyPOCY;
    QVector<QVector<double>> snimkyUhel;
    QVector<QString> videoNames;

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

#endif // MULTIPLEVIDEOET_H
