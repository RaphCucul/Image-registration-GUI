#ifndef SYSTEMMONITORWIDGET_H
#define SYSTEMMONITORWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QtCharts/QChartView>

/**
 * @class SystemMonitorWidget
 * @brief It is the parent class for CPU and RAM memory usage widgets.
 *
 * Because both widgets are using QtCharts, virtual functions can be declared. Earlier, there was also HDDWidget class, but now it is
 * depricated and it is replaced by HDDUsagePlot class.
 * All tasks in the SystemMonitorWidget constructor are common tasks required by the child
 * widgets, CpuWidget and MemoryWidget. The first step is the mRefreshTimer
 * initialization to define the timer interval and the slot to call whenever a timeout signal is
 * triggered. Then the static function  QTimer::singleShot()  will start the real timer after a
 * delay defined by  startDelayMs. The next part enables the antialiasing to smooth the
 * chart drawing. The chart's legend is hidden to get a minimalist display. The last part handles
 * the layout to display the QChartView widget in the SysInfoWidget class.
 */
class SystemMonitorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemMonitorWidget(QWidget *parent = nullptr);
    int startDelayMs = 1000; /**< Set a delay before charts updating */
    int updateSeriesDelayMs = 100; /**< The interval between charts updates */
protected:
    QtCharts::QChartView& chartView();
protected slots:
    virtual void updateSeries() = 0; /**< virtual function overloaded by child classes.*/
private:
    QTimer mRefreshTimer;
    QtCharts::QChartView mChartView;
signals:
    void updateWidget();
};
#endif // SYSTEMMONITORWIDGET_H
