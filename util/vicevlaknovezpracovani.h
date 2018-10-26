#ifndef VICEVLAKNOVEZPRACOVANI_H
#define VICEVLAKNOVEZPRACOVANI_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>

class VicevlaknoveZpracovani : public QThread
{
    Q_OBJECT
public:
   explicit VicevlaknoveZpracovani(QStringList videaKanalyza, cv::Point2f souradniceAnomalie,
                                   cv::Point2f souradniceCasZnac,QVector<double>PF,
                                   bool An, bool CZ, QObject* parent=nullptr);
    QVector<QVector<double>> vypocitanaEntropie();
    QVector<QVector<double>> vypocitanyTennengrad();
    double analyzaFWHM(cv::VideoCapture& capture,
                       int referencni_snimek_cislo,
                       int pocet_snimku_videa,
                       double& R,
                       bool zmena_meritka,
                       cv::Rect& vyrez_oblasti_standardni,
                       cv::Rect& vyrez_oblasti_navic,
                       QVector<double>& spatne_snimky_komplet);
private:
    void run();
    QVector<QVector<double>> entropieKomplet;
    QVector<QVector<double>> tennengradKomplet;
    QVector<double> parametryFrangi;
    QStringList zpracujVidea;
    double pocetVidei;
    double procento;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_CasZnac;
    bool AnomalieBool;
    bool CasZnacBool;

signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void hotovo();
    void unexpectedTermination();
};

#endif // VICEVLAKNOVEZPRACOVANI_H
