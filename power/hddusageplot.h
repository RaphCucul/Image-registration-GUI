#ifndef HDDUSAGEPLOT_H
#define HDDUSAGEPLOT_H

#include <QObject>
#include <QWidget>
#include "shared_staff/qcustomplot.h"
#include <QtCore>
#include <QtGui>

/**
 * @class HddUsagePlot
 * @brief The HddUsagePlot class is derived from the QCustomPlot class. In comparison with CPUWidget and MemoryWidget, it
 * does not use a SystemMonitor singleton. Updated data of HDD usage are calculated in the MainWindow class as the reaction
 * on the CPU graph data update => addData() function is used to update the QCustomPlot graph.
 */
class HddUsagePlot : public QCustomPlot
{
    Q_OBJECT
public:
    HddUsagePlot(QWidget *parent = nullptr);
    /**
     * @brief Sets the scale of the x axis. Actual setting is 60.
     * @param[in] max
     */
    void setMaximumTime(unsigned int max);
    /**
     * @brief Sets the scale of the y axis. Actual setting is 110.
     * @param[in] max
     */
    void setMaximumUsage(double max);
    /**
     * @brief The most important function - adds data to the graph. Also updates the variable storing data
     * from the past.
     * @param data
     */
    void addData(double data);

    /**
     * @brief Calls replot() function to re-initialise the graph completely (if some changes were made).
     */
    void redraw();
    /**
     * @brief Enables to define how the graph will look like.
     * @param themeColor
     * @param scale
     */
    void setThemeColor(const QColor & themeColor, unsigned int scale = 1);
private:
    bool blockRedraw;
    QVector<double> time, usage;

protected:
    virtual void resizeEvent(QResizeEvent * event);
};

#endif // HDDUSAGEPLOT_H
