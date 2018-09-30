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
    explicit GrafET(QVector<QVector<double>> E, QVector<QVector<double>> T, QString jmeno_videa,QWidget *parent = nullptr);
    ~GrafET();
    void vyhledatExtremy(QVector<QVector<double>>& zkoumanyVektor,QVector<double>& zapisSem,
                         int typExtremu,int pocetAnalyzovanychVidei);
    void vytvoreniPrahu(QVector<double>& vektorExtremu,QVector<double>& vektorPrahu,int typPrahu,int typExtremu);
    void standardizaceVektoru(QVector<QVector<double>>& zkoumanyVektor, QVector<QVector<double>>& vektorStandardizovany,
                              QVector<double>& max, QVector<double>& min, int pocetAnalyzovanychVidei);
    void standardizaceVektoru(QVector<double>& zkoumanyVektor,QVector<double>& vektorStandardizovany,
                              QVector<double>& max,QVector<double>& min,int pocetAnalyzovanychVidei);
private slots:
    void ZE();
    void ZT();
    void EHPZ();
    void EDPZ();
    void THPZ();
    void TDPZ();
    void zmenaTabu(int indexTabu);

private:
    Ui::GrafET *ui;
    QCustomPlot* AktualniGrafickyObjekt;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<double>> entropieStandard;
    QVector<QVector<double>> tennengradStandard;
    QVector<QVector<double>> HP_entropie,DP_entropie,HP_tennengrad,DP_tennengrad;
    QVector<double> snimkyRozsah;
    QString JmenoVidea;
    int pocetSnimkuVidea = 0;
    int pocetVidei = 0;
    int aktualniIndex = 0;
    QVector<double> maxEntropie;
    QVector<double> minEntropie;
    QVector<double> maxTennengrad;
    QVector<double> minTennengrad;
    bool zobrazEntropii = false;
    bool zobrazTennengrad = false;
    QVector<double> horniPrah_entropie;
    QVector<double> horniPrah_entropiePrepocet;
    QVector<double> dolniPrah_entropie;
    QVector<double> dolniPrah_entropiePrepocet;
    QVector<double> horniPrah_tennengrad;
    QVector<double> horniPrah_tennengradPrepocet;
    QVector<double> dolniPrah_tennengrad;
    QVector<double> dolniPrah_tennengradPrepocet;
    bool zobrazHorPra_entropie = false;
    bool zobrazDolPra_entropie = false;
    bool zobrazHorPra_tennengrad = false;
    bool zobrazDolPra_tennengrad = false;
};
#endif // GRAFET_H
