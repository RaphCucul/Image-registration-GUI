#ifndef TABS_H
#define TABS_H

#include <QWidget>
#include <QIcon>
#include "main_program/t_b_ho.h"
#include "main_program/frangi_detektor.h"
#include "main_program/registratetwo.h"
#include "main_program/registratevideo.h"
#include "main_program/chartinit.h"
//#include "dialogs/errordialog.h"

namespace Ui {
class tabs;
}

class tabs : public QWidget
{
    Q_OBJECT

public:
    explicit tabs(QWidget *parent = nullptr);
    QIcon iconRotation(QIcon i_icon);
    ~tabs();

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

    Ui::tabs *ui;

    t_b_HO* chooseFolders;
    Frangi_detektor* frangiDetector;
    RegistrateTwo* registrateTwo;
    RegistrateVideo* registrateVideo;
    ChartInit* initializeGraph;
    //QHash<QRect,ErrorDialog*> localErrorDialogHandler;
};

#endif // TABS_H
