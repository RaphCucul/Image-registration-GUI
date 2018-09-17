#include "licovanividea.h"
#include "ui_licovanividea.h"

LicovaniVidea::LicovaniVidea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LicovaniVidea)
{
    ui->setupUi(this);
}

LicovaniVidea::~LicovaniVidea()
{
    delete ui;
}
