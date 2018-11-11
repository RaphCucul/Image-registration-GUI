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
    /// Prvotni hodnoceni entropie
    QP->addGraph();
    QP->graph(8)->setPen(QPen(QColor(118, 111, 219)));
    QP->graph(8)->setLineStyle(QCPGraph::lsNone);
    QP->graph(8)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    QP->graph(8)->setVisible(false);
    /// Prvotni hodnoceni tennengrad
    QP->addGraph();
    QP->graph(9)->setPen(QPen(QColor(254, 132, 251)));
    QP->graph(9)->setLineStyle(QCPGraph::lsNone);
    QP->graph(9)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));
    QP->graph(9)->setVisible(false);
    /// Prvni rozhodovani
    QP->addGraph();
    QP->graph(10)->setPen(QPen(QColor(254, 132, 11)));
    QP->graph(10)->setLineStyle(QCPGraph::lsNone);
    QP->graph(10)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 4));
    QP->graph(10)->setVisible(false);
    /// Druhe rozhodovani
    QP->addGraph();
    QP->graph(11)->setPen(QPen(QColor(94, 186, 110)));
    QP->graph(11)->setLineStyle(QCPGraph::lsNone);
    QP->graph(11)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 4));
    QP->graph(11)->setVisible(false);
    /*QP->addGraph();
    QP->graph(12)->setData(snimkyDruheRozhodovani,entropie,true);
    QP->graph(12)->setPen(QPen(QColor(0,0,0)));
    QP->graph(12)->setVisible(false);
    QP->addGraph();
    QP->graph(13)->setData(snimkyDruheRozhodovani,entropieStandard,true);
    QP->graph(13)->setPen(QPen(QColor(0,0,0)));
    QP->graph(13)->setVisible(false);*/
    /*QP->addGraph();
    QP->graph(14)->setData(snimkyDruheRozhodovani,entropie,true);
    QP->graph(14)->setPen(QPen(QColor(0,0,0)));
    QP->graph(14)->setVisible(false);
    QP->addGraph();
    QP->graph(15)->setData(snimkyPrvotniOhodnoceni,entropieStandard,true);
    QP->graph(15)->setPen(QPen(QColor(0,0,0)));
    QP->graph(15)->setVisible(false);*/
}
