#include "hlavni_program/licovanividea.h"
#include "dialogy/clickimageevent.h"
#include "dialogy/singlevideoet.h"
#include "dialogy/multiplevideoet.h"
#include "dialogy/singlevideolicovani.h"
#include "dialogy/multivideolicovani.h"
#include "analyza_obrazu/entropie.h"
//#include "util/util_licovanividea.h"
#include "dialogy/grafet.h"
#include "ui_licovanividea.h"

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

LicovaniVidea::LicovaniVidea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LicovaniVidea)
{
    ui->setupUi(this);
    SingleVideoET* SVET = new SingleVideoET();
    MultipleVideoET* MVET = new MultipleVideoET();
    SingleVideoLicovani* SVreg = new SingleVideoLicovani();
    MultiVideoLicovani* MVreg = new MultiVideoLicovani();
    ui->metody->addTab(SVET,"SVET");
    ui->metody->addTab(MVET,"MVET");
    ui->metody->addTab(SVreg,"SVreg");
    ui->metody->addTab(MVreg,"MVreg");

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    QObject::connect(SVreg,SIGNAL(calculationStarted()),this,SLOT(disableTabs()));
    QObject::connect(SVreg,SIGNAL(calculationStopped()),this,SLOT(enableTabs()));
}

LicovaniVidea::~LicovaniVidea()
{
    delete ui;
}

void LicovaniVidea::enableTabs(){
    for (int var = 0; var < ui->metody->tabBar()->count(); var++) {
        qDebug()<<"Calculating in the index "<<ui->metody->currentIndex();
        if (var != ui->metody->currentIndex())
            ui->metody->setTabEnabled(var,true);
    }
    emit calculationStopped();
}

void LicovaniVidea::disableTabs(){
    for (int var = 0; var < ui->metody->tabBar()->count(); var++) {
        qDebug()<<"Calculating in the index "<<ui->metody->currentIndex();
        if (var != ui->metody->currentIndex())
            ui->metody->setTabEnabled(var,false);
    }
    emit calculationStarted();
}

/*void LicovaniVidea::zobrazKliknutelnyDialog()
{
    QWidget* widgetCasZnacCB = ui->checkboxy->itemAtPosition(0,0)->widget();
    QCheckBox* CasZnacCB = qobject_cast<QCheckBox*>(widgetCasZnacCB);
    QWidget* widgetSvetAnomCB = ui->checkboxy->itemAtPosition(0,2)->widget();
    QCheckBox* SvetAnomCB = qobject_cast<QCheckBox*>(widgetSvetAnomCB);
    if (SvetAnomCB->isChecked() && verticalAnomalySelected == false)
    {
        QString kompletni_cesta = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,1,1);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
        verticalAnomalySelected = true;
    }
    if (CasZnacCB->isChecked() && horizontalAnomalySelected == false)
    {
        QString kompletni_cesta = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,1,2);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
        horizontalAnomalySelected = true;
    }
}*/
