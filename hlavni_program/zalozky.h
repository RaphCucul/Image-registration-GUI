#ifndef ZALOZKY_H
#define ZALOZKY_H

#include <QWidget>
#include <QIcon>
#include "hlavni_program/t_b_ho.h"
#include "hlavni_program/frangi_detektor.h"
#include "hlavni_program/licovanidvou.h"
#include "hlavni_program/licovanividea.h"
#include "hlavni_program/chartinit.h"

namespace Ui {
class zalozky;
}

class zalozky : public QWidget
{
    Q_OBJECT

public:
    explicit zalozky(QWidget *parent = nullptr);
    QIcon iconRotation(QIcon i_icon);
    ~zalozky();

private slots:
    void on_pages_tabBarClicked(int i_index);
    void fileFolderDirectoryLocated();
    void disableTabs_slot();
    void enableTabs_slot();
private:
    void enableTabs();
    void disableTabs();
    Ui::zalozky *ui;

    t_b_HO* chooseFolders;
    Frangi_detektor* frangiDetector;
    LicovaniDvou* registrateTwo;
    LicovaniVidea* registrateVideo;
    ChartInit* initializeGraph;
};

#endif // ZALOZKY_H
