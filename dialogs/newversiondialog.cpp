#include "newversiondialog.h"
#include "ui_newversiondialog.h"

NewVersionDialog::NewVersionDialog(QString currentVersion,
                                   QString newVersion,
                                   QString descriptionText,
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewVersionDialog)
{
    ui->setupUi(this);

    ui->currentNo->setText(currentVersion);
    ui->newerNo->setText(newVersion);
    ui->description->setText(descriptionText);
}

NewVersionDialog::~NewVersionDialog()
{
    delete ui;
}

void NewVersionDialog::on_confirmButton_clicked()
{
    this->close();
}
