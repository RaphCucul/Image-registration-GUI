#ifndef GRAFET_H
#define GRAFET_H

#include <QDialog>
#include <vector>
#include "fancy_staff/qcustomplot.h"

namespace Ui {
class GrafET;
}

class GrafET : public QDialog
{
    Q_OBJECT

public:
    explicit GrafET(QVector<QVector<double>> E, QVector<QVector<double>> T, QVector<QVector<int>> PrOhE,
                    QVector<QVector<int>> PrOhT, QVector<QVector<int> > PrRozh, QVector<QVector<int>> DruhRozh,
                    QVector<QVector<int>> ohodKompl,QStringList jmeno_videa, QWidget *parent = nullptr);
    ~GrafET();
    void vyhledatExtremy(QVector<QVector<double>>& zkoumanyVektor,QVector<double>& zapisSem,
                         int typExtremu,int pocetAnalyzovanychVidei);
    void vytvoreniPrahu(QVector<double>& vektorExtremu,QVector<double>& vektorPrahu,int typPrahu,int typExtremu);
    void standardizaceVektoru(QVector<QVector<double>>& zkoumanyVektor, QVector<QVector<double>>& vektorStandardizovany,
                              QVector<double>& max, QVector<double>& min, int pocetAnalyzovanychVidei);
    void standardizaceVektoru(QVector<double>& zkoumanyVektor,QVector<double>& vektorStandardizovany,
                              QVector<double>& max,QVector<double>& min,int pocetAnalyzovanychVidei);
    void liniePrahu(QVector<double>& jednotliveHodnotyVidei,QVector<QVector<double>>& vektorPrahoveHodnoty,int pctVid,
                    int pctSnVid);
private slots:
    void ZE();
    void ZT();
    void EHPZ();
    void EDPZ();
    void THPZ();
    void TDPZ();
    void zmenaTabu(int indexTabu);
    void prOhE();
    void prOhT();
    void prvHod();
    void druhHod();
    void on_ohodnocKomplet_stateChanged(int arg1);
    void on_IV1_stateChanged(int arg1);
    void on_IV4_stateChanged(int arg1);
    void on_IV5_stateChanged(int arg1);

private:
    Ui::GrafET *ui;
    QCustomPlot* AktualniGrafickyObjekt;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<int>> snimkyDruheRozhodovaniKomplet;
    QVector<QVector<int>> snimkyOhodnoceniKomplet;
    QVector<QVector<double>> entropieStandard;
    QVector<QVector<double>> tennengradStandard;
    QVector<QVector<double>> HP_entropie,DP_entropie,HP_tennengrad,DP_tennengrad;
    QVector<double> snimkyRozsah;
    QStringList JmenoVidea;
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
