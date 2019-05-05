#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QLineEdit>
#include <fstream>
#include <iostream>
#include <QVBoxLayout>
#include <QSettings>
#include <QApplication>

//#include <QtQuick/QQuickView>
//#include <QtQuick/QQuickWindow>
//#include <QtQuickWidgets/QQuickWidget>

#include "vykon/cpuwidget.h"
#include "vykon/memorywidget.h"
#include "vykon/hddwidget.h"
#include "hlavni_program/zalozky.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SystemMonitor::instance().init();

    setupUsagePlots();
    QObject::connect(ui->cpuWidget,SIGNAL(updateWidget()),this,SLOT(updateWidget()));
    this->setStyleSheet("background-color: white");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUsagePlots()
{
    ui->hddWidget->setMaximumTime(60);
    ui->hddWidget->setMaximumUsage(110);
    ui->hddWidget->setThemeColor(QColor(Qt::blue));
}

void MainWindow::updateWidget()
{
    double hddused = SystemMonitor::instance().hddUsed();
    ui->hddWidget->pridejData(hddused);
}
