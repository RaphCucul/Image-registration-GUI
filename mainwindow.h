#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QApplication>
#include <QActionGroup>
#include <QTranslator>
//#include <QEvent>
#include "util/systemmonitor.h"
#include "dialogs/errordialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupUsagePlots();
private slots:
    void updateWidget();
    void slotLanguageChanged(QAction* action);
    void slotSettingsChanged(QAction* action);
private:
    Ui::MainWindow *ui;
    void loadLanguage(const QString& rLanguage);
    void switchTranslator(QString language);
    QActionGroup* languageGroup = nullptr;
    QActionGroup* settingsGroup = nullptr;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandler;
    bool alreadyEvaluated = false;
};
#endif // MAINWINDOW_H
