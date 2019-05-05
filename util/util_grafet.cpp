#include "util/util_grafet.h"

void inicializujGrafickyObjekt(QCustomPlot* i_QCustomPlot,
                               QVector<double> i_entropy,
                               QVector<double> i_tennengrad,
                               QVector<double> i_entropieStandard,
                               QVector<double> i_tennengradStandard,
                               QVector<double> i_HP_entropie,
                               QVector<double> i_DP_entropie,
                               QVector<double> i_HP_tennengrad,
                               QVector<double> i_DP_tennengrad,
                               QVector<double> i_framesRange)
{
    i_QCustomPlot->axisRect()->setMinimumMargins(QMargins(0,20,0,20));
    i_QCustomPlot->xAxis->setTickLabels(true);
    i_QCustomPlot->yAxis->setTickLabels(true);
    i_QCustomPlot->xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    i_QCustomPlot->xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    i_QCustomPlot->yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    i_QCustomPlot->yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));

    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(0)->setData(i_framesRange,i_entropy,true);
    i_QCustomPlot->graph(0)->setPen(QPen(QColor(255,0,0)));
    i_QCustomPlot->graph(0)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(1)->setData(i_framesRange,i_tennengrad,true);
    i_QCustomPlot->graph(1)->setPen(QPen(QColor(0,0,255)));
    i_QCustomPlot->graph(1)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(2)->setData(i_framesRange,i_entropieStandard,true);
    i_QCustomPlot->graph(2)->setPen(QPen(QColor(255,0,0)));
    i_QCustomPlot->graph(2)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(3)->setData(i_framesRange,i_tennengradStandard,true);
    i_QCustomPlot->graph(3)->setPen(QPen(QColor(0,0,255)));
    i_QCustomPlot->graph(3)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(4)->setData(i_framesRange,i_HP_entropie,true);
    i_QCustomPlot->graph(4)->setPen(QPen(QColor(0,255,0)));
    i_QCustomPlot->graph(4)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(5)->setData(i_framesRange,i_DP_entropie,true);
    i_QCustomPlot->graph(5)->setPen(QPen(QColor(0,255,0)));
    i_QCustomPlot->graph(5)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(6)->setData(i_framesRange,i_HP_tennengrad,true);
    i_QCustomPlot->graph(6)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(6)->setVisible(false);
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(7)->setData(i_framesRange,i_DP_tennengrad,true);
    i_QCustomPlot->graph(7)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(7)->setVisible(false);
    /// Prvotni hodnoceni entropie
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(8)->setPen(QPen(QColor(118, 111, 219)));
    i_QCustomPlot->graph(8)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(8)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
    i_QCustomPlot->graph(8)->setVisible(false);
    /// Prvotni hodnoceni tennengrad
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(9)->setPen(QPen(QColor(254, 132, 251)));
    i_QCustomPlot->graph(9)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(9)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 8));
    i_QCustomPlot->graph(9)->setVisible(false);
    /// Prvni rozhodovani
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(10)->setPen(QPen(QColor(254, 132, 11)));
    i_QCustomPlot->graph(10)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(10)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 8));
    i_QCustomPlot->graph(10)->setVisible(false);
    /// Druhe rozhodovani
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(11)->setPen(QPen(QColor(94, 186, 110)));
    i_QCustomPlot->graph(11)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(11)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 8));
    i_QCustomPlot->graph(11)->setVisible(false);
    /// kompletni hodnoceni
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(12)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(12)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(12)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8));
    i_QCustomPlot->graph(12)->setVisible(false);
    /// index vhodnosti 1
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(13)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(13)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(13)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 8));
    i_QCustomPlot->graph(13)->setVisible(false);
    /// index vhodnosti 4
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(14)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(14)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(14)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssStar, 8));
    i_QCustomPlot->graph(14)->setVisible(false);
    /// index vhodnosti 5
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(15)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(15)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(15)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangleInverted, 8));
    i_QCustomPlot->graph(15)->setVisible(false);
}

QVector<double> vyberHodnotySnimku(QVector<int> i_problematicIndexes, QVector<double> i_values, int i_chooseType)
{
    QVector<double> outputVector;
    if (i_chooseType == 0){ // standard choice of values
        for (int var = 0; var < i_problematicIndexes.length(); var++) {
            if (var > i_values.length() || i_problematicIndexes[var] > i_values.length())
                break;
            else
                outputVector.push_back(i_values[i_problematicIndexes[var]]);
        }
    }
    if (i_chooseType == 1){ // only all problematic frames
        for (int var = 0; var < i_problematicIndexes.length(); var++) {
            if (i_problematicIndexes[var] == 1 || i_problematicIndexes[var] == 4 || i_problematicIndexes[var] == 5){
                outputVector.push_back(i_values[var]);
            }
        }
    }
    if (i_chooseType == 2){ // index of ... 1
        for (int var = 0; var < i_problematicIndexes.length(); var++) {
            if (i_problematicIndexes[var] == 1){
                outputVector.push_back(i_values[var]);
            }
        }
    }
    if (i_chooseType == 3){ // index of ... 4
        for (int var = 0; var < i_problematicIndexes.length(); var++) {
            if (i_problematicIndexes[var] == 4){
                outputVector.push_back(i_values[var]);
            }
        }
    }
    if (i_chooseType == 4){ // index of ... 5
        for (int var = 0; var < i_problematicIndexes.length(); var++) {
            if (i_problematicIndexes[var] == 5){
                outputVector.push_back(i_values[var]);
            }
        }
    }
    return outputVector;
}

QVector<double> transformInt2Double(QVector<int> i_input, int i_chooseType)
{
    QVector<double> outputVector;
    if (i_chooseType == 0){
        for (int var = 0; var < i_input.length(); var++){
            outputVector.push_back(double(i_input[var]));
        }
    }
    if (i_chooseType == 1){ // all bad frames
        for (int var = 0; var < i_input.length(); var++){
            if (i_input[var] == 1 || i_input[var] == 4 || i_input[var] == 5){
                outputVector.push_back(double(var));
            }
        }
    }
    if (i_chooseType == 2){ // index of ... 1
        for (int var = 0; var < i_input.length(); var++){
            if (i_input[var] == 1){
                outputVector.push_back(double(var));
            }
        }
    }
    if (i_chooseType == 3){ // index of ... 4
        for (int var = 0; var < i_input.length(); var++){
            if (i_input[var] == 4){
                outputVector.push_back(double(var));
            }
        }
    }
    if (i_chooseType == 4){ // index of ... 5
        for (int var = 0; var < i_input.length(); var++){
            if (i_input[var] == 5){
                outputVector.push_back(double(var));
            }
        }
    }

    return outputVector;
}
