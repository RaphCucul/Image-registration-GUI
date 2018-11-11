#ifndef QTHREADFIRSTPART_H
#define QTHREADFIRSTPART_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>

class qThreadFirstPart : public QThread
{
    Q_OBJECT
public:
   explicit qThreadFirstPart(QStringList videaKanalyza, cv::Point2f souradniceAnomalie,
                                   cv::Point2f souradniceCasZnac,QVector<double>PF,
                                   bool An, bool CZ, QObject* parent=nullptr);
    QVector<QVector<double>> vypocitanaEntropie();
    QVector<QVector<double>> vypocitanyTennengrad();
    QVector<QVector<int>> vypocitanePrvotniOhodnoceniEntropie();
    QVector<QVector<int>> vypocitanePrvotniOhodnoceniTennengrad();
    QVector<QVector<int>> vypocitaneKompletniOhodnoceni();
    QVector<QVector<int>> vypocitaneSpatneSnimkyKomplet();
    cv::Rect vypocitanyVKstandard();
    cv::Rect vypocitanyVKextra();
    QVector<int> urceneReferenceVidei();
    /*QVector<QVector<double>> vypocitanePrvniRozhodnuti();
    QVector<QVector<double>> vypocitaneDruheRozhodnuti();
    QVector<QVector<double>> vypocitaneFrangiX();
    QVector<QVector<double>> vypocitaneFrangiY();
    QVector<QVector<double>> vypocitanePOCX();
    QVector<QVector<double>> vypocitanePOCY();
    QVector<QVector<double>> vypocitaneFrangiEuklid();
    QVector<QVector<double>> vypocitanyUhel();
    QVector<QVector<double>> vypocitaneOhodnoceni();*/
    /*void rozhodovani_prvni(QVector<double>& spatne_snimky_prvotni_ohodnoceni,
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
                           QVector<double>& parametryFrangianalyzy);*/
    void run() override;

private:
    QVector<QVector<double>> entropieKomplet;
    QVector<QVector<double>> tennengradKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> snimkyKompletniOhodnoceni;
    QVector<QVector<int>> spatneSnimkyKomplet;
    QVector<int> snimkyReferencni;
    /*QVector<QVector<double>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<double>> snimkyDruheRozhodovaniKomplet;
    QVector<QVector<double>> snimkyOhodnoceniKomplet;
    QVector<QVector<double>> snimkyFrangiX;
    QVector<QVector<double>> snimkyFrangiY;
    QVector<QVector<double>> snimkyFrangiEuklid;
    QVector<QVector<double>> snimkyPOCX;
    QVector<QVector<double>> snimkyPOCY;
    QVector<QVector<double>> snimkyUhel;*/
    QVector<double> parametryFrangi;
    QStringList zpracujVidea;
    int pocetVidei;
    double procento;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_CasZnac;
    cv::Rect ziskany_VK_standard;
    cv::Rect ziskany_VK_extra;
    bool AnomalieBool;
    bool CasZnacBool;

signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void hotovo(int);
    void actualVideo(QString);
    void unexpectedTermination();
};

#endif // QTHREADFIRSTPART_H
