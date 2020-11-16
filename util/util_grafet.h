#ifndef UTIL_GRAFET_H
#define UTIL_GRAFET_H
#include "shared_staff/qcustomplot.h"
#include <QMap>
#include <QStringList>

/**
 * @class util_GraphET
 * @brief The util_GraphET class is a helper class of the GrafET class.
 *
 * The purpose of the util_GraphET is simple - initialise
 * a QCustumPlot automatically as much as possible. The initialisation means creating separate graph items, adjusting visual styles etc.
 * The class constructor gets QCustomPlot class object as an input - this object is in the QDialog.
 */
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
     * @brief Selects frames which will be visualised in the graph.
     * @param[in] i_indexes - frames indexes (x axis)
     * @param[in] i_values - values of entropy/tennengrad of given frames (y axis)
     * @param[in] i_chooseType - category of frames - based on the ET analysis and evaluation index
     * @return Vector of values (y axis) for frames according to the chosen type
     */
    QVector<double> chooseFrames(QVector<int> i_indexes,
                                 QVector<double> i_values,int i_chooseType);
    /**
     * @brief Converts int values of a vector to double.
     * @param[in] i_input
     * @param[in] i_chooseType
     * @return Converted values.
     */
    QVector<double> convertInt2Double(QVector<int> i_input,int i_chooseType);

    /**
     * @brief Returns the index of requested graph element.
     * @param[in] i_requestedPlotPart
     */
    int getNumberReference(QString i_requestedPlotPart);
private:

    /**
     * @brief Main initialisation function. Loops through the list of graph parts to create corresponding graph elements,
     * together with the graphical settings.
     * @param[in] i_QCustomPlot
     * @param[in] i_data - all data for a graph
     * @param[in] i_range - y axis range
     */
    void initGraphObject(QCustomPlot* i_QCustomPlot,
                         QMap<QString,QVector<double>> i_data,
                         QVector<double> i_range);

    /**
     * @brief Assigns numeric index to each plottable graphical object.
     * Creates a connection between these indexes and scatter styles.
     */
    void initGraphPlotNumbersReferences();

    //void addElement(QCustomPlot* i_graphObject,QVector<double> i_range, QVector<double> i_data,ElementType type);

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
