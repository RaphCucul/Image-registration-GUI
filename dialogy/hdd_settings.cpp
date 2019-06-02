#include "hdd_settings.h"
#include "ui_hdd_settings.h"

HDD_Settings::HDD_Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HDD_Settings)
{
    ui->setupUi(this);
}

HDD_Settings::~HDD_Settings()
{
    delete ui;
}
