#include "dialogy/grafet.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>

GrafET::GrafET(std::vector<double> E, std::vector<double> T, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrafET)
{
    ui->setupUi(this);
    entroie = E;
    tennengrad = T;
    Graf* g = new Graf(E,T,false,false,0.0,0.0,0.0,0.0,false,false,false,false);
    //ui->grafyTBW->setStyleSheet("QTabBar::tab {height: 15px;width: 15px;padding-top:2px;padding-bottom:,2px}");
    ui->grafyTBW->addTab(g,"test");
    ui->grafyTBW->setCurrentIndex(-1);
}

GrafET::~GrafET()
{
    delete ui;
}
Graf::~Graf()
{

}
Graf::Graf(std::vector<double> E, std::vector<double> T, bool zobrazE, bool zobrazT, double HP_E, double DP_E,
           double HP_T, double DP_T, bool zobrazHP_E, bool zobrazDP_E, bool zobrazHP_T, bool zobrazDP_T,
           QWidget *parent) :
    QCustomPlot(parent)
{
    entropie = QVector<double>::fromStdVector(E);
    tennengrad = QVector<double>::fromStdVector(T);
    unsigned int pocet_snimku = E.size();
    std::vector<double> snimky(pocet_snimku);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    QVector<double> snimkyRozsah = QVector<double>::fromStdVector(snimky);
    addGraph();
    axisRect()->setMinimumMargins(QMargins(0,20,0,20));
    xAxis->setTickLabels(true);
    yAxis->setTickLabels(true);
    xAxis->setLabel("Snimky");
    yAxis->setLabel("Entropie");
    xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    //qDebug()<<entropie;
    //qDebug()<<snimkyRozsah;
    xAxis->setRange(1, 261);
    yAxis->setRange(0, 6);
    graph(0)->setData(snimkyRozsah,entropie,true);
    replot();
}
