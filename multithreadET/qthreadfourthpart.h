#ifndef QTHREADFOURTHPART_H
#define QTHREADFOURTHPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

class qThreadFourthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFourthPart(QStringList& sV,
                               QVector<QVector<int>>& sPrvniRozhodovaniK,
                               QVector<QVector<int>>& ohodKomplet,
                               QVector<QVector<double>>& Rproblematickych,
                               QVector<QVector<double>>& FWHMproblematickych,
                               QVector<QVector<double>>& POCX,
                               QVector<QVector<double>>& POCY,
                               QVector<QVector<double>>& U,
                               QVector<QVector<double>>& F_X,
                               QVector<QVector<double>>& F_Y,
                               QVector<QVector<double>>& F_E,
                               QVector<double>& prumeryR,
                               QVector<double>& prumeryFWHM,
                               QObject* parent=nullptr);
    void run() override;
    QVector<QVector<int>> snimkyRozhodovaniDruheKomplet();
    QVector<QVector<int>> snimkyUpdateOhodnoceniKomplet();
    QVector<QVector<double>> snimkyFrangiXestimated();
    QVector<QVector<double>> snimkyFrangiYestimated();
    QVector<QVector<double>> snimkyFrangiEuklidestimated();
    QVector<QVector<double>> snimkyPOCXestimated();
    QVector<QVector<double>> snimkyPOCYestimated();
    QVector<QVector<double>> snimkyUhelestimated();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void hotovo(int);
    void actualVideo(int);
    void unexpectedTermination();
private:
    QStringList seznamVidei;
    QVector<QVector<int>> snimky_k_provereni_prvni,snimkyProvereniDruheKomplet,ohodnoceniSnimkuKomplet;
    QVector<QVector<double>> vypoctene_hodnoty_R,vypoctene_hodnoty_FWHM,POC_x,POC_y,uhel,frangi_x,
    frangi_y,frangi_euklid;
    QVector<double> prumerny_korelacni_koeficient,prumerne_FWHM;
    double pocetVidei;
    double pocetSnimku;
};

#endif // QTHREADFOURTHPART_H
