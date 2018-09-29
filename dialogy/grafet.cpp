#include "dialogy/grafet.h"
#include "util/entropie.h"
#include "util/util_grafet.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>
#include <QWidget>
#include <QHBoxLayout>

GrafET::GrafET(QVector<double> E, QVector<double> T, QString jmeno_videa, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrafET)
{
    ui->setupUi(this);


    entropie = E;
    tennengrad = T;
    //qDebug()<<entropie<<" "<<tennengrad;
    JmenoVidea = jmeno_videa;
    ui->E_HPzobraz->setEnabled(false);
    ui->E_DPzobraz->setEnabled(false);
    ui->T_HPzobraz->setEnabled(false);
    ui->T_DPzobraz->setEnabled(false);

    /// inicializace parametrů podle vstupních hodnot
    // hledání extrémů ve vektorech
    maxEntropie = *std::max_element(entropie.begin(),entropie.end());
    minEntropie = *std::min_element(entropie.begin(),entropie.end());
    maxTennengrad = *std::max_element(tennengrad.begin(),tennengrad.end());
    minTennengrad = *std::min_element(tennengrad.begin(),tennengrad.end());

    /// POZOR - JEDNÁ SE O DOČASNÉ ŘEŠENÍ - NUTNO IMPLEMENTOVAT PRVOTNÍ OHODNOCENÍ
    horniPrah_entropie = maxEntropie-0.1;
    ui->E_HP->setValue(horniPrah_entropie);
    dolniPrah_entropie = minEntropie+0.1;
    ui->E_DP->setValue(dolniPrah_entropie);
    horniPrah_tennengrad = maxTennengrad-50;
    ui->T_HP->setValue(horniPrah_tennengrad);
    dolniPrah_tennengrad = minTennengrad+80;
    ui->T_DP->setValue(dolniPrah_tennengrad);

    /// pro zjednodušení vykreslování se přepočítané hodnoty prahů do standardního rozsahu
    /// ukládájí zde a pak se již jen volají dle potřeby
    horniPrah_entropiePrepocet = (horniPrah_entropie-minEntropie)/(maxEntropie-minEntropie);
    dolniPrah_entropiePrepocet = (dolniPrah_entropie-minEntropie)/(maxEntropie-minEntropie);
    horniPrah_tennengradPrepocet = (horniPrah_tennengrad-minTennengrad)/(maxTennengrad-minTennengrad);
    dolniPrah_tennengradPrepocet = (dolniPrah_tennengrad-minTennengrad)/(maxTennengrad-minTennengrad);
    pocetSnimkuVidea = entropie.length();
    // vektory entropie a tennengradu normalizované podle odpovídajících maxim
    entropieStandard.fill(0,pocetSnimkuVidea);
    tennengradStandard.fill(0,pocetSnimkuVidea);
    standardizaceVektoruDat(entropieStandard,entropie,minEntropie,maxEntropie);
    standardizaceVektoruDat(tennengradStandard,tennengrad,minTennengrad,maxTennengrad);

    /// generování vektoru 0->počet snímků videa-1 pro osu x
    /// týká se to jak osy x pro snímky, tak pro osy y pro prahové linie
    std::vector<double> snimky(pocetSnimkuVidea);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    snimkyRozsah = QVector<double>::fromStdVector(snimky);
    HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
    DP_entropie.fill(dolniPrah_entropie,pocetSnimkuVidea);
    HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
    DP_tennengrad.fill(dolniPrah_tennengrad,pocetSnimkuVidea);   

    /// inicializace QTabWidgetu a vytvoření první záložky
    //ui->grafyTBW->setCurrentIndex(-1); // abych při přidání tabu níže neměl rovnou dva taby
    /*QWidget* tabWidget = new QWidget();
    QHBoxLayout* tabLayout = new QHBoxLayout();
    tabLayout->addWidget(GrafickyObjekt);
    tabLayout->addWidget(GrafickyObjekt);
    tabWidget->setLayout(tabLayout);*/
    QVector<QString> jmena = {"jedna","dva"};
    for (int b = 0; b < 2;b++)
    {
        QWidget *WSCustomPlot = new QWidget();
        QCustomPlot* GrafickyObjekt = new QCustomPlot(WSCustomPlot);
        ui->grafyTBW->addTab(GrafickyObjekt,jmena[b]);
    }
    //ui->grafyTBW->addTab(tabWidget, "jedna");
    //ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
    //ui->grafyTBW->addTab(tabWidget,"dva");

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

    //ui->grafyTBW->setStyleSheet("QTabBar::tab {height: 15px;width: 15px;padding-top:2px;padding-bottom:,2px}");
    ui->grafyTBW->setCurrentIndex(0);
    QWidget* w = ui->grafyTBW->currentWidget();
    QCustomPlot* GrafickyObjekt = qobject_cast<QCustomPlot*>(w);
    ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
    inicializujGrafickyObjekt(GrafickyObjekt,entropie,tennengrad,entropieStandard,tennengradStandard,
                              HP_entropie,DP_entropie,HP_tennengrad,DP_tennengrad,snimkyRozsah);
    AktualniGrafickyObjekt = GrafickyObjekt;
    ui->zobrazGrafE->setChecked(true);
    qDebug()<<ui->grafyTBW->currentIndex();
}

GrafET::~GrafET()
{
    delete ui;
}

void GrafET::zmenaTabu(int indexTabu)
{
    ui->zobrazGrafE->setChecked(false);
    ui->grafyTBW->setCurrentIndex(indexTabu);
    QWidget* w = ui->grafyTBW->currentWidget();
    QCustomPlot* GrafickyObjekt = qobject_cast<QCustomPlot*>(w);
    ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
    inicializujGrafickyObjekt(GrafickyObjekt,entropie,tennengrad,entropieStandard,tennengradStandard,
                              HP_entropie,DP_entropie,HP_tennengrad,DP_tennengrad,snimkyRozsah);
    AktualniGrafickyObjekt = GrafickyObjekt;

    ui->zobrazGrafE->setChecked(true);
    qDebug()<<ui->grafyTBW->currentIndex();
}

void GrafET::ZE()
{
    if(ui->zobrazGrafE->isChecked()==true && ui->zobrazGrafT->isChecked() == false)
    {
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minEntropie-0.5,maxEntropie+0.5);
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
        AktualniGrafickyObjekt->yAxis->setRange(minTennengrad-1,maxTennengrad+1);
        AktualniGrafickyObjekt->graph(1)->setVisible(true); // zviditelňuji graf tennengradu
        if (ui->T_HPzobraz->isChecked())
        {
            HP_tennengrad.clear();
            HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
        }
        if (ui->T_DPzobraz->isChecked())
        {
            DP_tennengrad.clear();
            DP_tennengrad.fill(dolniPrah_tennengrad,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
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
            AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
        }
        if (ui->T_DPzobraz->isChecked())
        {
            DP_tennengrad.clear();
            DP_tennengrad.fill(dolniPrah_tennengradPrepocet,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
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
        AktualniGrafickyObjekt->yAxis->setRange(minEntropie-0.5,maxEntropie+0.5);
        AktualniGrafickyObjekt->graph(0)->setVisible(true);
        if (ui->E_HPzobraz->isChecked() == true)
        {
            HP_entropie.clear();
            HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            DP_entropie.clear();
            DP_entropie.fill(dolniPrah_entropie,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
        }

        AktualniGrafickyObjekt->replot();

        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true)
    {
        AktualniGrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        AktualniGrafickyObjekt->yAxis->setRange(minTennengrad-1,maxTennengrad+1);
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
            AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
            AktualniGrafickyObjekt->graph(4)->setVisible(true);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            DP_entropie.clear();
            DP_entropie.fill(dolniPrah_entropiePrepocet,pocetSnimkuVidea);
            AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
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
    horniPrah_entropie = ui->E_HP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        HP_entropie.clear();
        HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
        AktualniGrafickyObjekt->graph(4)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        HP_entropie.clear();
        HP_entropie.fill(horniPrah_entropiePrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
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
    dolniPrah_entropie = ui->E_DP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_DPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        DP_entropie.clear();
        DP_entropie.fill(dolniPrah_entropie,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
        AktualniGrafickyObjekt->graph(5)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_DPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        DP_entropie.clear();
        DP_entropie.fill(dolniPrah_entropiePrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
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
    horniPrah_tennengrad = ui->T_HP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";        
        HP_tennengrad.clear();
        HP_tennengrad.fill(horniPrah_tennengradPrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
        AktualniGrafickyObjekt->graph(6)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        HP_tennengrad.clear();
        HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
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
    dolniPrah_tennengrad= ui->T_DP->value();    
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        DP_tennengrad.clear();
        DP_tennengrad.fill(dolniPrah_tennengradPrepocet,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
        AktualniGrafickyObjekt->graph(7)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        DP_tennengrad.clear();
        DP_tennengrad.fill(dolniPrah_tennengrad,pocetSnimkuVidea);
        AktualniGrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
        AktualniGrafickyObjekt->graph(7)->setVisible(true);
        AktualniGrafickyObjekt->replot();
    }
    else if(ui->E_DPzobraz->isChecked() == false)
    {
        AktualniGrafickyObjekt->graph(7)->setVisible(false);
        AktualniGrafickyObjekt->replot();
    }
}

