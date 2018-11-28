#ifndef QTHREADFIFTHPART_H
#define QTHREADFIFTHPART_H

#include <QThread>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>

class qThreadFifthPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadFifthPart(QStringList& sV,
                              cv::Rect& VK_s,
                              cv::Rect& VK_e,
                              QVector<QVector<double>>& POCX,
                              QVector<QVector<double>>& POCY,
                              QVector<QVector<double>>& U,
                              QVector<QVector<double>>& F_X,
                              QVector<QVector<double>>& F_Y,
                              QVector<QVector<double>>& F_E,
                              bool zmenaMeritka,
                              bool casovaZnacka,
                              QVector<QVector<int>> &hodnoceni_vsech_snimku_videa,
                              QVector<QVector<int>>& sProverDruhy,
                              QVector<int>& referencniSnimkyVidei,
                              QVector<double>& parametryFrangianalyzy,
                              QObject* parent = nullptr);
    void run() override;
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
    double pocetVidei;
    double pocetSnimku;
    QStringList seznamVidei;
    QVector<QVector<int>> snimky_k_provereni_druhy,ohodnoceniSnimkuKomplet;
    QVector<QVector<double>> POC_x,POC_y,uhel,frangi_x,frangi_y,frangi_euklid;
    QVector<double> prumerny_korelacni_koeficient,prumerne_FWHM,parametryFrangi;
    QVector<int> referenceKompletni;
    cv::Rect vyrez_korelace_standard;
    cv::Rect vyrez_korelace_extra;
    bool zmena_meritka,casova_znacka;
};

#endif // QTHREADFIFTHPART_H
