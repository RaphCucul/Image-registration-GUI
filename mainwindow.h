#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QApplication>
#include <QActionGroup>
#include <QTranslator>
#include <QTimer>
#include <QElapsedTimer>
#include <QMessageBox>

//#include <QEvent>
#include "util/systemmonitor.h"
#include "dialogs/errordialog.h"
#include "util/versioncheckerparent.h"
#include "power/cpuwidget.h"
#include "power/memorywidget.h"
#include "power/hddusageplot.h"

namespace Ui {
class MainWindow;
}

/**
 * @class MainWindow
 * @brief The MainWindow class handles power widgets update and user's interaction with QAction elements in "Language", "Settings"
 * and "Help" menu.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    /**
     * @brief Sets y-axis range and grid color of the HDD widget main grid.
     */
    void setupUsagePlots();
private slots:
    /**
     * @brief Updates HDD widget with actual HDD usage information.
     */
    void updateWidget();

    /**
     * @brief When a user changes a language of the application, appropriate function is called.
     * @param action
     * @sa loadLanguage(const QString& rLanguage)
     */
    void slotLanguageChanged(QAction* action);
    /**
     * @brief Handles user's interactions with "Settings" menu.
     * @param action - selected option
     */
    void slotSettingsChanged(QAction* action);

    /**
     * @brief Handles user's interactions with "Help" menu.
     * @param action - selected option
     */
    void slotHelpChanged(QAction* action);

    /**
     * @brief Updates a QStatusBar with info about the program up-to-date state.
     * @param status
     */
    void versionChecked(bool status);

    /**
     * @brief After 5 seconds since program start, a version check is initiated if a user allows this program feature.
     *
     * If a new version is discovered, a program downloads release notes of the new version.
     * @sa versionChecked(bool status)
     */
    void timerTimeOut();

    /**
     * @brief It is possible to turn on/off HDD widget. Hovewer, the program must be started with admin rights.
     * Restart of the program is necessary to apply the change.
     * @param newStatus - true if HDD widget must be shown
     */
    void onHddUsagePlotClicked(bool newStatus);
protected:

    /**
     * @brief Overriden close event.
     * @param e
     */
    void closeEvent(QCloseEvent* e) override;
private:
    Ui::MainWindow *ui;

    /**
     * @brief Handles switching of the language.
     * @param rLanguage - newly selected language
     * @sa switchTranslator(QString language)
     */
    void loadLanguage(const QString& rLanguage);

    /**
     * @brief Saves newly selected language to global settings.
     *
     * Invokes a QMessageBox informing the restart of the program is necessary.
     * @param language - newly selected language
     */
    void switchTranslator(QString language);

    /**
     * @brief Adds CPU and Memory widget to the main program window. May add also HDD widget if allowed.
     * @param includeHDD - if true, HDD widget is added
     */
    void placePowerWidgets(bool includeHDD);
    QActionGroup* languageGroup = nullptr;
    QActionGroup* settingsGroup = nullptr;
    QActionGroup* helpGroup = nullptr;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandler;
    bool alreadyEvaluated = false;
    QLabel* CPUusedLabel = nullptr;
    QLabel* versionActual = nullptr;

    int timerCounter = 0;
    QTimer* timer = nullptr;
    VersionCheckerParent* vcp = nullptr;

    CPUWidget* CPU = nullptr;
    MemoryWidget* Memory = nullptr;
    HddUsagePlot* HDD = nullptr;
};

#endif // MAINWINDOW_H
