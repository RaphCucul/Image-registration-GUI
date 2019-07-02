#ifndef HDDUSAGEPLOT_H
#define HDDUSAGEPLOT_H

#include <QObject>
#include <QWidget>
#include "shared_staff/qcustomplot.h"
#include <QtCore>
#include <QtGui>

class HddUsagePlot : public QCustomPlot
{
    Q_OBJECT
public:
    HddUsagePlot(QWidget *parent = nullptr);
    void setMaximumTime(unsigned int max);
    void setMaximumUsage(double max);
    void addData(double data);
    QPixmap convertToPixmap(int width = 0, int height = 0, double scale = 1.0);
    void redraw();
    void setThemeColor(const QColor & themeColor, unsigned int scale = 1);
private:
    bool blockRedraw;
    QVector<double> time, usage;

protected:
    virtual void resizeEvent(QResizeEvent * event);
};

#endif // HDDUSAGEPLOT_H
