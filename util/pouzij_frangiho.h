#ifndef POUZIJ_FRANGIHO_H_INCLUDED
#define POUZIJ_FRANGIHO_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "frangi.h"
#include "upravy_obrazu.h"
#include <QVector>

cv::Mat predzpracovani_obrazu(const cv::Mat &obrazek_vstupni, float sigma_s, float sigma_r);
/// medianova a rekurzivni filtrace

void osetreni_okraju_snimku(cv::Mat &vstupni_obraz,int typ_snimku, int velikost_okraje_r,int velikost_okraje_s);
/// prumerovani okraju

void nulovani_okraju(cv::Mat &vstupni_obraz, int typ_snimku, int velikost_okraje_r,int velikost_okraje_s);

cv::Point3d frangi_analyza(const cv::Mat vstupni_snimek,
                           int mod_zpracovani,
                           int presnost,
                           int zobraz_vysledek_detekce,
                           std::string jmeno_okna,
                           int typ_snimku,
                           bool pritomnost_casove_znamky,
                           cv::Point3d mira_posunuti,
                           QVector<double> parametryFF);
/// funkce provadejici kompletni segmentaci cev s hledanim maxima frangiho funkce
cv::Point2d vypocet_teziste_frangi(const cv::Mat &frangi,
                                   const double& maximum_frangi,
                                   const cv::Point& souradnice_maxima_frangi);
/// vymezi prostor okolo nalezeneho maxima a vypocita subpixelove presnou pozici frangiho maxima
#endif // POUZIJ_FRANGIHO_H_INCLUDED
