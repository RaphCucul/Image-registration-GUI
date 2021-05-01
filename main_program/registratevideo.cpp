#include "main_program/registratevideo.h"
#include "dialogs/singlevideoet.h"
#include "dialogs/multiplevideoet.h"
#include "dialogs/singlevideoregistration.h"
#include "dialogs/multivideoregistration.h"
#include "ui_registratevideo.h"

#include <QDebug>
RegistrateVideo::RegistrateVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegistrateVideo)
{
    ui->setupUi(this);


    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    SingleVideoET* SVET = new SingleVideoET();
    MultipleVideoET* MVET = new MultipleVideoET();
    SingleVideoRegistration* SVreg = new SingleVideoRegistration();
    MultiVideoRegistration* MVreg = new MultiVideoRegistration();
    ui->metody->addTab(SVET,"SVET");
    ui->metody->addTab(MVET,"MVET");
    ui->metody->addTab(SVreg,"SVreg");
    ui->metody->addTab(MVreg,"MVreg");
    ui->metody->tabBar()->setExpanding(true);
    int tabWidth = (this->width()/4.1);
    ui->metody->setStyleSheet("QTabWidget::pane {border-style: none;}"
                              "QTabWidget::tab-bar {left: 1px;}"
                              "QTabBar::tab {"
                                "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                    "stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
                                    "stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);"
                                    "padding: 1px;"
                                    "height: 32px;"
                                    "border-radius: 4px;"
                                    "border: 1px solid black;"
                                    "width: " + QString::number(tabWidth) + "px; }"
                               "QTabBar::tab:selected{"
                                    "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                    "stop: 0 #b3d9ff, stop: 0.4 #99ccff,"
                                    "stop: 0.5 #80bfff, stop: 1.0 #4da6ff);"
                                "}"
                                "QTabBar::tab:hover{"
                                    "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                    "stop: 0 #f14d36, stop: 0.4 #FB644F,"
                                    "stop: 0.5 #FB644F, stop: 1.0 #f14d36);}");

    QObject::connect(SVreg,SIGNAL(calculationStarted()),this,SLOT(disableTabs()));
    QObject::connect(SVreg,SIGNAL(calculationStopped()),this,SLOT(enableTabs()));
    QObject::connect(this,SIGNAL(checkFilePaths()),SVreg,SLOT(checkPaths()));

    QObject::connect(MVreg,SIGNAL(calculationStarted()),this,SLOT(disableTabs()));
    QObject::connect(MVreg,SIGNAL(calculationStopped()),this,SLOT(enableTabs()));

    QObject::connect(SVET,SIGNAL(calculationStarted()),this,SLOT(disableTabs()));
    QObject::connect(SVET,SIGNAL(calculationStopped()),this,SLOT(enableTabs()));
    QObject::connect(this,SIGNAL(checkFilePaths()),SVET,SLOT(checkPaths()));

    QObject::connect(MVET,SIGNAL(calculationStarted()),this,SLOT(disableTabs()));
    QObject::connect(MVET,SIGNAL(calculationStopped()),this,SLOT(enableTabs()));

}

RegistrateVideo::~RegistrateVideo()
{
    delete ui;
}

void RegistrateVideo::enableTabs(){
    for (int var = 0; var < ui->metody->tabBar()->count(); var++) {
        if (var != ui->metody->currentIndex())
            ui->metody->setTabEnabled(var,true);
    }
    emit calculationStopped();
}

void RegistrateVideo::disableTabs(){
    for (int var = 0; var < ui->metody->tabBar()->count(); var++) {
        if (var != ui->metody->currentIndex())
            ui->metody->setTabEnabled(var,false);
    }
    emit calculationStarted();
}

void RegistrateVideo::checkPathinitialization(){
    emit checkFilePaths();
}

void RegistrateVideo::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e)
    int tabWidth = (this->width()/4.1);
    ui->metody->setStyleSheet( ui->metody->styleSheet() +
                                        "QTabBar::tab {"
                                        "width: " + QString::number(tabWidth) + "px; }" );
}
