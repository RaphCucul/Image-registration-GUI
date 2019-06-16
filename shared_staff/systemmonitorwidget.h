#ifndef SYSTEMMONITORWIDGET_H
#define SYSTEMMONITORWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QtCharts/QChartView>

class SystemMonitorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemMonitorWidget(QWidget *parent = nullptr);
    int startDelayMs = 100;
    int updateSeriesDelayMs = 100;
protected:
    QtCharts::QChartView& chartView();
protected slots:
    virtual void updateSeries() = 0;
private:
    QTimer mRefreshTimer;
    QtCharts::QChartView mChartView;
signals:
    void updateWidget();
};
#endif // SYSTEMMONITORWIDGET_H
