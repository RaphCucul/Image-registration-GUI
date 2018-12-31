#ifndef LICOVANIPARENT_H
#define LICOVANIPARENT_H

#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QProgressBar>
#include <QMap>
#include <QString>
#include <QLabel>
#include <QPointer>

class LicovaniParent : public QWidget
{
    Q_OBJECT
public:
    explicit LicovaniParent(QWidget *parent = nullptr);
signals:

public slots:

protected:
    int licovaniDokonceno = 0;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<int>> snimkyDruheRozhodovaniKomplet;
    QVector<QVector<int>> snimkyOhodnoceniKomplet;
    QVector<QVector<double>> snimkyFrangiX;
    QVector<QVector<double>> snimkyFrangiY;
    QVector<QVector<double>> snimkyFrangiEuklid;
    QVector<QVector<double>> snimkyPOCX;
    QVector<QVector<double>> snimkyPOCY;
    QVector<QVector<double>> snimkyUhel;

    QVector<int> PritomnostCasoveZnacky;
    QVector<int> PritomnostSvetelneAnomalie;
    QJsonObject parametryFrangiJS;
    QVector<double> parametryFrangi;
    QStringList seznamVidei;
    QStringList videoListFull;
    QStringList videoListNames;

    bool canceled;
    int processedVideoNo = 0;

   /* QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiX","FrangiEuklid","POCX","POCY",
                             "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad"
                             "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};*/
    QStringList videoParameters = {"FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","VerticalAnomaly","HorizontalAnomaly"};
    QMap<QString,QVector<QVector<double>>> videoParametersDouble;
    QMap<QString,QVector<QVector<int>>> videoParametersInt;
    QMap<QString,QVector<int>> videoAnomalies;
};

#endif // LICOVANIPARENT_H
