#include "power/hddwidget.h"
#include "util/systemmonitor.h"
#include "shared_staff/qcustomplot.h"
#include <QtCharts/QAreaSeries>

using namespace QtCharts;
const int CHART_X_RANGE_COUNT = 50;
const int CHART_X_RANGE_MAX = CHART_X_RANGE_COUNT - 1;

HddWidget::HddWidget(QWidget *parent):SystemMonitorWidget(parent),
    mSeries(new QLineSeries(this)),
    mPointPositionX(0)
{
    QAreaSeries* areaSeries = new QAreaSeries(mSeries);
    QChart* chart = chartView().chart();
    chart->addSeries(areaSeries);
    chart->createDefaultAxes();
    chart->axisX()->setVisible(false);
    chart->axisX()->setRange(0, CHART_X_RANGE_MAX);
    chart->axisY()->setRange(0, 100);
    chart->axisX()->setLabelsVisible(false);
    chart->axisY()->setLabelsVisible(false);
    chart->setMargins(QMargins(0,0,0,0));
}

void HddWidget::updateSeries(){
    double hddused = SystemMonitor::instance().hddUsed();    
    mSeries->append(mPointPositionX++,hddused);
    if (mSeries->count() > CHART_X_RANGE_COUNT) {
        QChart* chart = chartView().chart();
        chart->scroll(chart->plotArea().width()/CHART_X_RANGE_MAX, 0);
        mSeries->remove(0);
    }


}
