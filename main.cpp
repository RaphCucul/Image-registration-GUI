#include "mainwindow.h"
#include "dialogy/zalozky.h"
#include "vykon/cpuwidget.h"
#include "vykon/hddusageplot.h"
#include "vykon/memorywidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
