#ifndef GRAFET_H
#define GRAFET_H

#include <QDialog>
#include <vector>
#include "qcustomplot.h"

namespace Ui {
class GrafET;
}

class GrafET : public QDialog
{
    Q_OBJECT

public:
    explicit GrafET(std::vector<double> E, std::vector<double> T, QString jmeno_videa,QWidget *parent = nullptr);
    ~GrafET();
private slots:
    void ZE();
    void ZT();
    /*void EHPZ();
    void EDPZ();
    void THPZ();
    void TDPZ();
    void EHP();
    void EDP();
    void THP();
    void TDP();*/
private:
    Ui::GrafET *ui;
    QCustomPlot* GrafickyObjekt;
    QVector<double> entropie;
    QVector<double> tennengrad;
    QVector<double> entropieStandard;
    QVector<double> tennengradStandard;
    QVector<double> HP_entropie,DP_entropie,HP_tennengrad,DP_tennengrad;
    QVector<double> snimkyRozsah;
    QString JmenoVidea;
    int pocetSnimkuVidea = 0;
    double maxEntropie = 0.0;
    double minEntropie = 0.0;
    double maxTennengrad = 0.0;
    double minTennengrad = 0.0;
    bool zobrazEntropii = false;
    bool zobrazTennengrad = false;
    double horniPrah_entropie = 0.0;
    double dolniPrah_entropie = 0.0;
    double horniPrah_tennengrad = 0.0;
    double dolniPrah_tennengrad = 0.0;
    bool zobrazHorPra_entropie = false;
    bool zobrazDolPra_entropie = false;
    bool zobrazHorPra_tennengrad = false;
    bool zobrazDolPra_tennengrad = false;
};

/*class Graf : public QCustomPlot
{
    Q_OBJECT
public:
    Graf(QVector<double> E, QVector<double> T, bool zobrazE, bool zobrazT, double HP_E, double DP_E,
         double HP_T, double DP_T, bool zobrazHP_E, bool zobrazDP_E, bool zobrazHP_T, bool zobrazDP_T,
         QWidget *parent = nullptr);
    ~Graf();
    QVector<double> entropie;
    QVector<double> tennengrad;
    int vyobrazeneGrafy = 0;
};*/

#endif // GRAFET_H
