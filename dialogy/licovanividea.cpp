#include "dialogy/licovanividea.h"
#include "dialogy/clickimageevent.h"
#include "dialogy/singlevideoet.h"
#include "dialogy/multiplevideoet.h"
#include "util/entropie.h"
#include "util/util_licovanividea.h"
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
    ui->metody->addTab(SVET,"SVET");
    ui->metody->addTab(MVET,"MVET");
}

LicovaniVidea::~LicovaniVidea()
{
    delete ui;
}

/*void LicovaniVidea::zobrazKliknutelnyDialog()
{
    /*QWidget* widgetCasZnacCB = ui->checkboxy->itemAtPosition(0,0)->widget();
    QCheckBox* CasZnacCB = qobject_cast<QCheckBox*>(widgetCasZnacCB);
    QWidget* widgetSvetAnomCB = ui->checkboxy->itemAtPosition(0,2)->widget();
    QCheckBox* SvetAnomCB = qobject_cast<QCheckBox*>(widgetSvetAnomCB);
    if (SvetAnomCB->isChecked() && volbaSvetAnomETSingle == false)
    {
        QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,1,1);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
        volbaSvetAnomETSingle = true;
    }
    if (CasZnacCB->isChecked() && volbaCasZnackyETSingle == false)
    {
        QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,1,2);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
        volbaCasZnackyETSingle = true;
    }
}*/
