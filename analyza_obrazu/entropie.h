#ifndef ENTROPIE_H_INCLUDED
#define ENTROPIE_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <string>
#include "analyza_obrazu/upravy_obrazu.h"
#include <QProgressBar>
#include <QVector>
void vypocet_entropie(cv::Mat &zkoumany_snimek, double &entropie, cv::Scalar &tennengrad);
/// pocita entropii snimku

int entropie_tennengrad_videa(cv::VideoCapture& capture, QVector<double>& entropie,
                              QVector<double>& tennengrad,QProgressBar* progbar);
/// hlavni funkce volajici vypocet_entropie

double frekvence_binu(cv::Mat &vstupni_obraz,int &velikost_histogramu);
/// soucasti vypocet_entropie

void standardizaceVektoruDat(QVector<double>& dataStandardizovana,QVector<double>& dataOriginalni,
                             double minimum,double maximum);
/// zajišťuje přepočet hodnot do rozsahu <0,1>

//void aktualizaceProgressBaru(QProgressBar* pb, int procento);

#endif // ENTROPIE_H_INCLUDED
