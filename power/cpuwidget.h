#ifndef CPUWIDGET_H
#define CPUWIDGET_H
#include "shared_staff/systemmonitorwidget.h"
#include <QWidget>
#include <QtCharts/QPieSeries>

/**
 * @class CPUWidget
 * @brief The CPUWidget class is derived from the SystemMonitorWidget class. CPUWidget class overrides updateSeries
 * function and together with the SystemMonitor singleton updates the mSeries chart with actual CPU load value.
 */
class CPUWidget : public SystemMonitorWidget
{
    Q_OBJECT
public:
     explicit CPUWidget(QWidget* parent = nullptr);
protected slots:
    /**
     * @brief Overriden function updates the mSeries chart with the actual CPU load and free values.
     */
    void updateSeries() override;
private:
    QtCharts::QPieSeries* mSeries;
};

#endif // CPUWIDGET_H
