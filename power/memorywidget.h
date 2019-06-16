#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QWidget>
#include "shared_staff/systemmonitorwidget.h"
#include <QtCharts/QPieSeries>

class MemoryWidget : public SystemMonitorWidget
{
    Q_OBJECT
public:
    MemoryWidget(QWidget *parent = nullptr);
protected slots:
    void updateSeries() override;
private:
    QtCharts::QPieSeries* mSeries;
    qint64 mPointPositionX;
   /* The  mPointPositionX  is
   an  unsigned long long  (using the Qt notation  qint64 ) variable that will track the last X
   position of our data set. This huge value is used to make sure that  mPointPositionX  never overflows*/
};

#endif // MEMORYWIDGET_H
