#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

#include "util/souborove_operace.h"
using namespace std;

void analyzuj_jmena_souboru_avi(QString vybrana_cesta_k_souborum,
                                QStringList &seznam_jmen_souboru,
                                int &celkovy_pocet_souboru_s_koncovkou,
                                QString koncovka_co_hledam)
{
     QDir chosenDirectory(vybrana_cesta_k_souborum);
     seznam_jmen_souboru = chosenDirectory.entryList(QStringList() << "*."+koncovka_co_hledam << "*."+koncovka_co_hledam.toUpper(),QDir::Files);
     celkovy_pocet_souboru_s_koncovkou = seznam_jmen_souboru.size();
}

void zpracujJmeno(QString& celeJmeno,QString& slozka,QString& zkraceneJmeno,QString& koncovka)
{
    int lastindexSlash = celeJmeno.lastIndexOf("/");
    int lastIndexDot = celeJmeno.length() - celeJmeno.lastIndexOf(".");
    slozka = celeJmeno.left(lastindexSlash);
    zkraceneJmeno = celeJmeno.mid(lastindexSlash+1,
         (celeJmeno.length()-lastindexSlash-lastIndexDot-1));
    koncovka = celeJmeno.right(lastIndexDot-1);
}

QJsonObject readJson(QFile& soubor)
{
    QByteArray val;
    soubor.open(QIODevice::ReadOnly | QIODevice::Text);
    val = soubor.readAll();
    soubor.close();
    //qDebug() << val;
    QJsonDocument d = QJsonDocument::fromJson(val);
    QJsonObject sett2 = d.object();
    //qDebug()<<sett2;
    QJsonArray value = sett2["cestaKvideim"].toArray();
    //qDebug()<<value.size()<<value[0].toString();
    //QJsonValue value2 = value[0].toValue();
    return sett2;
}

void writeJson(QJsonObject &object, QJsonArray &pole, QString typ, QString pathAndName)
{
    object[typ] = pole;
    QJsonDocument document;
    document.setObject(object);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName(pathAndName);
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

QJsonArray vector2array(QVector<double> &vektor)
{
    QJsonArray pole;
    copy(vektor.begin(), vektor.end(), back_inserter(pole));
    return pole;
}

QJsonArray vector2array(QVector<int>& vektor)
{
    QJsonArray pole;
    copy(vektor.begin(), vektor.end(), back_inserter(pole));
    return pole;
}
