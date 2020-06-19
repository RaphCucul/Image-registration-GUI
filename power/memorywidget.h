#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QWidget>
#include "shared_staff/systemmonitorwidget.h"
#include <QtCharts/QPieSeries>

/**
 * @class MemoryWidget
 * @brief The MemoryWidget class is derived class from the SystemMonitorWidget class. The MemoryWidget overrides the updateSeries
 * function and together with SystemoMonitor signleton updates the graph of the RAM usage.
 */
class MemoryWidget : public SystemMonitorWidget
{
    Q_OBJECT
public:
    MemoryWidget(QWidget *parent = nullptr);
protected slots:
    /**
     * @brief Overridden function updates the mSeries graph with actual RAM load value.
     */
    void updateSeries() override;
private:
    QtCharts::QPieSeries* mSeries;
};

#endif // MEMORYWIDGET_H
