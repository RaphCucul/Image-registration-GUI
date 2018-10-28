#ifndef FAZOVA_KORELACE_FUNKCE_H_INCLUDED
#define FAZOVA_KORELACE_FUNKCE_H_INCLUDED
#include <opencv2/opencv.hpp>

cv::Point3d fk_translace_hann(const cv::Mat& referencni_snimek, const cv::Mat& posunuty);
/// fazova korelace s Hannovym oknem

cv::Point3d fk_translace(const cv::Mat& referencni_snimek, const cv::Mat& posunuty);
/// fazova korelace bez Hannova okna, vstupem nejcastei korelacni vyrezy

cv::Point3d fk_rotace(const cv::Mat& referencni_snimek,
                      const cv::Mat& posunuty,
                      double maximalni_uhel,
                      const double& hodnota_maxima_fk_translace,
                      cv::Point3d& hodnoty_translace);
/// fazova korelace pro zjisteni uhlu rotace
#endif // FAZOVA_KORELACE_FUNKCE_H_INCLUDED
