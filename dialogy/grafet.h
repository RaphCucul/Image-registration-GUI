#ifndef GRAFET_H
#define GRAFET_H

#include <QDialog>
#include <vector>
#include "fancy_staff/qcustomplot.h"

namespace Ui {
class GrafET;
}

class GrafET : public QDialog
{
    Q_OBJECT

public:
    enum class ThresholdType{
        UPPER,
        LOWER
    };
    enum class ValueType{
        ENTROPY,
        TENNENGRAD
    };
    enum class ExtremeType{
        MAX,
        MIN
    };

    explicit GrafET(QVector<double> i_entropy,
                    QVector<double> i_tennengrad,
                    QVector<int> i_FirstEvalEntropy,
                    QVector<int> i_FirstEvalTennengrad,
                    QVector<int> i_FirstDecisionResults,
                    QVector<int> i_SecondDecisionResults,
                    QVector<int> i_CompleteEvaluation,
                    QWidget *parent = nullptr);
    ~GrafET();
private slots:
    void showEntropy();
    void showTennengrad();
    void showEntropyUpperThreshold();
    void showEntropyLowerThreshold();
    void showTennengradUpperThreshold();
    void showTennengradLowerThreshold();
    //void zmenaTabu(int indexTabu);
    void firstEvaluationEntropy_f();
    void firstEvaluationTennengrad_f();
    void firstEvaluation_f();
    void secondEvaluation_f();
    void on_ohodnocKomplet_stateChanged(int arg1);
    void on_IV1_stateChanged(int arg1);
    void on_IV4_stateChanged(int arg1);
    void on_IV5_stateChanged(int arg1);

private:
    /**
     * @brief Function adds bad frame markers (evalType) into the graph.
     * @param i_graphLine
     * @param i_tabIndex
     * @param i_y_coords
     * @param i_x_coords
     * @param i_evalType
     */
    void populateGraph(int i_graphLine,QVector<int>& i_y_coords,
                       QVector<double>& i_x_coords,int i_evalType);

    /**
     * @brief Function hides all elements if neither entropy nor tennegrad are visible.
     */
    void hideAll();

    /**
     * @brief Function return the maximum or the minimum of the input vector.
     * @param i_analysedVector
     * @param typExtremu
     * @return
     */
    double findExtremes(QVector<double>& i_analysedVector, GrafET::ExtremeType extreme);

    /**
     * @brief Function creates the threshold of entropy or tennengrad for the graphical purpose.
     * @param i_extrem
     * @param threshold
     * @param type
     * @return
     */
    double createThreshold(double i_extrem, ThresholdType threshold, ValueType type);

    /**
     * @brief Function calculates standardized value of the input value.
     * @param i_inputVector
     * @param i_standardizedVector
     * @param i_max
     * @param i_min
     */
    void valueStandardization(QVector<double>& i_inputVector, QVector<double>& i_standardizedVector,
                              double i_max, double i_min);

    /**
     * @brief Function calculates standardized value of the input value.
     * @param i_originalValue
     * @param i_max
     * @param i_min
     * @return
     */
    double valueStandardization(double i_originalValue, double i_max, double i_min);

    /**
     * @brief Function creates the vector for the graph.
     * @param i_originalValueForVector
     * @param i_frameCount
     * @return
     */
    QVector<double> thresholdLine(double i_originalValueForVector, int i_frameCount);

    Ui::GrafET *ui;
    QCustomPlot* ActualGraphicalObject;
    QVector<double> entropy;
    QVector<double> tennengrad;
    QVector<int> framesEntropyFirstEvalComplete;
    QVector<int> framesTennengradFirstEvalComplete;
    QVector<int> framesFirstEvalComplete;
    QVector<int> framesSecondEvalComplete;
    QVector<int> framesEvalComplete;
    QVector<double> entropyStandard;
    QVector<double> tennengradStandard;
    QVector<double> UT_entropy,LT_entropy,HP_tennengrad,DP_tennengrad;
    QVector<double> valueRange;
    int frameCount = 0;
    double maxEntropy;
    double minEntropy;
    double maxTennengrad;
    double minTennengrad;
    double upperThreshold_entropy;
    double upperThreshold_entropyStandardized;
    double lowerThreshold_entropy;
    double lowerThreshold_entropyStandardized;
    double upperThreshold_tennengrad;
    double upperThreshold_tennengradStandardized;
    double lowerThreshold_tennengrad;
    double lowerThreshold_tennengradStandardized;
};
#endif // GRAFET_H
