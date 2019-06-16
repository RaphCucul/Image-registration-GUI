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
    void pridejData(double data);
    QPixmap prevedDoPixmap(int width = 0, int height = 0, double scale = 1.0);
    void prekresleni();
    void setThemeColor(const QColor & themeColor, unsigned int scale = 1);
private:
    bool blokacePrekresleni;
    QVector<double> cas, vyuziti;

protected:
    virtual void resizeEvent(QResizeEvent * event);
};

#endif // HDDUSAGEPLOT_H
