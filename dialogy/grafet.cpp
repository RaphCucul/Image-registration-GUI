#include "dialogy/grafet.h"
#include "analyza_obrazu/entropie.h"
#include "util/util_grafet.h"
#include "util/souborove_operace.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>
#include <QWidget>
#include <QHBoxLayout>

GrafET::GrafET(QVector<QVector<double>> E, QVector<QVector<double>> T, QStringList jmeno_videa, QWidget *parent) :
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
    //qDebug()<<"Počet snímků prvního videa: "<<entropie[0].length();
    //QVector<double> he = entropie[pocetVidei-1];
    //qDebug()<<"Vektor hodnot: "<<he;
    vyhledatExtremy(entropie,maxEntropie,1,pocetVidei); // maximum 1
    vyhledatExtremy(entropie,minEntropie,2,pocetVidei); // minimum 2
    vyhledatExtremy(tennengrad,maxTennengrad,1,pocetVidei);
    vyhledatExtremy(tennengrad,minTennengrad,2,pocetVidei);
    qDebug()<<"Extremy stanoveny.";
    /// POZOR - JEDNÁ SE O DOČASNÉ ŘEŠENÍ - NUTNO IMPLEMENTOVAT PRVOTNÍ OHODNOCENÍ
    /// nové vstupy - prahy
   vytvoreniPrahu(maxEntropie,horniPrah_entropie,1,1); // typPrahu typExtremu
   vytvoreniPrahu(minEntropie,dolniPrah_entropie,2,1); // práh horní a dolní
   vytvoreniPrahu(maxTennengrad,horniPrah_tennengrad,1,2); // extrém minimum a maximum
   vytvoreniPrahu(minTennengrad,dolniPrah_tennengrad,2,2);
    qDebug()<<"Prahy vytvoreny.";
    /// pro zjednodušení vykreslování se přepočítané hodnoty prahů do standardního rozsahu
    /// ukládájí zde a pak se již jen volají dle potřeby
    standardizaceVektoru(entropie,entropieStandard,maxEntropie,minEntropie,pocetVidei);
    standardizaceVektoru(tennengrad,tennengradStandard,maxTennengrad,minTennengrad,pocetVidei);
    standardizaceVektoru(horniPrah_entropie,horniPrah_entropiePrepocet,maxEntropie,minEntropie,pocetVidei);
    standardizaceVektoru(dolniPrah_entropie,dolniPrah_entropiePrepocet,maxEntropie,minEntropie,pocetVidei);
    standardizaceVektoru(horniPrah_tennengrad,horniPrah_tennengradPrepocet,maxTennengrad,minTennengrad,
                         pocetVidei);
    standardizaceVektoru(dolniPrah_tennengrad,dolniPrah_tennengradPrepocet,maxTennengrad,minTennengrad,
                         pocetVidei);
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
    ui->grafyTBW->setStyleSheet("QTabBar::tab { height: 15px; width: 105px; }");
    for (int b = 0; b < pocetVidei;b++)
    {
        QWidget *WSCustomPlot = new QWidget();
        QCustomPlot* GrafickyObjekt = new QCustomPlot(WSCustomPlot);
        QString slozka,jmeno,koncovka;
        QString celek = JmenoVidea.at(b);
        zpracujJmeno(celek,slozka,jmeno,koncovka);
        ui->grafyTBW->addTab(GrafickyObjekt,jmeno);
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

    ui->grafyTBW->setCurrentIndex(0);
    aktualniIndex = 0;
    pocetSnimkuVidea = entropie[aktualniIndex].length();
    //qDebug()<<entropie[aktualniIndex];
    QWidget* w = ui->grafyTBW->currentWidget();
    QCustomPlot* GrafickyObjekt = qobject_cast<QCustomPlot*>(w);
    ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
    liniePrahu(horniPrah_entropie,HP_entropie,pocetVidei,pocetSnimkuVidea);
    liniePrahu(dolniPrah_entropie,DP_entropie,pocetVidei,pocetSnimkuVidea);
    liniePrahu(horniPrah_tennengrad,HP_tennengrad,pocetVidei,pocetSnimkuVidea);
    liniePrahu(dolniPrah_tennengrad,DP_tennengrad,pocetVidei,pocetSnimkuVidea);
    std::vector<double> snimky(pocetSnimkuVidea);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    snimkyRozsah = QVector<double>::fromStdVector(snimky);
    //qDebug()<<snimkyRozsah;
    inicializujGrafickyObjekt(GrafickyObjekt,entropie[aktualniIndex],tennengrad[aktualniIndex],entropieStandard[aktualniIndex],tennengradStandard[aktualniIndex],
                              HP_entropie[aktualniIndex],DP_entropie[aktualniIndex],
                              HP_tennengrad[aktualniIndex],DP_tennengrad[aktualniIndex],snimkyRozsah);
    AktualniGrafickyObjekt = GrafickyObjekt;
    ui->zobrazGrafE->setChecked(true);
    ui->E_HP->setValue(horniPrah_entropie[aktualniIndex]);
    ui->E_DP->setValue(dolniPrah_entropie[aktualniIndex]);
    ui->T_HP->setValue(horniPrah_tennengrad[aktualniIndex]);
    ui->T_DP->setValue(dolniPrah_tennengrad[aktualniIndex]);
    qDebug()<<ui->grafyTBW->currentIndex();
}

GrafET::~GrafET()
{
    delete ui;
}

void GrafET::vyhledatExtremy(QVector<QVector<double>> &zkoumanyVektor, QVector<double>& zapisSem, int typExtremu,
                             int pocetAnalyzovanychVidei)
{
    for (int a = 0; a < pocetAnalyzovanychVidei; a++)
    {

        if (typExtremu == 1)
        {
            QVector<double> pom =zkoumanyVektor[a];
            QVector<double>::iterator maximumIterator = std::max_element(pom.begin(),pom.end());
            int maximumPosition = std::distance(pom.begin(),maximumIterator);
            zapisSem.push_back(pom[maximumPosition]);
            qDebug()<<"Maximum: "<<pom[maximumPosition];
        }
        else
        {
            QVector<double> pom =zkoumanyVektor[a];
            QVector<double>::iterator minimumIterator = std::min_element(pom.begin(),pom.end());
            int minimumPosition = std::distance(pom.begin(),minimumIterator);
            zapisSem.push_back(pom[minimumPosition]);
            qDebug()<<"Minimum: "<<pom[minimumPosition];
        }
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
                vektorPrahu.push_back(vektorExtremu[a]-0.01);
            if (typPrahu == 2) // dolní
                vektorPrahu.push_back(vektorExtremu[a]+0.01);
        }
        if (typExtremu == 2) // tennengrad
        {
            if (typPrahu == 1) // horní
                vektorPrahu.push_back(vektorExtremu[a]-5);
            if (typPrahu == 2) // dolní
                vektorPrahu.push_back(vektorExtremu[a]+5);
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
void GrafET::liniePrahu(QVector<double>& jednotliveHodnotyVidei, QVector<QVector<double>>& vektorPrahoveHodnoty, int pctVid,
                int pctSnVid)
{
    for (int a = 0; a < pctVid; a++)
    {
        QVector<double> pom;
        pom.fill(jednotliveHodnotyVidei[a],pctSnVid);
        vektorPrahoveHodnoty.push_back(pom);
    }
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
        qDebug()<<ui->grafyTBW->currentIndex();
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
        ui->E_HP->setValue(horniPrah_entropie[aktualniIndex]);
        ui->E_DP->setValue(dolniPrah_entropie[aktualniIndex]);
        ui->T_HP->setValue(horniPrah_tennengrad[aktualniIndex]);
        ui->T_DP->setValue(dolniPrah_tennengrad[aktualniIndex]);
        qDebug()<<ui->grafyTBW->currentIndex();
    }
}

void GrafET::ZE()
{
    int aktualniIndex = ui->grafyTBW->currentIndex();
    if(ui->zobrazGrafE->isChecked()==true && ui->zobrazGrafT->isChecked() == false)
    {
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minEntropie[aktualniIndex]-0.5,maxEntropie[aktualniIndex]+0.5);
        AktualniGrafickyObjekt->graph(0)->setVisible(true); // zviditelňuji graf entropie

        AktualniGrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true)
    {
        AktualniGrafickyObjekt->graph(0)->setVisible(false);
        AktualniGrafickyObjekt->graph(2)->setVisible(false); // standardy E & T
        AktualniGrafickyObjekt->graph(3)->setVisible(false);
        ui->E_HPzobraz->setChecked(false);ui->E_DPzobraz->setChecked(false);
        // měl jsem zaškrtnuté oba grafy, nyní se vracím pouze na tennengrad - mažu evidenci k entropii
        // ale mohl jsem mít zapnuté grafy také pro tennengrad - je potřeba je přepočítat na původní hodnoty
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minTennengrad[aktualniIndex]-1,maxTennengrad[aktualniIndex]+1);
        AktualniGrafickyObjekt->graph(1)->setVisible(true); // zviditelňuji graf tennengradu

        HP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
        HP_tennengrad[aktualniIndex].fill(horniPrah_tennengrad[aktualniIndex],pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
        DP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
        DP_tennengrad[aktualniIndex].fill(dolniPrah_tennengrad[aktualniIndex],pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);

        // při jakékoliv změně překresluji
        AktualniGrafickyObjekt->replot();
        // znepřístupňuji nabídku pro grafy prahů entropie
        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        //ui->T_HPzobraz->setEnabled(true);
        //ui->T_DPzobraz->setEnabled(true);
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
            HP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            HP_tennengrad[aktualniIndex].fill(horniPrah_tennengradPrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
        }
        if (ui->T_DPzobraz->isChecked())
        {
            DP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            DP_tennengrad[aktualniIndex].fill(dolniPrah_tennengradPrepocet[aktualniIndex],pocetSnimkuVidea);
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
        AktualniGrafickyObjekt->graph(4)->setVisible(false);
        AktualniGrafickyObjekt->graph(5)->setVisible(false);
        AktualniGrafickyObjekt->graph(6)->setVisible(false);
        AktualniGrafickyObjekt->graph(7)->setVisible(false);
        AktualniGrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(false);ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(false);ui->T_DPzobraz->setEnabled(false);
        ui->E_DPzobraz->setChecked(false);ui->E_HPzobraz->setChecked(false);
        ui->T_HPzobraz->setChecked(false);ui->T_DPzobraz->setChecked(false);
    }
}

void GrafET::ZT()
{
    int aktualniIndex = ui->grafyTBW->currentIndex();
    if(ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(2)->setVisible(false); // standardy
        AktualniGrafickyObjekt->graph(3)->setVisible(false);
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minEntropie[aktualniIndex]-0.5,maxEntropie[aktualniIndex]+0.5);
        AktualniGrafickyObjekt->graph(0)->setVisible(true);
        if (ui->E_HPzobraz->isChecked() == true)
        {
            HP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            HP_entropie[aktualniIndex].fill(horniPrah_entropie[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            DP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            DP_entropie[aktualniIndex].fill(dolniPrah_entropie[aktualniIndex],pocetSnimkuVidea);
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
            HP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            HP_entropie[aktualniIndex].fill(horniPrah_entropiePrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(4)->setVisible(true);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            DP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            DP_entropie[aktualniIndex].fill(dolniPrah_entropiePrepocet[aktualniIndex],pocetSnimkuVidea);
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
    double aktualHodnotaEHP = ui->E_HP->value();
    qDebug()<<"Aktualni hodnota EHP: "<<aktualHodnotaEHP;
    int aktualniIndex = ui->grafyTBW->currentIndex();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        if (std::abs(aktualHodnotaEHP-horniPrah_entropie[aktualniIndex])>0.005)
        {
            HP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            horniPrah_entropie[aktualniIndex] = aktualHodnotaEHP;
            HP_entropie[aktualniIndex].fill(horniPrah_entropie[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(4)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {

            AktualniGrafickyObjekt->graph(4)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }

    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        if (std::abs(aktualHodnotaEHP-horniPrah_entropie[aktualniIndex])>0.005)
        {

            HP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            horniPrah_entropiePrepocet[aktualniIndex] = (aktualHodnotaEHP-minEntropie[aktualniIndex])/(maxEntropie[aktualniIndex]-minEntropie[aktualniIndex]);
            qDebug()<<"EHP prepocet: "<<horniPrah_entropiePrepocet[aktualniIndex];
            HP_entropie[aktualniIndex].fill(horniPrah_entropiePrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(4)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            HP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            HP_entropie[aktualniIndex].fill(horniPrah_entropiePrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(4)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
    }    
    else if (ui->E_HPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(4)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

void GrafET::EDPZ()
{
    double aktualniHodnotaEDP = ui->E_DP->value();
    int aktualniIndex = ui->grafyTBW->currentIndex();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_DPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf dolního prahu tennengradu.";
        if (std::abs(aktualniHodnotaEDP-dolniPrah_entropie[aktualniIndex])>0.005)
        {
            DP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            dolniPrah_entropie[aktualniIndex] = aktualniHodnotaEDP;
            DP_entropie[aktualniIndex].fill(dolniPrah_entropie[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(5)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            AktualniGrafickyObjekt->graph(5)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }

    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_DPzobraz->isChecked() == true)
    {
        if (std::abs(aktualniHodnotaEDP-dolniPrah_entropie[aktualniIndex])>0.005)
        {
            DP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            dolniPrah_entropiePrepocet[aktualniIndex] = (aktualniHodnotaEDP-minEntropie[aktualniIndex])/(maxEntropie[aktualniIndex]-minEntropie[aktualniIndex]);
            DP_entropie[aktualniIndex].fill(dolniPrah_entropiePrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(5)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            DP_entropie[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            DP_entropie[aktualniIndex].fill(dolniPrah_entropiePrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie[aktualniIndex]);
            AktualniGrafickyObjekt->graph(5)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
    }
    else if(ui->E_DPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(5)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

void GrafET::THPZ()
{
    double aktualHodnotaTHP = ui->T_HP->value();
    qDebug()<<"Aktualni hodnota THP: "<<aktualHodnotaTHP;
    int aktualniIndex = ui->grafyTBW->currentIndex();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu tennengradu.";
        if (std::abs(aktualHodnotaTHP-horniPrah_tennengrad[aktualniIndex])>0.005)
        {

            HP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            horniPrah_tennengradPrepocet[aktualniIndex] = (aktualHodnotaTHP-minTennengrad[aktualniIndex])/(maxTennengrad[aktualniIndex]-minTennengrad[aktualniIndex]);
            qDebug()<<"THP prepocet: "<<horniPrah_tennengradPrepocet[aktualniIndex];
            HP_tennengrad[aktualniIndex].fill(horniPrah_tennengradPrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
            AktualniGrafickyObjekt->graph(6)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            HP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            HP_tennengrad[aktualniIndex].fill(horniPrah_tennengradPrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
            AktualniGrafickyObjekt->graph(6)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        if (std::abs(aktualHodnotaTHP-horniPrah_tennengrad[aktualniIndex])>0.005)
        {
            HP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            horniPrah_tennengrad[aktualniIndex] = aktualHodnotaTHP;
            HP_tennengrad[aktualniIndex].fill(horniPrah_tennengrad[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad[aktualniIndex]);
            AktualniGrafickyObjekt->graph(6)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            AktualniGrafickyObjekt->graph(6)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
    }
    else if (ui->T_HPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(6)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

void GrafET::TDPZ()
{
    double aktualniHodnotaTDP = ui->T_DP->value();
    int aktualniIndex = ui->grafyTBW->currentIndex();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        if (std::abs(aktualniHodnotaTDP-dolniPrah_tennengrad[aktualniIndex])>0.005)
        {
            DP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            dolniPrah_tennengradPrepocet[aktualniIndex] = (aktualniHodnotaTDP-minTennengrad[aktualniIndex])/(maxTennengrad[aktualniIndex]-minTennengrad[aktualniIndex]);
            DP_tennengrad[aktualniIndex].fill(dolniPrah_tennengradPrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
            AktualniGrafickyObjekt->graph(7)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            DP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            DP_tennengrad[aktualniIndex].fill(dolniPrah_tennengradPrepocet[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
            AktualniGrafickyObjekt->graph(7)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        if (std::abs(aktualniHodnotaTDP-dolniPrah_tennengrad[aktualniIndex])>0.005)
        {
            DP_tennengrad[aktualniIndex].fill(0.0,pocetSnimkuVidea);
            dolniPrah_tennengrad[aktualniIndex] = aktualniHodnotaTDP;
            DP_tennengrad[aktualniIndex].fill(dolniPrah_tennengrad[aktualniIndex],pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad[aktualniIndex]);
            AktualniGrafickyObjekt->graph(7)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
        else
        {
            AktualniGrafickyObjekt->graph(7)->setVisible(true);
            AktualniGrafickyObjekt->replot();
        }
    }
    else if(ui->T_DPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(7)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}
