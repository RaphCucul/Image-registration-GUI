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
    void analyzaFWHM(cv::VideoCapture& capture,
                       int referencni_snimek_cislo,
                       int pocet_snimku_videa,
                       bool zmena_meritka,
                       double& vypocteneR,
                       double& vypocteneFWHM,
                       cv::Rect& vyrez_oblasti_standardni,
                       cv::Rect& vyrez_oblasti_navic,
                       QVector<double>& spatne_snimky_komplet);
    void rozhodovani_prvni(QVector<double>& spatne_snimky_prvotni_ohodnoceni,
                           QVector<double>& hodnoceni_vsech_snimku_videa,
                           QVector<double>& POC_x,
                           QVector<double>& POC_y,
                           QVector<double>& uhel,
                           QVector<double>& frangi_x,
                           QVector<double>& frangi_y,
                           QVector<double>& frangi_euklid,
                           double prumerny_korelacni_koeficient,
                           double prumerne_FWHM,
                           cv::VideoCapture& cap,
                           cv::Mat& referencni_snimek,
                           cv::Rect& vyrez_korelace_standardni,
                           cv::Rect& vyrez_korelace_navic,
                           bool zmena_velikosti_obrazu,
                           QVector<double>& snimky_k_provereni_prvni,
                           QVector<double>& vypoctene_hodnoty_R,
                           QVector<double>& vypoctene_hodnoty_FWHM);

    void rozhodovani_druhe(QVector<double>& snimky_k_provereni_prvni,
                           QVector<double>& hodnoceni_vsech_snimku_videa,
                           QVector<double>& vypoctene_hodnoty_R,
                           QVector<double>& vypoctene_hodnoty_FWHM,
                           QVector<double>& POC_x,
                           QVector<double>& POC_y,
                           QVector<double>& uhel,
                           QVector<double>& frangi_x,
                           QVector<double>& frangi_y,
                           QVector<double>& frangi_euklid,
                           double prumerny_korelacni_koeficient,
                           double prumerne_FWHM,
                           QVector<double>& snimky_k_provereni_druhy);
    void rozhodovani_treti(cv::Mat& obraz,
                           cv::Rect& vyrez_korelace_navic,
                           cv::Rect& vyrez_korelace_standardni,
                           QVector<double>& frangi_x,
                           QVector<double>& frangi_y,
                           QVector<double>& frangi_euklid,
                           QVector<double>& POC_x,
                           QVector<double>& POC_y,
                           QVector<double>& uhel,
                           bool zmena_velikosti_snimku,
                           bool casova_znamka,
                           cv::VideoCapture& cap,
                           QVector<double>& hodnoceni_vsech_snimku_videa,
                           QVector<double>& snimky_k_provereni_druhy,
                           QVector<double>& parametryFrangianalyzy);
private:
    void run();
    QVector<QVector<double>> entropieKomplet;
    QVector<QVector<double>> tennengradKomplet;
    QVector<QVector<double>> snimkyPrvotniOhodnoceniKomplet;
    QVector<QVector<double>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<double>> snimkyDruheRozhodovaniKomplet;
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
    void NameOfAnalysedVideo(QString);
};

#endif // VICEVLAKNOVEZPRACOVANI_H
