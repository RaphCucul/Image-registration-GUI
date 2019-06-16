#include "main_program/registratevideo.h"
#include "dialogs/clickimageevent.h"
#include "dialogs/singlevideoet.h"
#include "dialogs/multiplevideoet.h"
#include "dialogs/singlevideoregistration.h"
#include "dialogs/multivideoregistration.h"
#include "image_analysis/entropy.h"
#include "dialogs/grafet.h"
#include "ui_registratevideo.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <QLabel>
#include <QFileDialog>
#include <QSpacerItem>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QCoreApplication>

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
