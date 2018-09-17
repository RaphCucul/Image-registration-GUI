#ifndef LICOVANIVIDEA_H
#define LICOVANIVIDEA_H

#include <QWidget>

namespace Ui {
class LicovaniVidea;
}

class LicovaniVidea : public QWidget
{
    Q_OBJECT

public:
    explicit LicovaniVidea(QWidget *parent = nullptr);
    ~LicovaniVidea();

private:
    Ui::LicovaniVidea *ui;
};

#endif // LICOVANIVIDEA_H
