#include "licovanividea.h"
#include "ui_licovanividea.h"

#include <QDebug>
#include <QLineEdit>
#include <QPushButton>

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

void LicovaniVidea::clearLayout(QGridLayout *layout)
{
    int pocetSLoupcu = layout->columnCount();
    int pocetRadku = layout->rowCount();
    qDebug()<<"pocetSLoupcu: "<<pocetSLoupcu<<" pocetRadku: "<<pocetRadku;
    for (int a = 1; a <= pocetRadku; a++)
    {
        for (int b = 1; b <= pocetSLoupcu; b++)
        {
            QWidget* widget = layout->itemAtPosition(a-1,b-1)->widget();
            layout->removeWidget(widget);
            delete widget;
        }
    }
}

void LicovaniVidea::on_comboBox_activated(int index)
{
    if (index == 0)
    {
        if (predchozi_index != 0)
            clearLayout(ui->nabidkaAnalyzy);

        predchozi_index = 0;
    }
    else if (index == 1)
    {
        if (predchozi_index != 0)
            clearLayout(ui->nabidkaAnalyzy);
        predchozi_index = 1;
        QLineEdit* vybraneVideoLE = new QLineEdit(this);
        QPushButton* vyberVideaPB = new QPushButton(this);
        QLineEdit* cisloReferenceLE = new QLineEdit(this);
        QLineEdit* cisloPosunutehoLE = new QLineEdit(this);
    }
}
