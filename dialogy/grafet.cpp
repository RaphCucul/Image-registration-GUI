#include "dialogy/grafet.h"
#include "util/entropie.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>
#include <QWidget>

GrafET::GrafET(std::vector<double> E, std::vector<double> T, QString jmeno_videa, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrafET)
{
    ui->setupUi(this);
    QWidget *WSCustomPlot = new QWidget();
    GrafickyObjekt = new QCustomPlot(WSCustomPlot);

    entropie = QVector<double>::fromStdVector(E);
    tennengrad = QVector<double>::fromStdVector(T);
    horniPrah_entropie = ui->E_HP->value();
    dolniPrah_entropie = ui->E_DP->value();
    horniPrah_tennengrad = ui->T_HP->value();
    dolniPrah_tennengrad = ui->T_DP->value();
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
    //qDebug()<<"E: "<<maxEntropie<<" "<<minEntropie;
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

    /// nastavení grafu
    GrafickyObjekt->axisRect()->setMinimumMargins(QMargins(0,20,0,20));
    GrafickyObjekt->xAxis->setTickLabels(true);
    GrafickyObjekt->yAxis->setTickLabels(true);
    GrafickyObjekt->xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    GrafickyObjekt->xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    GrafickyObjekt->yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    GrafickyObjekt->yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));

    /// Grafy proměnných
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(0)->setData(snimkyRozsah,entropie,true);
    GrafickyObjekt->graph(0)->setPen(QPen(QColor(255,0,0)));
    GrafickyObjekt->graph(0)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(1)->setData(snimkyRozsah,tennengrad,true);
    GrafickyObjekt->graph(1)->setPen(QPen(QColor(0,0,255)));
    GrafickyObjekt->graph(1)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(2)->setData(snimkyRozsah,entropieStandard,true);
    GrafickyObjekt->graph(2)->setPen(QPen(QColor(255,0,0)));
    GrafickyObjekt->graph(2)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(3)->setData(snimkyRozsah,tennengradStandard,true);
    GrafickyObjekt->graph(3)->setPen(QPen(QColor(0,0,255)));
    GrafickyObjekt->graph(3)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie,true);
    GrafickyObjekt->graph(4)->setPen(QPen(QColor(0,255,0)));
    GrafickyObjekt->graph(4)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie,true);
    GrafickyObjekt->graph(5)->setPen(QPen(QColor(0,255,0)));
    GrafickyObjekt->graph(5)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad,true);
    GrafickyObjekt->graph(6)->setPen(QPen(QColor(0,0,0)));
    GrafickyObjekt->graph(6)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad,true);
    GrafickyObjekt->graph(7)->setPen(QPen(QColor(0,0,0)));
    GrafickyObjekt->graph(7)->setVisible(false);

    /// inicializace QTabWidgetu a vytvoření první záložky
    ui->grafyTBW->setCurrentIndex(-1);    
    ui->grafyTBW->addTab(GrafickyObjekt,JmenoVidea);
    ui->grafyTBW->setCurrentWidget(GrafickyObjekt);

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

    //ui->grafyTBW->setStyleSheet("QTabBar::tab {height: 15px;width: 15px;padding-top:2px;padding-bottom:,2px}");

}

GrafET::~GrafET()
{
    delete ui;
}

void GrafET::ZE()
{
    if(ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false)
    {
        GrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        GrafickyObjekt->yAxis->setRange(0,maxEntropie+0.5);

        GrafickyObjekt->graph(0)->setVisible(true);
        GrafickyObjekt->graph(0)->setScatterSkip(0);

        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(false);
        ui->T_DPzobraz->setEnabled(false);
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true)
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);

        GrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        GrafickyObjekt->yAxis->setRange(minTennengrad-1,maxTennengrad+1);
        GrafickyObjekt->graph(1)->setVisible(true);

        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true)
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(1)->setVisible(false);

        GrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        GrafickyObjekt->yAxis->setRange(0,1);
        GrafickyObjekt->graph(2)->setVisible(true);
        GrafickyObjekt->graph(2)->setScatterSkip(0);
        GrafickyObjekt->graph(3)->setVisible(true);
        GrafickyObjekt->graph(3)->setScatterSkip(0);

        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(1)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);
        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(false);
        ui->T_DPzobraz->setEnabled(false);
    }
}

void GrafET::ZT()
{
    if(ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false)
    {
        GrafickyObjekt->graph(1)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);

        GrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        GrafickyObjekt->yAxis->setRange(0,maxEntropie+0.5);
        GrafickyObjekt->graph(0)->setVisible(true);
        GrafickyObjekt->graph(0)->setScatterSkip(0);

        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(false);
        ui->T_DPzobraz->setEnabled(false);
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true)
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);

        GrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        GrafickyObjekt->yAxis->setRange(minTennengrad-1,maxTennengrad+1);
        GrafickyObjekt->graph(1)->setVisible(true);

        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true)
    {
        GrafickyObjekt->xAxis->setRange(1, pocetSnimkuVidea);
        GrafickyObjekt->yAxis->setRange(0,1);
        GrafickyObjekt->graph(2)->setVisible(true);
        GrafickyObjekt->graph(2)->setScatterSkip(0);
        GrafickyObjekt->graph(3)->setVisible(true);
        GrafickyObjekt->graph(3)->setScatterSkip(0);

        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(1)->setVisible(false);

        GrafickyObjekt->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
    }
    else
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(1)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);
        GrafickyObjekt->replot();
        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(false);
        ui->T_DPzobraz->setEnabled(false);
    }
}

void GrafET::EHPZ()
{
    horniPrah_entropie = ui->E_HP->value();
    //qDebug()<<"horní práh entropie: "<<horniPrah_entropie;
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        HP_entropie.clear();
        HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
        GrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
        GrafickyObjekt->graph(4)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        double prepocetHP_E = (horniPrah_entropie-minEntropie)/(maxEntropie-minEntropie);
        HP_entropie.clear();
        HP_entropie.fill(prepocetHP_E,pocetSnimkuVidea);
        GrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
        GrafickyObjekt->graph(4)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->E_HPzobraz->isChecked() == true)
    {
        HP_entropie.clear();
        HP_entropie.fill(horniPrah_entropie,pocetSnimkuVidea);
        GrafickyObjekt->graph(4)->setData(snimkyRozsah,HP_entropie);
        GrafickyObjekt->graph(4)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->E_HPzobraz->isChecked() == false)
    {
        GrafickyObjekt->graph(4)->setVisible(false);
        GrafickyObjekt->replot();
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
        GrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
        GrafickyObjekt->graph(5)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->E_DPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        double prepocetDP_E = (dolniPrah_entropie-minEntropie)/(maxEntropie-minEntropie);
        DP_entropie.clear();
        DP_entropie.fill(prepocetDP_E,pocetSnimkuVidea);
        GrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
        GrafickyObjekt->graph(5)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->E_DPzobraz->isChecked() == true)
    {
        DP_entropie.clear();
        DP_entropie.fill(dolniPrah_entropie,pocetSnimkuVidea);
        GrafickyObjekt->graph(5)->setData(snimkyRozsah,DP_entropie);
        GrafickyObjekt->graph(5)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if(ui->E_DPzobraz->isChecked() == false)
    {
        GrafickyObjekt->graph(5)->setVisible(false);
        GrafickyObjekt->replot();
    }
}

void GrafET::THPZ()
{
    horniPrah_tennengrad = ui->T_HP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->T_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu tennengradu.";
        HP_tennengrad.clear();
        HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
        GrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
        GrafickyObjekt->graph(6)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        qDebug()<<"Zobrazuji graf horního prahu entropie.";
        double prepocetHP_T = (horniPrah_tennengrad-minTennengrad)/(maxTennengrad-minTennengrad);
        HP_tennengrad.clear();
        HP_tennengrad.fill(prepocetHP_T,pocetSnimkuVidea);
        GrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
        GrafickyObjekt->graph(6)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_HPzobraz->isChecked() == true)
    {
        HP_tennengrad.clear();
        HP_tennengrad.fill(horniPrah_tennengrad,pocetSnimkuVidea);
        GrafickyObjekt->graph(6)->setData(snimkyRozsah,HP_tennengrad);
        GrafickyObjekt->graph(6)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->T_HPzobraz->isChecked() == false)
    {
        GrafickyObjekt->graph(6)->setVisible(false);
        GrafickyObjekt->replot();
    }
}

void GrafET::TDPZ()
{
    dolniPrah_tennengrad= ui->T_DP->value();
    if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == false &&
            ui->T_DPzobraz->isChecked() == true)
    {
        DP_tennengrad.clear();
        DP_tennengrad.fill(dolniPrah_entropie,pocetSnimkuVidea);
        GrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
        GrafickyObjekt->graph(7)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == true && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        double prepocetDP_T = (dolniPrah_tennengrad-minTennengrad)/(maxTennengrad-minTennengrad);
        DP_tennengrad.clear();
        DP_tennengrad.fill(prepocetDP_T,pocetSnimkuVidea);
        GrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
        GrafickyObjekt->graph(7)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if (ui->zobrazGrafE->isChecked() == false && ui->zobrazGrafT->isChecked() == true &&
             ui->T_DPzobraz->isChecked() == true)
    {
        DP_tennengrad.clear();
        DP_tennengrad.fill(dolniPrah_entropie,pocetSnimkuVidea);
        GrafickyObjekt->graph(7)->setData(snimkyRozsah,DP_tennengrad);
        GrafickyObjekt->graph(7)->setVisible(true);
        GrafickyObjekt->replot();
    }
    else if(ui->E_DPzobraz->isChecked() == false)
    {
        GrafickyObjekt->graph(7)->setVisible(false);
        GrafickyObjekt->replot();
    }
}
