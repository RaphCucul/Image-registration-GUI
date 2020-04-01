#ifndef UTIL_GRAFET_H
#define UTIL_GRAFET_H
#include "shared_staff/qcustomplot.h"
#include <QMap>
#include <QStringList>

class util_GraphET
{
public:
    enum class ElementType{
        LINE, POINT
    };

    util_GraphET(QCustomPlot* i_QCustomPlot,
                 QMap<QString,QVector<double>> i_data,
                 QVector<double> i_range);

    /**
     * @brief chooseFrames
     * @param i_problematicIndexes
     * @param i_values
     * @param i_chooseType
     * @return
     */
    QVector<double> chooseFrames(QVector<int> i_problematicIndexes,
                                 QVector<double> i_values,int i_chooseType);
    /**
     * @brief convertInt2Double
     * @param i_input
     * @param i_chooseType
     * @return
     */
    QVector<double> convertInt2Double(QVector<int> i_input,int i_chooseType);

    int getNumberReference(QString i_requestedPlotPart);
private:

    /**
     * @brief initGraphObject
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
    void initGraphObject(QCustomPlot* i_QCustomPlot,
                         QMap<QString,QVector<double>> i_data,
                         QVector<double> i_range);

    /**
     * @brief initGraphPlotNumbersReferences
     */
    void initGraphPlotNumbersReferences();

    void addElement(QCustomPlot* i_graphObject,QVector<double> i_range, QVector<double> i_data,ElementType type);

    QMap<QString,int> graphPlotNumbersReferences;
    QMap<int,QCPScatterStyle::ScatterShape> elementsMarkStyle;
    QVector<QColor> linesColors = {QColor(255,0,0),QColor(0,0,255),QColor(255,0,0),QColor(0,0,255),QColor(0,255,0),
                                   QColor(0,255,0),QColor(0,0,0),QColor(0,0,0),
                                   QColor(118, 111, 219),QColor(254, 132, 251),QColor(254, 132, 11),
                                   QColor(94, 186, 110)};
    QVector<int> scatterStyles = {4,2,7,9,5,6,8,10,13,11,11,11};
    QStringList plotParts = {"entropy", "tennengrad", "entropyStand", "tennengradStand",
                             "entropyUT", "entropyLT", "tennengradUT", "tennengradLT",
                             "entropyFirstEval", "tennengradFirstEval", "decisAlgorFirst", "decisAlgorSecond",
                             "evalComplete", "eval1", "eval4", "eval5", "framesUTE", "framesLTE",
                             "framesUTT", "framesLTT"};
};
#endif // UTIL_GRAFET_H
