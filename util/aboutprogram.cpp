#include "aboutprogram.h"
#include "ui_aboutprogram.h"

#include <QFile>
#include <QApplication>
#include <QTextStream>

AboutProgram::AboutProgram(QString i_version, QString i_language, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutProgram)
{
    ui->setupUi(this);
    ui->version->setText(i_version);
    QFile file(QApplication::applicationDirPath()+"\\"+i_language+"_releaseNotes.txt");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        QString text = in.readAll();
        file.close();
        ui->releaseNotes->setText(text);
    }
    else {

        ui->releaseNotes->setText("");
    }
    this->setModal(false);
}

AboutProgram::~AboutProgram()
{
    delete ui;
}
