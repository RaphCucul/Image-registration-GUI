#ifndef LICOVANIVIDEA_H
#define LICOVANIVIDEA_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QString>

namespace Ui {
class LicovaniVidea;
}

class LicovaniVidea : public QWidget
{
    Q_OBJECT

public:
    explicit LicovaniVidea(QWidget *parent = nullptr);
    ~LicovaniVidea();
    void clearLayout(QGridLayout *layout);

private slots:
    void on_comboBox_activated(int index);
    void zobrazKliknutelnyDialog();
    void vybVidPB_clicked(QWidget* W);
    void EplusTPB_clicked();
    void ZVPB_clicked();
private:
    Ui::LicovaniVidea *ui;
    int predchozi_index = 0;
    QVector<QString> vybraneVideoETSingle;
    bool spravnostVideaETSingle = false;
    bool volbaCasZnackyETSingle = false;
    bool volbaSvetAnomETSingle = false;
    std::vector<double> hodnoty_entropie;
    std::vector<double> hodnoty_tennengrad;
};

#endif // LICOVANIVIDEA_H
