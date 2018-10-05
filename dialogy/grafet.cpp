#include "dialogy/grafet.h"
#include "util/entropie.h"
#include "util/util_grafet.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>
#include <QWidget>
#include <QHBoxLayout>

GrafET::GrafET(QVector<QVector<double>> E, QVector<QVector<double>> T, QVector<QString> jmeno_videa, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrafET)
{
    ui->setupUi(this);
    /// jako vstupy jsou definovány vektory vektorů - je tedy nutné nejdříve identifikovat počet
    /// jednotlivých vektorů a pak je jeden po jednom identifikovat
    entropie = E;
    tennengrad = T;
    JmenoVidea = jmeno_videa;

    ui->E_HPzobraz->setEnabled(false);
    ui->E_DPzobraz->setEnabled(false);
    ui->T_HPzobraz->setEnabled(false);
    ui->T_DPzobraz->setEnabled(false);

    /// inicializace parametrů podle vstupních hodnot
    /// hledání extrémů ve vektorech
    pocetVidei = entropie.size();
    qDebug()<<"Počet videí k zobrazení: "<<pocetVidei;
    qDebug()<<"Počet snímků: "<<entropie[0].length();
    //QVector<double> he = entropie[pocetVidei-1];
    //qDebug()<<"Vektor hodnot: "<<he;
    vyhledatExtremy(entropie,maxEntropie,1,pocetVidei); // maximum 1
    vyhledatExtremy(entropie,minEntropie,2,pocetVidei); // minimum 2
    vyhledatExtremy(tennengrad,maxTennengrad,1,pocetVidei);
    vyhledatExtremy(tennengrad,minTennengrad,2,pocetVidei);
    qDebug()<<"Extremy stanoveny.";
    /// POZOR - JEDNÁ SE O DOČASNÉ ŘEŠENÍ - NUTNO IMPLEMENTOVAT PRVOTNÍ OHODNOCENÍ
    /// nové vstupy - prahy
   vytvoreniPrahu(maxEntropie,horniPrah_entropie,1,1);
   vytvoreniPrahu(minEntropie,dolniPrah_entropie,2,1);
   vytvoreniPrahu(maxTennengrad,horniPrah_tennengrad,1,2);
   vytvoreniPrahu(minTennengrad,dolniPrah_tennengrad,2,2);
    qDebug()<<"Prahy vytvoreny.";
    /// pro zjednodušení vykreslování se přepočítané hodnoty prahů do standardního rozsahu
    /// ukládájí zde a pak se již jen volají dle potřeby
    standardizaceVektoru(entropie,entropieStandard,maxEntropie,minEntropie,pocetVidei);
    standardizaceVektoru(tennengrad,tennengradStandard,maxTennengrad,minTennengrad,pocetVidei);
    standardizaceVektoru(horniPrah_entropie,horniPrah_entropiePrepocet,maxEntropie,minEntropie,pocetVidei);
    standardizaceVektoru(dolniPrah_entropie,dolniPrah_entropiePrepocet,maxEntropie,minEntropie,pocetVidei);
    standardizaceVektoru(horniPrah_tennengrad,horniPrah_tennengradPrepocet,maxTennengrad,minTennengrad,
                         pocetSnimkuVidea);
    standardizaceVektoru(dolniPrah_tennengrad,dolniPrah_tennengradPrepocet,maxTennengrad,minTennengrad,
                         pocetSnimkuVidea);
    qDebug()<<"Standardizovano.";
    // vektory entropie a tennengradu normalizované podle odpovídajících maxim
    //entropieStandard.fill(0,pocetSnimkuVidea);
    //tennengradStandard.fill(0,pocetSnimkuVidea);
    //standardizaceVektoruDat(entropieStandard,entropie,minEntropie,maxEntropie);
    //standardizaceVektoruDat(tennengradStandard,tennengrad,minTennengrad,maxTennengrad);*/

    /// generování vektoru 0->počet snímků videa-1 pro osu x
    /// týká se to jak osy x pro snímky, tak pro osy y pro prahové linie
    /*std::vector<double> snimky(pocetSnimkuVidea);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    snimkyRozsah = QVector<double>::fromStdVector(snimky);*/


    /// inicializace QTabWidgetu a vytvoření první záložky
    ui->grafyTBW->setCurrentIndex(-1); // abych při přidání tabu níže neměl rovnou dva taby
    /*QWidget* tabWidget = new QWidget();
    QHBoxLayout* tabLayout = new QHBoxLayout();
    tabLayout->addWidget(GrafickyObjekt);
    tabLayout->addWidget(GrafickyObjekt);
    tabWidget->setLayout(tabLayout);*/

    for (int b = 0; b < pocetVidei;b++)
    {
        QWidget *WSCustomPlot = new QWidget();
        QCustomPlot* GrafickyObjekt = new QCustomPlot(WSCustomPlot);
        ui->grafyTBW->addTab(GrafickyObjekt,JmenoVidea[b]);
    }

    /// propojení všech prvků s funkcemi ovládajícími fungování grafu
    QObject::connect(ui->zobrazGrafE,SIGNAL(stateChanged(int)),this,SLOT(ZE()));
    QObject::connect(ui->zobrazGrafT,SIGNAL(stateChanged(int)),this,SLOT(ZT()));
    QObject::connect(ui->E_HPzobraz,SIGNAL(stateChanged(int)),this,SLOT(EHPZ()));
    QObject::connect(ui->E_DPzobraz,SIGNAL(stateChanged(int)),this,SLOT(EDPZ()));
    QObject::connect(ui->T_HPzobraz,SIGNAL(stateChanged(int)),this,SLOT(THPZ()));
    QObject::connect(ui->T_DPzobraz,SIGNAL(stateChanged(int)),this,SLOT(TDPZ()));
    QObject::connect(ui->E_HP,SIGNAL(valueChanged(double)),this,SLOT(EHPZ()));
    QObject::connect(ui->E_DP,SIGNAL(valueChanged(double)),this,SLOT(EDPZ()));
    QObject::connect(ui->T_HP,SIGNAL(valueChanged(double)),this,SLOT(THPZ()));
    QObject::connect(ui->T_DP,SIGNAL(valueChanged(double)),this,SLOT(TDPZ()));
    QObject::connect(ui->grafyTBW,SIGNAL(currentChanged(int)),this,SLOT(zmenaTabu(int)));

    ui->grafyTBW->setStyleSheet("QTabBar::tab {height: 15px;width: 42px;padding-top:2px;padding-bottom:,2px}");
    ui->grafyTBW->setCurrentIndex(0);
    aktualniIndex = 0;
    pocetSnimkuVidea = entropie[aktualniIndex].length();
    //qDebug()<<entropie[aktualniIndex];
    QWidget* w = ui->grafyTBW->currentWidget();
    QCustomPlot* GrafickyObjekt = qobject_cast<QCustomPlot*>(w);
    ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
    liniePrahu(horniPrah_entropie,HP_entropie,aktualniIndex,pocetSnimkuVidea);
    liniePrahu(dolniPrah_entropie,DP_entropie,aktualniIndex,pocetSnimkuVidea);
    liniePrahu(horniPrah_tennengrad,HP_tennengrad,aktualniIndex,pocetSnimkuVidea);
    liniePrahu(dolniPrah_tennengrad,DP_tennengrad,aktualniIndex,pocetSnimkuVidea);
    std::vector<double> snimky(pocetSnimkuVidea);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    snimkyRozsah = QVector<double>::fromStdVector(snimky);
    //qDebug()<<snimkyRozsah;
    inicializujGrafickyObjekt(GrafickyObjekt,entropie[aktualniIndex],tennengrad[aktualniIndex],entropieStandard[aktualniIndex],tennengradStandard[aktualniIndex],
                              HP_entropie[aktualniIndex],DP_entropie[aktualniIndex],
                              HP_tennengrad[aktualniIndex],DP_tennengrad[aktualniIndex],snimkyRozsah);
    AktualniGrafickyObjekt = GrafickyObjekt;
    ui->zobrazGrafE->setChecked(true);
    qDebug()<<ui->grafyTBW->currentIndex();
}

GrafET::~GrafET()
{
    delete ui;
}

void GrafET::vyhledatExtremy(QVector<QVector<double>>& zkoumanyVektor, QVector<double>& zapisSem, int typExtremu,
                             int pocetAnalyzovanychVidei)
{
    for (int a = 0; a < pocetAnalyzovanychVidei; a++)
    {
        QVector<double> pom =zkoumanyVektor[a];
        if (typExtremu == 1)
            zapisSem.push_back(*std::max_element(pom.begin(),pom.end()));
        else
            zapisSem.push_back(*std::min_element(pom.begin(),pom.end()));
        //qDebug()<<zapisSem;
    }
}
void GrafET::vytvoreniPrahu(QVector<double>& vektorExtremu,QVector<double>& vektorPrahu,int typPrahu,int typExtremu)
{
    int delkaVektoruExtremu = vektorExtremu.length();
    for (int a = 0; a < delkaVektoruExtremu; a++)
    {
        if (typExtremu == 1) // entropie
        {
            if (typPrahu == 1) // horní
                vektorPrahu.push_back(vektorExtremu[a]-0.1);
            if (typPrahu == 2) // dolní
                vektorPrahu.push_back(vektorExtremu[a]+0.1);
        }
        if (typExtremu == 2) // tennengrad
        {
            if (typPrahu == 1) // horní
                vektorPrahu.push_back(vektorExtremu[a]-50);
            if (typPrahu == 2) // dolní
                vektorPrahu.push_back(vektorExtremu[a]+80);
        }
    }
}
void GrafET::standardizaceVektoru(QVector<QVector<double>>& zkoumanyVektor,QVector<QVector<double>>& vektorStandardizovany,
                          QVector<double>& max,QVector<double>& min,int pocetAnalyzovanychVidei)
{
    for (int a = 0; a < pocetAnalyzovanychVidei; a++)
    {
        int pocet_snimku_videa = zkoumanyVektor[a].length();
        QVector<double> pom;
        pom.fill(0.0,pocet_snimku_videa);
        vektorStandardizovany.push_back(pom);
        for (int b = 0; b < pocet_snimku_videa; b++)
        {
            vektorStandardizovany[a][b] = ((zkoumanyVektor[a][b]-min[a])/(max[a]-min[a]));
        }
    }
}
void GrafET::liniePrahu(QVector<double>& jednotliveHodnotyVidei,QVector<QVector<double>>& vektorPrahoveHodnoty,int aktualIndx,
                int pctSnVid)
{
    QVector<double> pom;
    pom.fill(jednotliveHodnotyVidei[aktualIndx],pctSnVid);
    vektorPrahoveHodnoty.push_back(pom);
}
void GrafET::standardizaceVektoru(QVector<double>& zkoumanyVektor,QVector<double>& vektorStandardizovany,
                          QVector<double>& max,QVector<double>& min,int pocetAnalyzovanychVidei)
{
    vektorStandardizovany.fill(0.0,pocetAnalyzovanychVidei);
    for (int a = 0; a < pocetAnalyzovanychVidei; a++)
    {
        vektorStandardizovany[a] = (zkoumanyVektor[a]-min[a])/(max[a]-min[a]);
    }
}
void GrafET::zmenaTabu(int indexTabu)
{
    if (pocetVidei > 1)
    {
        // inicializuji parametry podle aktuálně vybraného videa
        ui->zobrazGrafE->setChecked(false);
        ui->grafyTBW->setCurrentIndex(indexTabu);
        aktualniIndex = indexTabu;
        pocetSnimkuVidea = entropie[aktualniIndex].length();
        std::vector<double> snimky(pocetSnimkuVidea);
        std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
        snimkyRozsah = QVector<double>::fromStdVector(snimky);
        // nastav správně vektor prahových hodnot
        if (aktualniIndex > HP_entropie.size())
        {
            liniePrahu(horniPrah_entropie,HP_entropie,aktualniIndex,pocetSnimkuVidea);
            liniePrahu(dolniPrah_entropie,DP_entropie,aktualniIndex,pocetSnimkuVidea);
            liniePrahu(horniPrah_tennengrad,HP_tennengrad,aktualniIndex,pocetSnimkuVidea);
            liniePrahu(dolniPrah_tennengrad,DP_tennengrad,aktualniIndex,pocetSnimkuVidea);
            qDebug()<<"zmena velikosti vektoru prahu";
        }
        qDebug()<<"nebylo nutne menit velikost";
        // zavolej aktuální widget
        QWidget* w = ui->grafyTBW->currentWidget();
        QCustomPlot* GrafickyObjekt = qobject_cast<QCustomPlot*>(w);
        ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
        inicializujGrafickyObjekt(GrafickyObjekt,entropie[aktualniIndex],tennengrad[aktualniIndex],
                                  entropieStandard[aktualniIndex],tennengradStandard[aktualniIndex],
                                  HP_entropie[aktualniIndex],DP_entropie[aktualniIndex],HP_tennengrad[aktualniIndex],
                                  DP_tennengrad[aktualniIndex],snimkyRozsah);
        AktualniGrafickyObjekt = GrafickyObjekt;

        ui->zobrazGrafE->setChecked(true);
        qDebug()<<ui->grafyTBW->currentIndex();
    }
}

void GrafET::ZE()
{
    if(ui->zobrazGrafE->isChecked()==true && ui->zobrazGrafT->isChecked() == false)
    {
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minEntropie[aktualniIndex]-0.5,maxEntropie[aktualniIndex]+0.5);
        AktualniGrafickyObjekt->graph(0)->setVisible(true); // zviditelňuji graf entropie

        AktualniGrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(false);
        ui->T_DPzobraz->setEnabled(false);
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true)
    {
        AktualniGrafickyObjekt->graph(0)->setVisible(false);
        AktualniGrafickyObjekt->graph(2)->setVisible(false); // standardy
        AktualniGrafickyObjekt->graph(3)->setVisible(false);
        ui->E_HPzobraz->setChecked(false);ui->E_DPzobraz->setChecked(false);
        // měl jsem zaškrtnuté oba grafy, nyní se vracím pouze na tennengrad - mažu evidenci k entropii
        // ale mohl jsem mít zapnuté grafy také pro tennengrad - je potřeba je přepočítat na původní hodnoty
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minTennengrad[aktualniIndex]-1,maxTennengrad[aktualniIndex]+1);
        AktualniGrafickyObjekt->graph(1)->setVisible(true); // zviditelňuji graf tennengradu
        if (ui->T_HPzobraz->isChecked())
        {
            HP_tennengrad.clear();
            HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
        }
        if (ui->T_DPzobraz->isChecked())
        {
            DP_tennengrad.clear();
            DP_tennengrad.fill(dolniPrah_tennengrad,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
        }
        // při jakékoliv změně překresluji
        AktualniGrafickyObjekt->replot();
        // znepřístupňuji nabídku pro grafy prahů entropie
        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true)
    {
        AktualniGrafickyObjekt->graph(0)->setVisible(false); // původní grafy s originílními hodnotami mažu
        AktualniGrafickyObjekt->graph(1)->setVisible(false);
        // musím zavést společné měřítko pro standardizovaná data
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(0,1);
        AktualniGrafickyObjekt->graph(2)->setVisible(true);
        AktualniGrafickyObjekt->graph(3)->setVisible(true);
        // může se stát, že mám prahy pro tennengrad zobrazené => přepočítávám do <0,1>
        if (ui->T_HPzobraz->isChecked())
        {
            HP_tennengrad.clear();
            HP_tennengrad.fill(horniPrah_tennengradPrepocet,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
        }
        if (ui->T_DPzobraz->isChecked())
        {
            DP_tennengrad.clear();
            DP_tennengrad.fill(dolniPrah_tennengradPrepocet,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
        }
        AktualniGrafickyObjekt->replot();
        // povoluji vykreslení prahů pro entropii
        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);        
    }
    else
    {
        AktualniGrafickyObjekt->graph(0)->setVisible(false);
        AktualniGrafickyObjekt->graph(1)->setVisible(false);
        AktualniGrafickyObjekt->graph(2)->setVisible(false);
        AktualniGrafickyObjekt->graph(3)->setVisible(false);
        AktualniGrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(false);ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(false);ui->T_DPzobraz->setEnabled(false);
        ui->E_DPzobraz->setChecked(false);ui->E_HPzobraz->setChecked(false);
        ui->T_HPzobraz->setChecked(false);ui->T_DPzobraz->setChecked(false);
    }
}

void GrafET::ZT()
{
    if(ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(2)->setVisible(false); // standardy
        AktualniGrafickyObjekt->graph(3)->setVisible(false);
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minEntropie[aktualniIndex]-0.5,maxEntropie[aktualniIndex]+0.5);
        AktualniGrafickyObjekt->graph(0)->setVisible(true);
        if (ui->E_HPzobraz->isChecked() == true)
        {
            HP_entropie.clear();
            HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            DP_entropie.clear();
            DP_entropie.fill(dolniPrah_entropie,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
        }

        AktualniGrafickyObjekt->replot();

        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true)
    {
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minTennengrad[aktualniIndex]-1,maxTennengrad[aktualniIndex]+1);
        AktualniGrafickyObjekt->graph(1)->setVisible(true);

        AktualniGrafickyObjekt->replot();

        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true)
    {
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(0,1);
        AktualniGrafickyObjekt->graph(2)->setVisible(true);
        AktualniGrafickyObjekt->graph(3)->setVisible(true);
        AktualniGrafickyObjekt->graph(0)->setVisible(false);
        if (ui->E_HPzobraz->isChecked() == true)
        {
            HP_entropie.clear();
            HP_entropie.fill(horniPrah_entropiePrepocet,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(4)->setVisible(true);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            DP_entropie.clear();
            DP_entropie.fill(dolniPrah_entropiePrepocet,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(5)->setVisible(true);
        }

        AktualniGrafickyObjekt->replot();

        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else
    {
        AktualniGrafickyObjekt->graph(0)->setVisible(false);
        AktualniGrafickyObjekt->graph(1)->setVisible(false);
        AktualniGrafickyObjekt->graph(2)->setVisible(false);
        AktualniGrafickyObjekt->graph(3)->setVisible(false);
        AktualniGrafickyObjekt->replot();
        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
    }
}

void GrafET::EHPZ()
{
    horniPrah_entropie[aktualniIndex] = ui->E_HP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        HP_entropie.clear();
        HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
        AktualniGrafickyObjekt->graph(4)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        HP_entropie.clear();
        HP_entropie.fill(horniPrah_entropiePrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
        AktualniGrafickyObjekt->graph(4)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }    
    else if (ui->E_HPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(4)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

void GrafET::EDPZ()
{
    dolniPrah_entropie[aktualniIndex] = ui->E_DP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_DPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        DP_entropie.clear();
        DP_entropie.fill(dolniPrah_entropie,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
        AktualniGrafickyObjekt->graph(5)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_DPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        DP_entropie.clear();
        DP_entropie.fill(dolniPrah_entropiePrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
        AktualniGrafickyObjekt->graph(5)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if(ui->E_DPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(5)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

void GrafET::THPZ()
{
    horniPrah_tennengrad[aktualniIndex] = ui->T_HP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";        
        HP_tennengrad.clear();
        HP_tennengrad.fill(horniPrah_tennengradPrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
        AktualniGrafickyObjekt->graph(6)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        HP_tennengrad.clear();
        HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
        AktualniGrafickyObjekt->graph(6)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->T_HPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(6)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

void GrafET::TDPZ()
{
    dolniPrah_tennengrad[aktualniIndex] = ui->T_DP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        DP_tennengrad.clear();
        DP_tennengrad.fill(dolniPrah_tennengradPrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
        AktualniGrafickyObjekt->graph(7)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        DP_tennengrad.clear();
        DP_tennengrad.fill(dolniPrah_tennengrad,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
        AktualniGrafickyObjekt->graph(7)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if(ui->E_DPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(7)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}
