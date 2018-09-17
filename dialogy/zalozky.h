#ifndef ZALOZKY_H
#define ZALOZKY_H

#include <QWidget>
#include <QIcon>
#include "t_b_ho.h"
#include "frangi_detektor.h"


namespace Ui {
class zalozky;
}

class zalozky : public QWidget
{
    Q_OBJECT

public:
    explicit zalozky(QWidget *parent = 0);
    QIcon rotace_ikonky(QIcon ikona_vstup);
    ~zalozky();

private:
    Ui::zalozky *ui;
    QIcon* ikona;
};

#endif // ZALOZKY_H
