#include "power/cpuwidget.h"
#include "util/systemmonitor.h"
//#include <QtCharts/QAreaSeries>
#include <QDebug>
#include <QLabel>
using namespace QtCharts;

CPUWidget::CPUWidget(QWidget *parent):SystemMonitorWidget(parent),mSeries(new QPieSeries(this))
{
    mSeries->setHoleSize(0.5);
    mSeries->setPieSize(1);
    mSeries->append("CPU Load", 1.0);
    mSeries->append("CPU Free", 99.0);
    // donut with  mSeries->setHoleSize(0.50)  and we append two data sets

    QChart* chart = chartView().chart();
    chart->addSeries(mSeries);
    chart->setMargins(QMargins(0,0,0,0));
}
void CPUWidget::updateSeries(){
    double cpuLoadAverage = SystemMonitor::instance().cpuLoadAverage();
    mSeries->clear();
    mSeries->append(QString::number(cpuLoadAverage), cpuLoadAverage);
    QLabel label(QString::number(cpuLoadAverage),this);
    mSeries->append("", 100.0 - cpuLoadAverage);
   updateWidget();
}
