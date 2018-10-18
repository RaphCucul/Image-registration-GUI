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
   explicit VicevlaknoveZpracovani(QStringList videaKanalyza, QObject* parent=nullptr);
    void vypocet_entropie(cv::Mat &zkoumany_snimek, double &entropie, cv::Scalar &tennengrad);
    double frekvence_binu(cv::Mat &histogram, int &velikost_histogramu);
    QVector<QVector<double>> vypocitanaEntropie();
    QVector<QVector<double>> vypocitanyTennengrad();
private:
    void run();
    QVector<QVector<double>> entropieKomplet;
    QVector<QVector<double>> tennengradKomplet;
    QStringList zpracujVidea;
    double pocetVidei;
    double procento;

signals:
    void percentageCompleted(int);
    void hotovo();
};

#endif // VICEVLAKNOVEZPRACOVANI_H
