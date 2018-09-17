#ifndef HDDWIDGET_H
#define HDDWIDGET_H
#include "fancy_staff/systemmonitorwidget.h"
#include <QWidget>
#include <QtCharts/QLineSeries>
#include "qcustomplot.h"

class HddWidget : public SystemMonitorWidget
{
    Q_OBJECT
public:
    explicit HddWidget(QWidget* parent = nullptr);
protected slots:
    void updateSeries() override;
private:
    QtCharts::QLineSeries* mSeries;
    qint64 mPointPositionX;

    QCustomPlot *customPlot = new QCustomPlot();
};

#endif // HDDWIDGET_H
