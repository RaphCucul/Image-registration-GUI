#ifndef KOREKCE_ZAPIS_H_INCLUDED
#define KOREKCE_ZAPIS_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QVector>
void licovani_nejvhodnejsich_snimku(cv::VideoCapture& cap,
                                    cv::VideoWriter& zap,
                                    cv::Mat& referencni_snimek,
                                    cv::Point3d bod_RefS_reverse,
                                    int index_posunuty,
                                    int &iterace,
                                    double &oblastMaxima,
                                    double &uhel,
                                    cv::Rect& vyrez_korelace_extra,
                                    cv::Rect& vyrez_korelace_standard,
                                    bool zmena_meritka,
                                    QVector<double> &parametry_frangi,
                                    );
cv::Mat eventualni_korekce_translace(cv::Mat& slicovany_snimek,cv::Mat& obraz,cv::Rect& vyrez_korelace_standard,
                                     cv::Point3d& korekce_bod,double &oblastMaxima);

#endif // KOREKCE_ZAPIS_H_INCLUDED
