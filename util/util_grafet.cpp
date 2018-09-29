#include "util/util_grafet.h"

void inicializujGrafickyObjekt(QCustomPlot* QP,
                               QVector<double> entropie,
                               QVector<double> tennengrad,
                               QVector<double> entropieStandard,
                               QVector<double> tennengradStandard,
                               QVector<double> HP_entropie,
                               QVector<double> DP_entropie,
                               QVector<double> HP_tennengrad,
                               QVector<double> DP_tennengrad,
                               QVector<double> snimkyRozsah)
{
    QP->axisRect()->setMinimumMargins(QMargins(0,20,0,20));
    QP->xAxis->setTickLabels(true);
    QP->yAxis->setTickLabels(true);
    QP->xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    QP->xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    QP->yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    QP->yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));

    QP->addGraph();
    QP->graph(0)->setData(snimkyRozsah,entropie,true);
    QP->graph(0)->setPen(QPen(QColor(255,0,0)));
    QP->graph(0)->setVisible(false);
    QP->addGraph();
    QP->graph(1)->setData(snimkyRozsah,tennengrad,true);
    QP->graph(1)->setPen(QPen(QColor(0,0,255)));
    QP->graph(1)->setVisible(false);
    QP->addGraph();
    QP->graph(2)->setData(snimkyRozsah,entropieStandard,true);
    QP->graph(2)->setPen(QPen(QColor(255,0,0)));
    QP->graph(2)->setVisible(false);
    QP->addGraph();
    QP->graph(3)->setData(snimkyRozsah,tennengradStandard,true);
    QP->graph(3)->setPen(QPen(QColor(0,0,255)));
    QP->graph(3)->setVisible(false);
    QP->addGraph();
    QP->graph(4)->setData(snimkyRozsah,HP_entropie,true);
    QP->graph(4)->setPen(QPen(QColor(0,255,0)));
    QP->graph(4)->setVisible(false);
    QP->addGraph();
    QP->graph(5)->setData(snimkyRozsah,DP_entropie,true);
    QP->graph(5)->setPen(QPen(QColor(0,255,0)));
    QP->graph(5)->setVisible(false);
    QP->addGraph();
    QP->graph(6)->setData(snimkyRozsah,HP_tennengrad,true);
    QP->graph(6)->setPen(QPen(QColor(0,0,0)));
    QP->graph(6)->setVisible(false);
    QP->addGraph();
    QP->graph(7)->setData(snimkyRozsah,DP_tennengrad,true);
    QP->graph(7)->setPen(QPen(QColor(0,0,0)));
    QP->graph(7)->setVisible(false);
}
