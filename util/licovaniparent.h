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
    QVector<double> PritomnostCasoveZnacky;
    QVector<double> PritomnostSvetelneAnomalie;
    QStringList seznamVidei;
    QVector<QVector<double>> licovaniXsour;
    QVector<QVector<double>> licovaniYsour;
    QVector<QVector<int>> vhodnostSnimku;
    QVector<QVector<double>> frangiXsour;
    QVector<QVector<double>> frangiYsour;

};

#endif // LICOVANIPARENT_H
