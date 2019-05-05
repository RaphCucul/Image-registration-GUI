#ifndef CPUWIDGET_H
#define CPUWIDGET_H
#include "fancy_staff/systemmonitorwidget.h"
#include <QWidget>
#include <QtCharts/QPieSeries>

class CPUWidget : public SystemMonitorWidget
{
    Q_OBJECT
public:
     explicit CPUWidget(QWidget* parent = nullptr);
protected slots:    
    void updateSeries() override;
    /*Since we overrode the  SysInfoWidget::updateSeries()  slot, we have to include
    the  Q_OBJECT  macro to allow  CPUWidget  to respond to
    the  SysInfoWidgetmRefreshTimer::timeout()  signal.*/
private:
    QtCharts::QPieSeries* mSeries;
    qint64 mPointPositionX;
};

#endif // CPUWIDGET_H
