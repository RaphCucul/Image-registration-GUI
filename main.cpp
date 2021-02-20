#include "mainwindow.h"
#include "main_program/tabs.h"
#include "shared_staff/globalsettings.h"
#include "power/cpuwidget.h"
#include "power/hddusageplot.h"
#include "power/memorywidget.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(translations);

    QApplication App(argc, argv);

    QString _applicationDirectoryPath = App.applicationDirPath();
    QString _storedAppDirPath = GlobalSettings::getSettings()->getAppPath();

    if (_storedAppDirPath == "" || _storedAppDirPath != _applicationDirectoryPath)
        GlobalSettings::getSettings()->setAppPath(_applicationDirectoryPath);
    if (!GlobalSettings::getSettings()->checkPresenceOfSetting("UsedCores"))
        GlobalSettings::getSettings()->setUsedCores(1);

    QTranslator translator;
    QString savedLanguage = GlobalSettings::getSettings()->getLanguage();
    if (savedLanguage == "CZ")
        translator.load(":/translations/czech.qm");
    else
        translator.load(":/translations/english.qm");
    App.installTranslator(&translator);

    MainWindow w;
    w.show();

    return App.exec();
}
