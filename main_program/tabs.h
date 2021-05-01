#ifndef TABS_H
#define TABS_H

#include <QWidget>
#include <QIcon>
#include "main_program/directories_loader.h"
#include "main_program/frangi_detektor.h"
#include "main_program/registratetwo.h"
#include "main_program/registratevideo.h"
#include "main_program/chartinit.h"
//#include "dialogs/errordialog.h"

namespace Ui {
class tabs;
}

/**
 * @class tabs
 * @brief The tabs class contains QTabWidget with all important program parts placed in independent tabs.
 */
class tabs : public QWidget
{
    Q_OBJECT

public:
    explicit tabs(QWidget *parent = nullptr);
    /**
     * @brief Rotates given icon for 90°.
     * @param[in] i_icon - original icon
     * @param[in] size - size of the icon
     * @return rotated icon
     */
    QIcon iconRotation(QIcon i_icon, int size);
    ~tabs();

private slots:
    /**
     * @brief When a tab is clicked, program automatically checks if a path to videos is set.
     *
     * If the path exists, it automatically checks the content of the folder and place the name of the first found
     * video to the videopicker widget. If a path is not found, a placeholder is visible in videopicker widget.
     * @param[in] i_index - tab index
     */
    void on_pages_tabBarClicked(int i_index);
    /**
     * @brief When the file with directories is located, loaded and all necessary conditions are passed, all tabs are enabled
     * and the program is fully functional.
     */
    void fileFolderDirectoryLocated();
    /**
     * @brief Calls "disableTabs" function in case a calculation has started. Tabs are disabled when the file with directories is not
     * found.
     *
     * In the case of running calculation, only currently selected tab stays active, the rest of tabs is deactivated.
     */
    void disableTabs_slot();
    /**
     * @brief Enables all tabs.
     *
     * Tabs can be enabled when a user completes the list of necessary directories or a calculation is finished/terminated.
     */
    void enableTabs_slot();
private:
    /**
     * @brief Enables the program tabs when a calculation is finished.
     */
    void enableTabs();

    /**
     * @brief Disables the program tabs when a calculation starts.
     */
    void disableTabs();

    /**
     * @brief Tabs for registration are disabled on the application startup by default until
     * the coordinates of frangi filter maximum are not obtained.
     */
    void default_disabled();

    Ui::tabs *ui;

    DirectoriesLoader* chooseFolders;
    Frangi_detektor* frangiDetector;
    RegistrateTwo* registrateTwo;
    RegistrateVideo* registrateVideo;
    ChartInit* initializeGraph;
};

#endif // TABS_H
