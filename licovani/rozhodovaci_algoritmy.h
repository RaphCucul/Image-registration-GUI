#ifndef ROZHODOVACI_ALGORITMY_H_INCLUDED
#define ROZHODOVACI_ALGORITMY_H_INCLUDED
#include <QVector>
#include <opencv2/opencv.hpp>
/// 3 nize uvedene funkce realizuji 3 faze druhe casti algoritmu
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

#endif // ROZHODOVACI_ALGORITMY_H_INCLUDED
