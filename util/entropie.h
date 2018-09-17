#ifndef ENTROPIE_H_INCLUDED
#define ENTROPIE_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "upravy_obrazu.h"
void vypocet_entropie(cv::Mat &zkoumany_snimek, double &entropie, cv::Scalar &tennengrad);
/// pocita entropii snimku

int entropie_tennengrad_videa(cv::VideoCapture& capture, std::vector<double>& entropie, std::vector<double>& tennengrad);
/// hlavni funkce volajici vypocet_entropie

double frekvence_binu(cv::Mat &vstupni_obraz,int &velikost_histogramu);
/// soucasti vypocet_entropie

#endif // ENTROPIE_H_INCLUDED
