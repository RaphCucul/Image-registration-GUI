#ifndef LICOVANIVIDEA_H
#define LICOVANIVIDEA_H

#include <QWidget>
#include <QGridLayout>

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

private:
    Ui::LicovaniVidea *ui;
    int predchozi_index = 0;
};

#endif // LICOVANIVIDEA_H
