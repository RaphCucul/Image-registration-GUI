#ifndef UPRAVY_OBRAZU_H_INCLUDED
#define UPRAVY_OBRAZU_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
cv::Mat translace_snimku(const cv::Mat& posunuty_original,const cv::Point3d& hodnoty_translace,int radky,int sloupce);
/// posunuti snimku podle zadanych parametru

cv::Mat rotace_snimku(const cv::Mat& snimek_po_translaci, const double uhel);
/// rotace snimku podle zadanych parametru

void ukaz_Mat(std::string jmeno_okna, cv::Mat snimek_k_zobrazeni);
/// pro 100% jistotu, ze se Mat soubor zobrazi bez ohledu na jeho typ

void kontrola_typu_snimku_32C1(cv::Mat& snimek_ke_kontrole);
void kontrola_typu_snimku_8C1(cv::Mat& snimek_ke_kontrole);
void kontrola_typu_snimku_8C3(cv::Mat& snimek_ke_kontrole);
cv::Mat kontrola_typu_snimku_64C1(cv::Mat& snimek_ke_kontrole);
/// konverzni funkce pro prevadeni typu Mat promennych pro potreby jednotlivych funkci
#endif // UPRAVY_OBRAZU_H_INCLUDED
