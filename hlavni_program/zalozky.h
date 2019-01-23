#ifndef ZALOZKY_H
#define ZALOZKY_H

#include <QWidget>
#include <QIcon>
#include "hlavni_program/t_b_ho.h"
#include "hlavni_program/frangi_detektor.h"
#include "hlavni_program/licovanidvou.h"
#include "hlavni_program/licovanividea.h"


namespace Ui {
class zalozky;
}

class zalozky : public QWidget
{
    Q_OBJECT

public:
    explicit zalozky(QWidget *parent = nullptr);
    QIcon rotace_ikonky(QIcon ikona_vstup);
    ~zalozky();

private slots:
    void on_stranky_tabBarClicked(int index);
    void fileFolderDirectoryLocated();
private:
    Ui::zalozky *ui;
    QIcon* ikona;

    t_b_HO* volba_slozek;
    Frangi_detektor* detektor;
    LicovaniDvou* licovaniDvou;
    LicovaniVidea* licovaniVidea;
};

#endif // ZALOZKY_H
