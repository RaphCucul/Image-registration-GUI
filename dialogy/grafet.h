#ifndef GRAFET_H
#define GRAFET_H

#include <QDialog>
#include <vector>
#include "qcustomplot.h"

namespace Ui {
class GrafET;
}

class GrafET : public QDialog
{
    Q_OBJECT

public:
    explicit GrafET(std::vector<double> E, std::vector<double> T, QWidget *parent = nullptr);
    ~GrafET();

private:
    Ui::GrafET *ui;
    std::vector<double> entroie;
    std::vector<double> tennengrad;
};

class Graf : public QCustomPlot
{
    Q_OBJECT
public:
    Graf(std::vector<double> E, std::vector<double> T, bool zobrazE, bool zobrazT, double HP_E, double DP_E,
         double HP_T, double DP_T, bool zobrazHP_E, bool zobrazDP_E, bool zobrazHP_T, bool zobrazDP_T,
         QWidget *parent = nullptr);
    ~Graf();
    QVector<double> entropie;
    QVector<double> tennengrad;
};

#endif // GRAFET_H
