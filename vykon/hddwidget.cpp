#include "hddwidget.h"
#include "util/systemmonitor.h"
#include "qcustomplot.h"
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
    //chart->setTitle("HDD used");
    chart->createDefaultAxes();
    chart->axisX()->setVisible(false);
    //chart->axisY()->setVisible(false);
    chart->axisX()->setRange(0, CHART_X_RANGE_MAX);
    chart->axisY()->setRange(0, 100);
    chart->axisX()->setLabelsVisible(false);
    chart->axisY()->setLabelsVisible(false);
    chart->setMargins(QMargins(0,0,0,0));
    //chart->zoomIn(QRectF(QPointF(410,300),QPointF(541,411)));

    /*QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->yAxis->setRange(0, 100);
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
    */
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
