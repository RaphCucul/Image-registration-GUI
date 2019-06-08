#include "mainwindow.h"
#include "hlavni_program/zalozky.h"
#include "fancy_staff/globalsettings.h"
#include "vykon/cpuwidget.h"
#include "vykon/hddusageplot.h"
#include "vykon/memorywidget.h"
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

    QTranslator translator;
    QString savedLanguage = GlobalSettings::getSettings()->getLanguage();
    if (savedLanguage == "CZ")
        translator.load(":/czech.qm");
    else
        translator.load(":/english.qm");
    App.installTranslator(&translator);

    MainWindow w;
    w.show();

    return App.exec();
}
