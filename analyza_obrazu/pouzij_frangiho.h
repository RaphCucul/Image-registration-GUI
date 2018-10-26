#ifndef POUZIJ_FRANGIHO_H_INCLUDED
#define POUZIJ_FRANGIHO_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QString>
#include "frangi.h"
#include "upravy_obrazu.h"
#include <QVector>
#include <QJsonObject>

/**
 * @brief predzpracovani_obrazu
 * @param obrazek_vstupni
 * @param sigma_s
 * @param sigma_r
 * @return Image after recursive and median filtration
 */
cv::Mat predzpracovani_obrazu(const cv::Mat &obrazek_vstupni, float sigma_s, float sigma_r);

/**
 * @brief osetreni_okraju_snimku
 * @param vstupni_obraz
 * @param typ_snimku
 * @param velikost_okraje_r
 * @param velikost_okraje_s
 */
void osetreni_okraju_snimku(cv::Mat &vstupni_obraz,int typ_snimku, int velikost_okraje_r,int velikost_okraje_s);

/**
 * @brief nulovani_okraju
 * @param vstupni_obraz
 * @param typ_snimku
 * @param velikost_okraje_r
 * @param velikost_okraje_s
 */
void nulovani_okraju(cv::Mat &vstupni_obraz, int typ_snimku, int velikost_okraje_r,int velikost_okraje_s);

/**
 * @brief frangi_analyza
 * @param vstupni_snimek
 * @param mod_zpracovani
 * @param presnost
 * @param zobraz_vysledek_detekce
 * @param jmeno_okna
 * @param typ_snimku
 * @param pritomnost_casove_znamky
 * @param mira_posunuti
 * @param parametryFF
 * @return Coordinates of pixel with maximal value after Frangi filtration
 */
cv::Point3d frangi_analyza(const cv::Mat vstupni_snimek,
                           int mod_zpracovani,
                           int presnost,
                           int zobraz_vysledek_detekce,
                           QString jmeno_okna,
                           int typ_snimku,
                           bool pritomnost_casove_znamky,
                           cv::Point3d mira_posunuti,
                           QVector<double> parametryFF);

/**
 * @brief vypocet_teziste_frangi
 * @param frangi
 * @param maximum_frangi
 * @param souradnice_maxima_frangi
 * @return Weighted coordinates of maximal value pixel
 */
cv::Point2d vypocet_teziste_frangi(const cv::Mat &frangi,
                                   const double& maximum_frangi,
                                   const cv::Point& souradnice_maxima_frangi);

/**
 * @brief inicializace_frangi_opt
 * @param nactenyObjekt
 * @param parametr
 * @param nacteneParametry
 * @param pozice
 */
void inicializace_frangi_opt(QJsonObject nactenyObjekt, QString parametr, QVector<double>& nacteneParametry,
                             int &pozice);

/**
 * @brief data_z_frangi_opt
 * @param pozice
 * @param nacteneParametry
 * @return
 */
double data_z_frangi_opt(int pozice, QVector<double>& nacteneParametry);

/**
 * @brief velikost_frangi_opt
 * @param velikost
 * @param nacteneParametry
 */
void velikost_frangi_opt(int velikost, QVector<double>& nacteneParametry);
#endif // POUZIJ_FRANGIHO_H_INCLUDED
