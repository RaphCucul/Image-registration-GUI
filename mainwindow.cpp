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
#include <QThread>
#include <QInputDialog>
#include <QUrl>

#include "power/cpuwidget.h"
#include "power/memorywidget.h"
#include "main_program/tabs.h"
#include "shared_staff/globalsettings.h"
#include "dialogs/hdd_settings.h"
#include "util/aboutprogram.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SystemMonitor::instance().init();

    setupUsagePlots();
    if (GlobalSettings::getSettings()->isHDDMonitorEnabled()) {
        QObject::connect(ui->cpuWidget,SIGNAL(updateWidget()),this,SLOT(updateWidget()));
    }
    else {
        ui->hddWidget->setThemeColor(QColor("#dfe9ff"));
    }
    connect(ui->hddWidget,SIGNAL(hddUsagePlotClicked(bool)),this,SLOT(onHddUsagePlotClicked(bool)));

    this->setStyleSheet("background-color: white");

    vcp = new VersionCheckerParent;

    CPUusedLabel = new QLabel(this);
    QString _cpuText = tr("Used cores: ")+QString::number(GlobalSettings::getSettings()->getUsedCores(),10);
    CPUusedLabel->setText(_cpuText);
    ui->statusBar->addPermanentWidget(CPUusedLabel);

    versionActual = new QLabel(this);
    ui->statusBar->addPermanentWidget(versionActual);

    languageGroup = new QActionGroup(ui->menuLanguage);
    languageGroup->setExclusive(true);
    settingsGroup = new QActionGroup(ui->menuSettings);
    settingsGroup->setExclusive(true);
    helpGroup = new QActionGroup(ui->menuHelp);

    connect(ui->menuLanguage,SIGNAL(triggered(QAction*)),this,SLOT(slotLanguageChanged(QAction*)));
    connect(ui->menuSettings,SIGNAL(triggered(QAction*)),this,SLOT(slotSettingsChanged(QAction*)));
    connect(ui->menuHelp,SIGNAL(triggered(QAction*)),this,SLOT(slotHelpChanged(QAction*)));

    localErrorDialogHandler[ui->hddWidget] = new ErrorDialog(ui->hddWidget);
    bool status = GlobalSettings::getSettings()->getAutoUpdateSetting();
    ui->menuSettings->actions().at(2)->setChecked(status);
    if (status){
        timer = new QTimer(this);
        timer->setInterval(1000);
        timer->setTimerType(Qt::PreciseTimer);
        QObject::connect(timer,SIGNAL(timeout()),this,SLOT(timerTimeOut()));
        timer->start(1000);
    }
    else {
        versionActual->setText(tr("No data"));
    }
}

void MainWindow::timerTimeOut(){
    timerCounter++;
    if (timerCounter > 5){
        timer->stop();        
        QThread* thread = new QThread;

        QObject::connect(vcp,SIGNAL(analysed()),vcp,SLOT(deleteLater()));
        QObject::connect(vcp,SIGNAL(versionChecked(bool)),this,SLOT(versionChecked(bool)));

        QObject::connect(thread,SIGNAL(started()),vcp,SLOT(initiateDownload()));

        QObject::connect(vcp,SIGNAL(analysed()),thread,SLOT(quit()));
        QObject::connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));

        thread->start();
    }
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
        ui->hddWidget->addData(hddused);
    else if (hddused < 0.0 && !alreadyEvaluated){
        localErrorDialogHandler[ui->hddWidget]->evaluate("center","",0);
        localErrorDialogHandler[ui->hddWidget]->show(false);
        alreadyEvaluated = true;
    }
}

void MainWindow::onStopUpdatingWidget()
{
    emit stopUpdatingPowerWidget();
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
    if ((rLanguage == tr("English")) && currentLanguage != "EN"){
        switchTranslator("EN");
    }
    else if ((rLanguage == tr("Czech")) && currentLanguage != "CZ")
        switchTranslator("CZ");
}

void MainWindow::versionChecked(bool status){
    if (status){
        versionActual->setText(tr("Out of date"));
        versionActual->setStyleSheet("color: red");
    }
    else{
        versionActual->setText(tr("Up to date"));
        versionActual->setStyleSheet("color: green");
    }
}

void MainWindow::slotSettingsChanged(QAction* action){
    if (nullptr != action){//->text() == tr("Add HDD counter name")
        if (action->text() == tr("Add HDD counter name")){
            HDD_Settings* _hddSettings = new HDD_Settings;
            _hddSettings->setModal(true);
            _hddSettings->exec();
        }
        else if (action->text() == tr("Check for update")){
            VersionCheckerParent* vcp = new VersionCheckerParent;
            QThread* thread = new QThread;

            QObject::connect(vcp,SIGNAL(analysed()),vcp,SLOT(deleteLater()));
            QObject::connect(vcp,SIGNAL(versionChecked(bool)),this,SLOT(versionChecked(bool)));

            QObject::connect(thread,SIGNAL(started()),vcp,SLOT(initiateDownload()));

            QObject::connect(vcp,SIGNAL(analysed()),thread,SLOT(quit()));
            QObject::connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));

            thread->start();
        }
        else if (action->text() == tr("Check for update automatically")){
            GlobalSettings::getSettings()->setAutoUpdateSetting(action->isChecked());
            action->setChecked(action->isChecked());
        }
        else if (action->text() == tr("Define number of CPU cores")) {
            bool ok;
            int number = QInputDialog::getInt(this,tr("Number of CPU cores"),tr("Type number of CPU cores to be used: "),
                                              1,1,QThread::idealThreadCount(),1,&ok);
            if (ok) {
                GlobalSettings::getSettings()->setUsedCores(number);
                CPUusedLabel->setText(tr("Used cores: ")+QString::number(number,10));
            }
        }
    }
}

void MainWindow::slotHelpChanged(QAction* action) {
    if (action != nullptr) {
        if (action->text() == tr("About program")) {
            AboutProgram* info = new AboutProgram(QString::fromStdString(vcp->getActualVersion()),
                                                  GlobalSettings::getSettings()->getLanguage());
            info->open();
        }
        else if (action->text() == tr("GitHub repository")) {
            QDesktopServices::openUrl(QUrl("https://github.com/RaphCucul/Frames-registration"));
        }
        else if (action->text() == tr("Website")) {
            QDesktopServices::openUrl(QUrl("https://github.com/RaphCucul/FR_webpages"));
        }
    }
}

void MainWindow::onHddUsagePlotClicked(bool newStatus) {
    GlobalSettings::getSettings()->setHDDMonitorStatus(newStatus);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("The change will be applied after the application restart"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::closeEvent(QCloseEvent *e){
    emit stopUpdatingPowerWidget();
    QMainWindow::closeEvent(e);
    QApplication::quit();
}
