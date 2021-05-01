#include "power/memorywidget.h"
#include "util/systemmonitor.h"
//#include <QtCharts/QAreaSeries>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
using namespace QtCharts;

MemoryWidget::MemoryWidget(QWidget *parent): SystemMonitorWidget(parent),mSeries(new QPieSeries(this))
{
    mSeries->setHoleSize(0.5);
    mSeries->setPieSize(1);
    mSeries->append("Memory Load", 1.0);
    mSeries->append("Memory Free", 99.0);
    QChart* chart = chartView().chart();
    chart->addSeries(mSeries);
    chart->setMargins(QMargins(0,0,0,0));
}
void MemoryWidget::updateSeries(){
   double memoryUsed = SystemMonitor::instance().memoryUsed();
    mSeries->clear();
    mSeries->append("Memory Load", memoryUsed);
    mSeries->append("Memory Free", 100.0 - memoryUsed);
}
