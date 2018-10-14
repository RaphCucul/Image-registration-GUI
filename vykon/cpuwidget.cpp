#include "vykon/cpuwidget.h"
#include "util/systemmonitor.h"
//#include <QtCharts/QAreaSeries>
#include <QDebug>
#include <QLabel>
using namespace QtCharts;

const int CHART_X_RANGE_COUNT = 30;
const int CHART_X_RANGE_MAX = CHART_X_RANGE_COUNT - 1;

CPUWidget::CPUWidget(QWidget *parent):SystemMonitorWidget(parent),mSeries(new QPieSeries(this))
  //mSeries(new QLineSeries(this))
{
    /*
     * Původní řešení využívající koláčový graf pro zobrazení vytížení disku
    */
    mSeries->setHoleSize(0.5);
    mSeries->setPieSize(1);
    mSeries->append("CPU Load", 1.0);
    mSeries->append("CPU Free", 99.0);
    /*We carve the donut with  mSeries->setHoleSize(0.35)  and we append two data sets
    to  mSeries : a fake  CPU Load  and  Cpu Free , which are expressed in percentages.
    The  mSeries  function is now ready to be linked to the class managing its drawing:  QChart .*/

    QChart* chart = chartView().chart();
    chart->addSeries(mSeries);
    chart->setMargins(QMargins(0,0,0,0));
    //chart->setTitle("CPU average load");
    /*chart  takes the ownership of  mSeries  and will
    draw it according to the series type–in our case, a  QPieSeries .  QChart  is not a  QWidget : it
    is a subclass of  QGraphicsWidget.*/

    /*
     * Nové řešení využívající postupu v MemoryWidget třídě
     */
    /*QAreaSeries* areaSeries = new QAreaSeries(mSeries);
    QChart* chart = chartView().chart();
    chart->addSeries(areaSeries);
    chart->createDefaultAxes();
    chart->axisX()->setVisible(false);
    chart->axisX()->setRange(0, CHART_X_RANGE_MAX);
    chart->axisY()->setRange(0, 100);
    chart->axisX()->setLabelsVisible(false);
    chart->axisY()->setLabelsVisible(false);
    chart->setMargins(QMargins(1,1,1,1));*/
}
void CPUWidget::updateSeries(){
    double cpuLoadAverage = SystemMonitor::instance().cpuLoadAverage();
    mSeries->clear();
    mSeries->append(QString::number(cpuLoadAverage), cpuLoadAverage);
    //mSeries->setLabelsPosition(QPieSlice::LabelInsideTangential);
    /*QLabel *label = new QLabel(this);
    label->show();
    label->setText(QString::number(cpuLoadAverage));
    label->setGeometry(90,205,10,10);
    label->show();*/
    QLabel label(QString::number(cpuLoadAverage),this);
    mSeries->append("", 100.0 - cpuLoadAverage);

    /* First, we get a reference to our  SysInfo  singleton. We then retrieve the current average
    CPU load in the  cpuLoadAverage  variable. We have to feed this data to our  mSeries .
    The  mSeries  object is a  QPieCharts , which implies that we just want a snapshot of the
    current CPU average load. Past history is not meaningful with this kind of graph; that's
    why we clear the  mSeries  data with the  mSeries->clear()  syntax, and append
    the  cpuLoadAverage  variable and then the free part ( 100.0 - cpuLoadAverage ).*/

   /* double cpuLoadAverage = SystemMonitor::instance().cpuLoadAverage();
    //qDebug()<<cpuLoadAverage;
     mSeries->append(mPointPositionX++, cpuLoadAverage);
     if (mSeries->count() > CHART_X_RANGE_COUNT) {
         QChart* chart = chartView().chart();
         chart->scroll(chart->plotArea().width()/CHART_X_RANGE_MAX, 0);
         mSeries->remove(0);
     }*/
    updateWidget();
}
