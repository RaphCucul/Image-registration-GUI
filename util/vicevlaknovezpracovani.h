#ifndef VICEVLAKNOVEZPRACOVANI_H
#define VICEVLAKNOVEZPRACOVANI_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>

class VicevlaknoveZpracovani : public QThread
{
    Q_OBJECT
public:
   explicit VicevlaknoveZpracovani(cv::VideoCapture capture, QVector<double>& entropie,
                           QVector<double>& tennengrad,double pocetVidei, QObject* parent=nullptr);
    void vypocet_entropie(cv::Mat &zkoumany_snimek, double &entropie, cv::Scalar &tennengrad);
    double frekvence_binu(cv::Mat &histogram, int &velikost_histogramu);
    QVector<double> vypocitanaEntropie();
    QVector<double> vypocitanyTennengrad();
private:
    void run();
    cv::VideoCapture cap;
    QVector<double> entropieAktual;
    QVector<double> tennengradAktual;
    double pocetzpracovavanychVidei;
signals:
    void percentageCompleted(int);
    void hotovo();
};

#endif // VICEVLAKNOVEZPRACOVANI_H
