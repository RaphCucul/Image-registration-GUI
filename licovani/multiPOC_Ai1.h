#ifndef MULTIPOC_AI1_H_INCLUDED
#define MULTIPOC_AI1_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QCheckBox>
int kompletni_slicovani(cv::VideoCapture& cap,
                        cv::Mat& referencni_snimek,
                        int cislo_posunuty,
                        int& iterace,
                        double& oblastMaxima,
                        double& uhel,
                        cv::Rect& korelacni_vyrez_navic,
                        cv::Rect& korelacni_vyrez_standardni,
                        bool nutnost_zmenit_velikost_snimku,
                        cv::Mat& slicovany_kompletne,
                        cv::Point3d& mira_translace,
                        double& celkovy_uhel);
/// funkce provadejici licovani videa s vicenasobnym pouzitim fazove korelace

void predzpracovaniKompletnihoLicovani(cv::Mat &reference,
                                       cv::Mat &obraz,
                                       QVector<double> &parFrang,
                                       cv::Point2f &hranice_anomalie,
                                       cv::Point3d &fraMax,
                                       cv::Rect &oblastAnomalie,
                                       cv::Rect &vyrezKoreEx,
                                       cv::Rect &vyrezKoreStand,
                                       cv::VideoCapture &cap,
                                       QCheckBox* pritomnostAnomalie,
                                       QCheckBox* casZnacka,
                                       bool &prCasZn,
                                       bool &prSveAn,
                                       bool &zmeMer);
/// funkce provede predzpracovani
#endif // MULTIPOC_AI1_H_INCLUDED
