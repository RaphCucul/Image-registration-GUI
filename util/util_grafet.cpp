#include "util/util_grafet.h"

util_GraphET::util_GraphET (QCustomPlot* i_QCustomPlot,
                            QMap<QString, QVector<double> > i_data, QVector<double> i_range)
{
    initGraphPlotNumbersReferences();
    initGraphObject(i_QCustomPlot,i_data,i_range);
}

void util_GraphET::initGraphPlotNumbersReferences() {
    for (int partIndex = 0; partIndex < plotParts.length(); partIndex++) {
        graphPlotNumbersReferences.insert(plotParts.at(partIndex),partIndex);
    }
    elementsMarkStyle.insert(0,QCPScatterStyle::ssCircle);
    elementsMarkStyle.insert(1,QCPScatterStyle::ssCross);
    elementsMarkStyle.insert(2,QCPScatterStyle::ssDiamond);
    elementsMarkStyle.insert(3,QCPScatterStyle::ssTriangle);
    elementsMarkStyle.insert(4,QCPScatterStyle::ssDisc);
    elementsMarkStyle.insert(5,QCPScatterStyle::ssSquare);
    elementsMarkStyle.insert(6,QCPScatterStyle::ssStar);
    elementsMarkStyle.insert(7,QCPScatterStyle::ssTriangleInverted);
    elementsMarkStyle.insert(8,QCPScatterStyle::ssCrossCircle);
    elementsMarkStyle.insert(9,QCPScatterStyle::ssCrossSquare);
    elementsMarkStyle.insert(10,QCPScatterStyle::ssCrossSquare);
    elementsMarkStyle.insert(11,QCPScatterStyle::ssCrossSquare);
}



void util_GraphET::initGraphObject(QCustomPlot* i_QCustomPlot,
                                   QMap<QString, QVector<double> > i_data,
                                   QVector<double> i_range)
{
    i_QCustomPlot->axisRect()->setMinimumMargins(QMargins(0,5,0,5));
    i_QCustomPlot->xAxis->setTickLabels(true);
    i_QCustomPlot->yAxis->setTickLabels(true);
    i_QCustomPlot->xAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    i_QCustomPlot->xAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    i_QCustomPlot->yAxis->setTickPen(QPen(QColor(255, 255, 255, 0)));
    i_QCustomPlot->yAxis->setSubTickPen(QPen(QColor(255, 255, 255, 0)));
    //i_QCustomPlot->setInteraction(QCP::iRangeDrag, Qt::Horizontal);
    //i_QCustomPlot->setInteraction(QCP::iRangeZoom);

    int _innerCounter = 0, _innerScatterCounter = 0;
    foreach (QString graphElement, plotParts) {
        i_QCustomPlot->addGraph();
        if (_innerCounter < 8){            
            i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setData(i_range,
                                                                                    i_data[graphElement],
                                                                                    true);
            i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setPen(QPen(linesColors[_innerCounter]));
        }
        else {
            if (_innerCounter < 12)
                i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setPen(QPen(linesColors[_innerCounter]));
            else
                i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setPen(QPen(QColor(0,0,0)));
            i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setLineStyle(QCPGraph::lsNone);
            i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setScatterStyle(QCPScatterStyle(elementsMarkStyle[_innerScatterCounter], 8));
            _innerScatterCounter++;
        }
        i_QCustomPlot->graph(graphPlotNumbersReferences[graphElement])->setVisible(false);
        _innerCounter++;
    }
    /*/// entropy
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(0)->setData(i_framesRange,i_data,true);
    i_QCustomPlot->graph(0)->setPen(QPen(QColor(255,0,0)));
    i_QCustomPlot->graph(0)->setVisible(false);

    /// tennengrad
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(1)->setData(i_framesRange,i_tennengrad,true);
    i_QCustomPlot->graph(1)->setPen(QPen(QColor(0,0,255)));
    i_QCustomPlot->graph(1)->setVisible(false);

    /// entropy standardizied
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(2)->setData(i_framesRange,i_entropieStandard,true);
    i_QCustomPlot->graph(2)->setPen(QPen(QColor(255,0,0)));
    i_QCustomPlot->graph(2)->setVisible(false);

    /// tennengrad standardizied
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(3)->setData(i_framesRange,i_tennengradStandard,true);
    i_QCustomPlot->graph(3)->setPen(QPen(QColor(0,0,255)));
    i_QCustomPlot->graph(3)->setVisible(false);

    /// upper threshold entropy
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(4)->setData(i_framesRange,i_HP_entropie,true);
    i_QCustomPlot->graph(4)->setPen(QPen(QColor(0,255,0)));
    i_QCustomPlot->graph(4)->setVisible(false);

    /// lower threshold entropy
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(5)->setData(i_framesRange,i_DP_entropie,true);
    i_QCustomPlot->graph(5)->setPen(QPen(QColor(0,255,0)));
    i_QCustomPlot->graph(5)->setVisible(false);

    /// upper threshold tennengrad
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(6)->setData(i_framesRange,i_HP_tennengrad,true);
    i_QCustomPlot->graph(6)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(6)->setVisible(false);

    /// lower threshold tennengrad
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(7)->setData(i_framesRange,i_DP_tennengrad,true);
    i_QCustomPlot->graph(7)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(7)->setVisible(false);
*/
    /*/// First evaluation - entropy
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(8)->setPen(QPen(QColor(118, 111, 219)));
    i_QCustomPlot->graph(8)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(8)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
    i_QCustomPlot->graph(8)->setVisible(false);

    /// First evaluation - tennengrad
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(9)->setPen(QPen(QColor(254, 132, 251)));
    i_QCustomPlot->graph(9)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(9)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 8));
    i_QCustomPlot->graph(9)->setVisible(false);

    /// First decision algorithm results
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(10)->setPen(QPen(QColor(254, 132, 11)));
    i_QCustomPlot->graph(10)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(10)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 8));
    i_QCustomPlot->graph(10)->setVisible(false);

    /// Second decision algorithm results
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(11)->setPen(QPen(QColor(94, 186, 110)));
    i_QCustomPlot->graph(11)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(11)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 8));
    i_QCustomPlot->graph(11)->setVisible(false);

    /// Complete evaluation
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(12)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(12)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(12)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8));
    i_QCustomPlot->graph(12)->setVisible(false);

    /// Evaluation index 1
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(13)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(13)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(13)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 8));
    i_QCustomPlot->graph(13)->setVisible(false);

    /// Evaluation index 4
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(14)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(14)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(14)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssStar, 8));
    i_QCustomPlot->graph(14)->setVisible(false);

    /// Evaluation index 5
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(15)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(15)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(15)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangleInverted, 8));
    i_QCustomPlot->graph(15)->setVisible(false);

    /// Frames affected by upper entropy threshold change
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(16)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(16)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(16)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 8));
    i_QCustomPlot->graph(16)->setVisible(false);

    /// Frames affected by lower entropy threshold change
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(17)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(17)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(17)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 8));
    i_QCustomPlot->graph(17)->setVisible(false);

    /// Frames affected by upper tennengrad threshold change
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(18)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(18)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(18)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 8));
    i_QCustomPlot->graph(18)->setVisible(false);

    /// Frames affected by lower tennengrad threshold change
    i_QCustomPlot->addGraph();
    i_QCustomPlot->graph(19)->setPen(QPen(QColor(0,0,0)));
    i_QCustomPlot->graph(19)->setLineStyle(QCPGraph::lsNone);
    i_QCustomPlot->graph(19)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 8));
    i_QCustomPlot->graph(19)->setVisible(false);
    */
}

QVector<double> util_GraphET::chooseFrames(QVector<int> i_problematicIndexes,
                                           QVector<double> i_values, int i_chooseType)
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

QVector<double> util_GraphET::convertInt2Double(QVector<int> i_input, int i_chooseType)
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

int util_GraphET::getNumberReference(QString i_requestedPlotPart) {
    return graphPlotNumbersReferences[i_requestedPlotPart];
}
