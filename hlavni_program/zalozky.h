#ifndef ZALOZKY_H
#define ZALOZKY_H

#include <QWidget>
#include <QIcon>
#include "hlavni_program/t_b_ho.h"
#include "hlavni_program/frangi_detektor.h"
#include "hlavni_program/licovanidvou.h"
#include "hlavni_program/licovanividea.h"
#include "hlavni_program/chartinit.h"
//#include "dialogy/errordialog.h"

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
    /**
     * @brief Function enables the program tabs when a calculation is finished.
     */
    void enableTabs();

    /**
     * @brief Function disables the program tabs when a calculation starts.
     */
    void disableTabs();

    /**
     * @brief Tabs for registration are disabled on the application startup by default until
     * the coordinates of frangi filter maximum are not obtained.
     */
    void default_disabled();

    Ui::zalozky *ui;

    t_b_HO* chooseFolders;
    Frangi_detektor* frangiDetector;
    LicovaniDvou* registrateTwo;
    LicovaniVidea* registrateVideo;
    ChartInit* initializeGraph;
    //QHash<QRect,ErrorDialog*> localErrorDialogHandler;
};

#endif // ZALOZKY_H
