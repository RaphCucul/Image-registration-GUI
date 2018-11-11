#ifndef LICOVANIPARENT_H
#define LICOVANIPARENT_H

#include <QObject>
#include <QWidget>

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
    QVector<QVector<double>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<double>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<double>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<double>> snimkyDruheRozhodovaniKomplet;
    QVector<QVector<double>> snimkyOhodnoceniKomplet;
    QVector<QVector<double>> snimkyFrangiX;
    QVector<QVector<double>> snimkyFrangiY;
    QVector<QVector<double>> snimkyFrangiEuklid;
    QVector<QVector<double>> snimkyPOCX;
    QVector<QVector<double>> snimkyPOCY;
    QVector<QVector<double>> snimkyUhel;

    QVector<double> PritomnostCasoveZnacky;
    QVector<double> PritomnostSvetelneAnomalie;
    QStringList seznamVidei;

};

#endif // LICOVANIPARENT_H
