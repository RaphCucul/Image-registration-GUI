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

#include "vykon/cpuwidget.h"
#include "vykon/memorywidget.h"
#include "vykon/hddwidget.h"
#include "hlavni_program/zalozky.h"
#include "fancy_staff/globalsettings.h"
#include "dialogy/hdd_settings.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SystemMonitor::instance().init();

    setupUsagePlots();
    QObject::connect(ui->cpuWidget,SIGNAL(updateWidget()),this,SLOT(updateWidget()));
    this->setStyleSheet("background-color: white");

    languageGroup = new QActionGroup(ui->menuLanguage);
    languageGroup->setExclusive(true);
    settingsGroup = new QActionGroup(ui->menuSettings);
    settingsGroup->setExclusive(true);
    connect(ui->menuLanguage,SIGNAL(triggered(QAction*)),this,SLOT(slotLanguageChanged(QAction*)));
    connect(ui->menuSettings,SIGNAL(triggered(QAction*)),this,SLOT(slotSettingsChanged(QAction*)));
    localErrorDialogHandler[ui->hddWidget] = new ErrorDialog(ui->hddWidget);


    /*QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
    QString savedLanguage = GlobalSettings::getSettings()->getLanguage();
    QTranslator translator;
    if (defaultLocale == "cs" && savedLanguage != "CS"){
        if (translator.load(":/czech.qm"))
            qDebug()<<"Loading successful";
        GlobalSettings::getSettings()->setLanguage("CS");
    }
    else if (defaultLocale != "cs" && savedLanguage == "CS"){
        if (translator.load(":/english.qm"))
            qDebug()<<"Loading successful";
        GlobalSettings::getSettings()->setLanguage("EN");
    }
    else if (defaultLocale == "cs" && savedLanguage == "CS"){
        if (translator.load(":/czech.qm"))
            qDebug()<<"Loading successful";
    }
    else{
        if (translator.load(":/english.qm"))
            qDebug()<<"Loading successful";
    }
    qApp->installTranslator(&translator);*/
    //system("start powershell.exe Set-ExecutionPolicy RemoteSigned \n");

    //system("pause");
    //system("cls");
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
    if (hddused >= 0.0)
        ui->hddWidget->pridejData(hddused);
    else if (hddused < 0.0 && !alreadyEvaluated){
        localErrorDialogHandler[ui->hddWidget]->evaluate("center","",0);
        localErrorDialogHandler[ui->hddWidget]->show();
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
        if (action->text() == "Add HDD counter name"){
            HDD_Settings* _hddSettings = new HDD_Settings;
            _hddSettings->setModal(true);
            _hddSettings->exec();
        }

    }
}
