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
    GrafickyObjekt->graph(0)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(1)->setData(snimkyRozsah,tennengrad,true);
    GrafickyObjekt->graph(1)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(2)->setData(snimkyRozsah,entropieStandard,true);
    GrafickyObjekt->graph(2)->setVisible(false);
    GrafickyObjekt->addGraph();
    GrafickyObjekt->graph(3)->setData(snimkyRozsah,tennengradStandard,true);
    GrafickyObjekt->graph(3)->setVisible(false);

    ui->grafyTBW->setCurrentIndex(-1);
    /*Graf* g = new Graf(entropie,tennengrad,zobrazEntropii,zobrazTennengrad,horniPrah_entropie,dolniPrah_entropie,
                       horniPrah_tennengrad,dolniPrah_tennengrad,zobrazHorPra_entropie,zobrazDolPra_entropie,
                       zobrazHorPra_tennengrad,zobrazDolPra_tennengrad);*/
    //GrafickyObjekt = g;
    ui->grafyTBW->addTab(GrafickyObjekt,JmenoVidea);
    ui->grafyTBW->setCurrentWidget(GrafickyObjekt);

    QObject::connect(ui->zobrazGrafE,SIGNAL(stateChanged(int)),this,SLOT(ZE()));
    QObject::connect(ui->zobrazGrafT,SIGNAL(stateChanged(int)),this,SLOT(ZT()));
    /*QObject::connect(ui->E_HPzobraz,SIGNAL(stateChanged(int)),this,SLOT(EHPZ()));
    QObject::connect(ui->E_DPzobraz,SIGNAL(stateChanged(int)),this,SLOT(EDPZ()));
    QObject::connect(ui->T_HPzobraz,SIGNAL(stateChanged(int)),this,SLOT(THPZ()));
    QObject::connect(ui->T_DPzobraz,SIGNAL(stateChanged(int)),this,SLOT(TDPZ()));
    QObject::connect(ui->E_HP,SIGNAL(stateChanged(int)),this,SLOT(EHP()));
    QObject::connect(ui->E_DP,SIGNAL(stateChanged(int)),this,SLOT(EDP()));
    QObject::connect(ui->T_HP,SIGNAL(stateChanged(int)),this,SLOT(THP()));
    QObject::connect(ui->T_DP,SIGNAL(stateChanged(int)),this,SLOT(TDP()));*/

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
    }
    else
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(1)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);
        GrafickyObjekt->replot();
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
    }
    else
    {
        GrafickyObjekt->graph(0)->setVisible(false);
        GrafickyObjekt->graph(1)->setVisible(false);
        GrafickyObjekt->graph(2)->setVisible(false);
        GrafickyObjekt->graph(3)->setVisible(false);
        GrafickyObjekt->replot();
    }
}

/*void GrafET::EHPZ()
{

}

void GrafET::EDPZ()
{

}

void GrafET::THPZ()
{

}

void GrafET::TDPZ()
{

}

void GrafET::EHP()
{

}

void GrafET::EDP()
{

}

void GrafET::THP()
{

}

void GrafET::TDP()
{

}*/

/*Graf::~Graf()
{

}*/
/*Graf::Graf(QVector<double> E,QVector<double> T, bool zobrazE, bool zobrazT, double HP_E, double DP_E,
           double HP_T, double DP_T, bool zobrazHP_E, bool zobrazDP_E, bool zobrazHP_T, bool zobrazDP_T,
           QWidget *parent) :
    QCustomPlot(parent)
{






    /// definuji všechny grafy, protože původní realizace nefungovala, když jsem měl v if podmínkách
    /// graph(0), graph(1) apod. - definuji všechno hned a zobrazování je řešeno přes viditelnost
    addGraph(xAxis,yAxis);
    graph(0)->setData(snimkyRozsah,entropie,true);
    graph(0)->setVisible(false);

    addGraph(xAxis,yAxis);
    graph(1)->setData(snimkyRozsah,tennengrad,true);
    graph(1)->setVisible(false);
    addGraph(xAxis,yAxis);
    graph(2)->setData(snimkyRozsah,T_standard,true);
    addGraph(xAxis,yAxis);
    graph(3)->setData(snimkyRozsah,E_standard,true);
    graph(2)->setVisible(false);
    graph(3)->setVisible(false);
    addGraph(xAxis,yAxis);
    graph(4)->setData(snimkyRozsah,HP_entropie,true);
    graph(4)->setVisible(false);
    addGraph(xAxis,yAxis);
    graph(5)->setData(snimkyRozsah,DP_entropie,true);
    graph(5)->setVisible(false);
    addGraph(xAxis,yAxis);
    graph(6)->setData(snimkyRozsah,HP_tennengrad,true);
    graph(6)->setVisible(false);
    addGraph(xAxis,yAxis);
    graph(7)->setData(snimkyRozsah,DP_tennengrad,true);
    graph(7)->setVisible(false);

    /// úprava chování podle požadavků na zobrazení
    if (zobrazE == true && zobrazT == false)
    {
        qDebug()<<"Entropie: "<<zobrazE<<"Tennengrad: "<<zobrazT;
        xAxis->setLabel("Snimky");
        yAxis->setLabel("Entropie");

        graph(0)->setVisible(true);
        qDebug()<<graphCount();
        vyobrazeneGrafy = 1;
        //replot();
    }
    if (zobrazE == false && zobrazT == true)
    {
        qDebug()<<"Entropie: "<<zobrazE<<"Tennengrad: "<<zobrazT;
        xAxis->setRange(1, 261);
        yAxis->setRange(minTennengrad, maxTennengrad+1);
        graph(1)->setVisible(true);
        qDebug()<<graphCount();
        vyobrazeneGrafy = 2;
        //replot();
    }
    if (zobrazE == true && zobrazT == true)
    {
        qDebug()<<"Entropie: "<<zobrazE<<"Tennengrad: "<<zobrazT;
        xAxis->setRange(1, 261);
        yAxis->setRange(0, 1);
        graph(0)->setData(snimkyRozsah,T_standard,true);
        graph(1)->setData(snimkyRozsah,E_standard,true);
        graph(0)->setVisible(true);
        graph(0)->setVisible(true);
        vyobrazeneGrafy = 3;
        qDebug()<<graphCount();
        //replot();
    }
    if (zobrazE == false && zobrazT == false)
    {

        qDebug()<<"Entropie: "<<zobrazE<<"Tennengrad: "<<zobrazT;
        if (vyobrazeneGrafy == 1 || vyobrazeneGrafy == 2)
        {
            if (graph(0)->visible())
                graph(0)->setVisible(false);
            //replot();
        }
        if (vyobrazeneGrafy == 3)
        {
            if (graph(0)->visible() || graph(1)->visible())
            {
                graph(0)->setVisible(false);
                graph(1)->setVisible(false);
            }
            //replot();
        }
        qDebug()<<graphCount();
        vyobrazeneGrafy=0;
    }


    //qDebug()<<entropie;
    //qDebug()<<snimkyRozsah;


    graph(1)->setData(snimkyRozsah,);
    graph(2);
    graph(3);
    graph(4);
}*/
