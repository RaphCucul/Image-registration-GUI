#include "fancy_staff/systemmonitorwidget.h"
#include <QtCharts/QChartView>
#include <QVBoxLayout>
using namespace QtCharts;

SystemMonitorWidget::SystemMonitorWidget(QWidget *parent) : QWidget(parent),mChartView(this)
{
    mRefreshTimer.setInterval(updateSeriesDelayMs);
    connect(&mRefreshTimer, &QTimer::timeout,this, &SystemMonitorWidget::updateSeries);
    QTimer::singleShot(startDelayMs,[this] { mRefreshTimer.start(); });
    mChartView.setRenderHint(QPainter::Antialiasing);
    mChartView.chart()->legend()->setVisible(false);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(&mChartView);
    setLayout(layout);
}
/* All tasks in the SystemMonitorWidget constructor are common tasks required by the child
widgets,  CpuWidget , MemoryWidget and HddWidget. The first step is the  mRefreshTimer
initialization to define the timer interval and the slot to call whenever a timeout signal is
triggered. Then the static function  QTimer::singleShot()  will start the real timer after a
delay defined by  startDelayMs . Here again, Qt combined with lambda functions will give
us a powerful code in just a few lines. The next part enables the antialiasing to smooth the
chart drawing. We hide the chart's legend to get a minimalist display. The last part handles
the layout to display the  QChartView  widget in our  SysInfoWidget  class.*/
QChartView& SystemMonitorWidget::chartView()
{
    return mChartView;
}
