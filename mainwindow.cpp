#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSettings>
#include <QApplication>

#include "power/cpuwidget.h"
#include "power/memorywidget.h"
#include "power/hddwidget.h"
#include "main_program/tabs.h"
#include "shared_staff/globalsettings.h"
#include "dialogs/hdd_settings.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SystemMonitor::instance().init();

    setupUsagePlots();
    QObject::connect(ui->cpuWidget,SIGNAL(updateWidget()),this,SLOT(updateWidget()));
    this->setStyleSheet("background-color: white");
    /*QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);*/

    languageGroup = new QActionGroup(ui->menuLanguage);
    languageGroup->setExclusive(true);
    settingsGroup = new QActionGroup(ui->menuSettings);
    settingsGroup->setExclusive(true);
    connect(ui->menuLanguage,SIGNAL(triggered(QAction*)),this,SLOT(slotLanguageChanged(QAction*)));
    connect(ui->menuSettings,SIGNAL(triggered(QAction*)),this,SLOT(slotSettingsChanged(QAction*)));
    localErrorDialogHandler[ui->hddWidget] = new ErrorDialog(ui->hddWidget);
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
    /*double hddused = SystemMonitor::instance().hddUsed();
    if (hddused >= 0.0)
        ui->hddWidget->updateSeries();*/

    double hddused = SystemMonitor::instance().hddUsed();
    if (hddused >= 0.0)
        ui->hddWidget->addData(hddused);
    else if (hddused < 0.0 && !alreadyEvaluated){
        localErrorDialogHandler[ui->hddWidget]->evaluate("center","",0);
        localErrorDialogHandler[ui->hddWidget]->show(false);
        alreadyEvaluated = true;
    }
}

void MainWindow::switchTranslator(QString language)
{
    GlobalSettings::getSettings()->setLanguage(language);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Language change will be applied after application restart");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::slotLanguageChanged(QAction* action)
{
    if(nullptr != action) {
        loadLanguage(action->text());
    }
}

void MainWindow::loadLanguage(const QString& rLanguage)
{
    QString currentLanguage = GlobalSettings::getSettings()->getLanguage();
    qDebug()<<rLanguage;
    if ((rLanguage == "Angličtina" || rLanguage == "English") && currentLanguage != "EN"){
        switchTranslator("EN");
    }
    else if ((rLanguage == "Čeština" || rLanguage == "Czech") && currentLanguage != "CZ")
        switchTranslator("CZ");
}

void MainWindow::slotSettingsChanged(QAction* action){    
    if (nullptr != action){
        if (action->text() == tr("Add HDD counter name")){
            HDD_Settings* _hddSettings = new HDD_Settings;
            _hddSettings->setModal(true);
            _hddSettings->exec();
        }

    }
}
