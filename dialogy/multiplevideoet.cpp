#include "multiplevideoet.h"
#include "ui_multiplevideoet.h"

MultipleVideoET::MultipleVideoET(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultipleVideoET)
{
    ui->setupUi(this);
}

MultipleVideoET::~MultipleVideoET()
{
    delete ui;
}
