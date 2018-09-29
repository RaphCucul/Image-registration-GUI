#ifndef UTIL_GRAFET_H
#define UTIL_GRAFET_H
#include "fancy_staff/qcustomplot.h"
void inicializujGrafickyObjekt(QCustomPlot* QP,
                               QVector<double> entropie,
                               QVector<double> tennengrad,
                               QVector<double> entropieStandard,
                               QVector<double> tennengradStandard,
                               QVector<double> HP_entropie,
                               QVector<double> DP_entropie,
                               QVector<double> HP_tennengrad,
                               QVector<double> DP_tennengrad,
                               QVector<double> snimkyRozsah);
#endif // UTIL_GRAFET_H
