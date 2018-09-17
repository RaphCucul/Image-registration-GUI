#include "memorywidget.h"
#include "util/systemmonitor.h"
//#include <QtCharts/QAreaSeries>
using namespace QtCharts;

const int CHART_X_RANGE_COUNT = 50;
const int CHART_X_RANGE_MAX = CHART_X_RANGE_COUNT - 1;

MemoryWidget::MemoryWidget(QWidget *parent): SystemMonitorWidget(parent),
    mSeries(new QPieSeries(this)),
    mPointPositionX(0)
{
    //QAreaSeries* areaSeries = new QAreaSeries(mSeries);
    /* areaSeries  that takes ownership of  mSeries  upon its
    initialization in  QAreaSeries* areaSeries = new
    QareaSeries(mSeries).  areaSeries  is then added to the  chart  object
    at  chart->addSeries(areaSeries).*/
    /*QChart* chart = chartView().chart();
    chart->addSeries(areaSeries);
    chart->createDefaultAxes();
    chart->axisX()->setVisible(false);
    chart->axisX()->setRange(0, CHART_X_RANGE_MAX);
    chart->axisY()->setRange(0, 100);
    chart->axisX()->setLabelsVisible(false);
    chart->axisY()->setLabelsVisible(false);
    chart->setMargins(QMargins(0,0,0,0));
    //chart->plotArea().adjust(380,300,)
    chart->setPreferredHeight(110);
    chart->setPreferredWidth(181);*/

    mSeries->setHoleSize(0.5);
    mSeries->setPieSize(1);
    mSeries->append("Memory Load", 1.0);
    mSeries->append("Memory Free", 99.0);
    QChart* chart = chartView().chart();
    chart->addSeries(mSeries);
    chart->setMargins(QMargins(0,0,0,0));
}
void MemoryWidget::updateSeries(){
    /*double memoryUsed = SystemMonitor::instance().memoryUsed();
    mSeries->append(mPointPositionX++, memoryUsed);*/
    // poslední hodnota o využití paměti je vložena do mSeries
    /*if (mSeries->count() > CHART_X_RANGE_COUNT) {
        QChart* chart = chartView().chart();
        chart->scroll(chart->plotArea().width()/CHART_X_RANGE_MAX, 0);*/ //  will then scroll to the latest
                                                                  // point on the X axis and nothing on Y.
       /* However, what will happen when we add
       more than  CHART_X_RANGE_COUNT  points? The visible “window” on the chart is static and
       the points will be added outside. This means that we will see the memory usage only for the
       first  CHART_X_RANGE_MAX  points and then, nothing.*/
   // mSeries->remove(0);}
    /* Fortunately,  QChart  provides a function to scroll inside the view to move the visible
    window. We start to handle this case only when the dataset is bigger than the visible
    window, meaning  if (mSeries->count() > CHART_X_RANGE_COUNT) . We then remove
    the point at the index 0 with  mSeries->remove(0)  to ensure that the widget will not store
    an infinite dataset.*/
   double memoryUsed = SystemMonitor::instance().memoryUsed();
    mSeries->clear();
    mSeries->append("Memory Load", memoryUsed);
    //QLabel label(QString::number(cpuLoadAverage),this);
    mSeries->append("Memory Free", 100.0 - memoryUsed);
}
