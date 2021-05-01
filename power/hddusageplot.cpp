#include "power/hddusageplot.h"
#include "shared_staff/qcustomplot.h"
#include "shared_staff/globalsettings.h"

HddUsagePlot::HddUsagePlot(QWidget *parent) : QCustomPlot (parent),
    time(30),usage(30)
{
    blockRedraw = false;
    axisRect()->setMinimumMargins(QMargins(0,20,0,20));
    this->setStyleSheet("border-color: red;border-width: 15px");
    xAxis->setTickLabels(false);
    yAxis->setTickLabels(false);
    xAxis->setRangeReversed(true);
    xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));

    addGraph();
    graph(0)->addData(time,usage);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(customMenuRequested(const QPoint &)));
}

void HddUsagePlot::setMaximumTime(unsigned int max)
{
    time.clear();
    for (unsigned int i = 0; i <= max; i++)
        time.append(i);

    usage.resize(max + 1);

    xAxis->setRange(0, max);
}

void HddUsagePlot::setMaximumUsage(double max)
{
    yAxis->setRange(0, max);
}

void HddUsagePlot::addData(double data)
{
    if (!usage.isEmpty()){
        usage.removeLast();
        graph(0)->data().data()->clear();
    }

    if (usage.length() > 100)
        usage.clear();

    usage.prepend(data);
    graph(0)->setData(time, usage);

    replot();
}

/*QPixmap HddUsagePlot::convertToPixmap(int width, int height, double scale)
{
    blockRedraw = true;

    xAxis->grid()->setVisible(false);
    yAxis->grid()->setVisible(false);

    QPixmap pixmap = QCustomPlot::toPixmap(width, height, scale);

    xAxis->grid()->setVisible(true);
    yAxis->grid()->setVisible(true);

    blockRedraw = false;
    return pixmap;
}*/

void HddUsagePlot::redraw()
{
    if(blockRedraw)
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

    redraw();
}

void HddUsagePlot::customMenuRequested(const QPoint &pos) {
    bool actualStatus = GlobalSettings::getSettings()->isHDDMonitorEnabled();
    QMenu contextMenu(tr("HDDUsagePlot context menu"), this);
    contextMenu.setStyleSheet("QMenu::item:selected{"
                              "background-color:qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #109910, stop: 1 #00ff00);"
                              "color: rgb(255, 255, 255);"
                              "}");
    QAction action1(actualStatus ? tr("Disable HDD monitoring") : tr("Enable HDD monitoring."), this);
    connect(&action1, &QAction::triggered,[=](){
        emit hddUsagePlotClicked(!actualStatus);
    });
    contextMenu.addAction(&action1);

    contextMenu.exec(mapToGlobal(pos));
}
