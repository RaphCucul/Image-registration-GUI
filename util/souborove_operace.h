#ifndef SOUBOROVE_OPERACE_H_INCLUDED
#define SOUBOROVE_OPERACE_H_INCLUDED

#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonArray>
#include <QVector>
using namespace std;
void analyzuj_jmena_souboru_avi(QString vybrana_cesta_k_souborum,
                                QStringList &seznam_jmen_souboru,
                                int &celkovy_pocet_souboru_s_koncovkou,
                                QString koncovka_co_hledam);
/// funkce analyzujici obsah slozky a hledajici soubory se zadanou koncovkou, vyuziva nize uvedene funkce
void zpracujJmeno(QString& celeJmeno,QString& slozka,QString& zkraceneJmeno,QString& koncovka);
QJsonObject readJson(QFile& soubor);
void writeJson(QJsonObject& object, QJsonArray& pole, QString typ, QString pathAndName);
QJsonArray vector2array(QVector<double>& vektor);
#endif // SOUBOROVE_OPERACE_H_INCLUDED
