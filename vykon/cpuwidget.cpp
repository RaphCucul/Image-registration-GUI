#include "vykon/cpuwidget.h"
#include "util/systemmonitor.h"
//#include <QtCharts/QAreaSeries>
#include <QDebug>
#include <QLabel>
using namespace QtCharts;

const int CHART_X_RANGE_COUNT = 30;
const int CHART_X_RANGE_MAX = CHART_X_RANGE_COUNT - 1;

CPUWidget::CPUWidget(QWidget *parent):SystemMonitorWidget(parent),mSeries(new QPieSeries(this))
  //mSeries(new QLineSeries(this))
{
    mSeries->setHoleSize(0.5);
    mSeries->setPieSize(1);
    mSeries->append("CPU Load", 1.0);
    mSeries->append("CPU Free", 99.0);
    // We carve the donut with  mSeries->setHoleSize(0.35)  and we append two data sets
    // to  mSeries : a fake  CPU Load  and  Cpu Free , which are expressed in percentages.
    // The  mSeries  function is now ready to be linked to the class managing its drawing:  QChart .

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

    /* First, we get a reference to our  SysInfo  singleton. We then retrieve the current average
    CPU load in the  cpuLoadAverage  variable. We have to feed this data to our  mSeries .
    The  mSeries  object is a  QPieCharts , which implies that we just want a snapshot of the
    current CPU average load. Past history is not meaningful with this kind of graph; that's
    why we clear the  mSeries  data with the  mSeries->clear()  syntax, and append
    the  cpuLoadAverage  variable and then the free part ( 100.0 - cpuLoadAverage ).*/

   updateWidget();
}
