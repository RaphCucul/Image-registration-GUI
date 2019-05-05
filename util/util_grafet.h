#ifndef UTIL_GRAFET_H
#define UTIL_GRAFET_H
#include "fancy_staff/qcustomplot.h"

/**
 * @brief Function initializes all necessary graphs and all corresponing elements for GraphET class.
 * @param i_QCustomPlot
 * @param i_entropy
 * @param i_tennengrad
 * @param i_entropieStandard
 * @param i_tennengradStandard
 * @param i_HP_entropie
 * @param i_DP_entropie
 * @param i_HP_tennengrad
 * @param i_DP_tennengrad
 * @param i_framesRange
 */
void inicializujGrafickyObjekt(QCustomPlot* i_QCustomPlot,
                               QVector<double> i_entropy,
                               QVector<double> i_tennengrad,
                               QVector<double> i_entropieStandard,
                               QVector<double> i_tennengradStandard,
                               QVector<double> i_HP_entropie,
                               QVector<double> i_DP_entropie,
                               QVector<double> i_HP_tennengrad,
                               QVector<double> i_DP_tennengrad,
                               QVector<double> i_framesRange);


QVector<double> vyberHodnotySnimku(QVector<int> i_problematicIndexes,
                                   QVector<double> i_values,int i_chooseType);
QVector<double> transformInt2Double(QVector<int> i_input,int i_chooseType);
#endif // UTIL_GRAFET_H
