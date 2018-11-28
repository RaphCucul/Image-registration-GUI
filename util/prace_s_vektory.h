#ifndef PRACE_S_VEKTORY_H_INCLUDED
#define PRACE_S_VEKTORY_H_INCLUDED
#include <QVector>
#include <opencv2/opencv.hpp>
double median_vektoru_cisel(QVector<double> vektor_hodnot);
/// pocita median z pridelenych hodnot

double kontrola_maxima(QVector<double>& vektor_hodnot);
/// zjistuje, jestli neni maximum prilis velke

QVector<int> spojeni_vektoru(QVector<int>& vektor1,QVector<int>& vektor2);

void okna_vektoru(QVector<double>& vektor_hodnot, QVector<double>& okna, double& zbytek_do_konce);
/// rozdeleni prubehu entropie/tenengradu na okna

QVector<double> mediany_vektoru(QVector<double>& vektor_hodnot,
                                    QVector<double>& vektor_oken,
                                    double zbytek_do_konce);
/// prochazi okna a pocita mediany

void analyza_prubehu_funkce(QVector<double>& vektor_hodnot,
                            QVector<double>& vektor_medianu,
                            QVector<double>& vektor_oken,
                            double& prepocitane_maximum,
                            QVector<double>& prahy,
                            double& tolerance,
                            int& dmin,
                            double& zbytek_do_konce,
                            QVector<int>& spatne_snimky,
                            QVector<double>& pro_provereni);
/// realizuje prvni cast algoritmu

int nalezeni_referencniho_snimku(double& prepocitane_maximum, QVector<double>& pro_provereni,
                                 QVector<double>& vektor_hodnot);
/// hleda nejvhodnejsi snimek k prepocitane hodnote maxima

/*void analyza_FWHM(cv::VideoCapture& capture,
                  int referencni_snimek_cislo,
                  int pocet_snimku_videa,
                  bool zmena_meritka,
                  double& vypocteneR,
                  double& vypocteneFWHM,
                  cv::Rect& vyrez_oblasti_standardni,
                  cv::Rect& vyrez_oblasti_navic,
                  QVector<double>& spatne_snimky_komplet);
/// pocita prumerny R a PMPf*/

void kontrola_celistvosti(QVector<int>& spatne_snimky);
/// kontroluje, jestli v rade vadnych snimku lze doplnit snimek pro kompletaci sekvence (v pripade nekolika vadnych
/// snimku za sebou zabrani preskoceni pripadneho spatneho snimku)

int findReferenceFrame(QVector<int> vectorEvaluation);

QVector<QVector<int>> divideIntoPeaces(int totalLength, int threadCount);
#endif // PRACE_S_VEKTORY_H_INCLUDED
