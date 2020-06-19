#include "power/cpuwidget.h"
#include "util/systemmonitor.h"
//#include <QtCharts/QAreaSeries>
#include <QDebug>
#include <QLabel>
using namespace QtCharts;

CPUWidget::CPUWidget(QWidget *parent):SystemMonitorWidget(parent),mSeries(new QPieSeries(this))
  //mSeries(new QLineSeries(this))
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

    /* Retrieve the current average CPU load in the  cpuLoadAverage  variable.
    Past history is not meaningful with this kind of graph; that's
    why the  mSeries is cleared (mSeries->clear()), and append
    the  cpuLoadAverage  variable and then the free part ( 100.0 - cpuLoadAverage ).*/

   updateWidget();
}
