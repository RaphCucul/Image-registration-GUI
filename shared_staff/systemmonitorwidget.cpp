#include "shared_staff/systemmonitorwidget.h"
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

QChartView& SystemMonitorWidget::chartView()
{
    return mChartView;
}
