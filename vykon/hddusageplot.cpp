#include "vykon/hddusageplot.h"
#include "fancy_staff/qcustomplot.h"

HddUsagePlot::HddUsagePlot(QWidget *parent) : QCustomPlot (parent),
    cas(30),vyuziti(30)
{
    blokacePrekresleni = false;
    axisRect()->setMinimumMargins(QMargins(0,20,0,20));
    //xAxis2->setVisible(false);
    //yAxis2->setVisible(false);
    xAxis->setTickLabels(false);
    //xAxis2->setTickLabels(false);
    yAxis->setTickLabels(false);
    //yAxis2->setTickLabels(false);
    xAxis->setRangeReversed(true);
    xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    //xAxis2->setTickPen(QPen(QColor(255, 255, 255, 0)));
    //xAxis2->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    //yAxis2->setTickPen(QPen(QColor(255, 255, 255, 0)));
    //yAxis2->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    addGraph();
    graph(0)->addData(cas,vyuziti);
}

void HddUsagePlot::setMaximumTime(unsigned int max)
{
    cas.clear();
    for (unsigned int i = 0; i <= max; i++)
        cas.append(i);

    vyuziti.resize(max + 1);

    xAxis->setRange(0, max);
}

void HddUsagePlot::setMaximumUsage(double max)
{
    yAxis->setRange(0, max);
}

void HddUsagePlot::pridejData(double data)
{
    vyuziti.removeLast();
    vyuziti.prepend(data);
    graph(0)->setData(cas, vyuziti);
    replot();
}

QPixmap HddUsagePlot::prevedDoPixmap(int width, int height, double scale)
{
    blokacePrekresleni = true;

    xAxis->grid()->setVisible(false);
    yAxis->grid()->setVisible(false);

    QPixmap pixmap = QCustomPlot::toPixmap(width, height, scale);

    xAxis->grid()->setVisible(true);
    yAxis->grid()->setVisible(true);

    blokacePrekresleni = false;
    return pixmap;
}

void HddUsagePlot::prekresleni()
{
    if(blokacePrekresleni)
        return;
    else
        QCustomPlot::replot();
}

void HddUsagePlot::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
}

void HddUsagePlot::setThemeColor(const QColor & themeColor, unsigned int scale)
{
    QColor color = themeColor;

    // set axises' colors
    color.setAlpha(50);
    xAxis->grid()->setPen(QPen(color, 1 * scale));
    yAxis->grid()->setPen(QPen(color, 1 * scale));
    color.setAlpha(255);
    xAxis->setBasePen(QPen(color, 2 * scale));
    xAxis2->setBasePen(QPen(color, 2 * scale));
    yAxis->setBasePen(QPen(color, 2 * scale));
    yAxis2->setBasePen(QPen(color, 2 * scale));

    // set line color
    //color.setAlpha(220);
    graph(0)->setPen(QPen(color, 1 * scale));
    //color.setAlpha(30);
    graph(0)->setBrush(QBrush(QColor(0,0,100,100)));

    prekresleni();
}
