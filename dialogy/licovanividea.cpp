#include "dialogy/licovanividea.h"
#include "dialogy/clickimageevent.h"
#include "util/entropie.h"
#include "util/util_licovanividea.h"
#include "dialogy/grafet.h"
#include "ui_licovanividea.h"

#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <QLabel>
#include <QFileDialog>
#include <QSpacerItem>

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
    //int citacItemu = 0;
    for (int a = 0; a < pocetRadku; a++)
    {
        for (int b = 0; b < pocetSLoupcu; b++)
        {
            QWidget* widget = layout->itemAtPosition(a,b)->widget();
            if (widget != nullptr)
            {
                layout->removeWidget(widget);
                delete widget;
            }
            else
            {
                QSpacerItem* item = layout->itemAtPosition(a,b)->spacerItem();
                layout->removeItem(item);
                delete item;
            }

        }
    }
}

void LicovaniVidea::on_comboBox_activated(int index)
{
    if (index == 0)
    {
        if (predchozi_index != 0)
        {
            if (predchozi_index == 1)
            {
                clearLayout(ui->checkboxy);
                clearLayout(ui->vyberVidea);
                clearLayout(ui->metody);
            }
        }


        predchozi_index = 0;
    }
    else if (index == 1)
    {
        /*if (predchozi_index != 0)
            clearLayout(ui->nabidkaAnalyzy);*/

        predchozi_index = 1;       
        ET_SingleVideoAnalysis(ui->checkboxy,ui->vyberVidea,ui->metody);
        ///
        QWidget* widgetvyberVideaPB = ui->vyberVidea->itemAtPosition(0,3)->widget();
        QPushButton* vybVidPB = qobject_cast<QPushButton*>(widgetvyberVideaPB);
        QWidget* widgetvybraneVideoLE = ui->vyberVidea->itemAtPosition(0,1)->widget();
        QLineEdit* vybVidLE = qobject_cast<QLineEdit*>(widgetvybraneVideoLE);
        QSignalMapper *VyberVideaPBMapper = new QSignalMapper(this);
        connect(vybVidPB,SIGNAL(clicked()),VyberVideaPBMapper,SLOT(map()));
        VyberVideaPBMapper->setMapping(vybVidPB,vybVidLE);
        QObject::connect(VyberVideaPBMapper, SIGNAL(mapped(QWidget *)),this, SLOT(vybVidPB_clicked(QWidget *)));
        ///
        QWidget* widgetCasZnacCB = ui->checkboxy->itemAtPosition(0,0)->widget();
        QCheckBox* CasZnacCB = qobject_cast<QCheckBox*>(widgetCasZnacCB);
        QObject::connect(CasZnacCB,SIGNAL(stateChanged(int)),this,SLOT(zobrazKliknutelnyDialog()));
        QWidget* widgetSvetAnCB = ui->checkboxy->itemAtPosition(0,2)->widget();
        QCheckBox* SvetAnCB = qobject_cast<QCheckBox*>(widgetSvetAnCB);
        QObject::connect(SvetAnCB,SIGNAL(stateChanged(int)),this,SLOT(zobrazKliknutelnyDialog()));
        ///
        QWidget* widgetETPB = ui->metody->itemAtPosition(0,1)->widget();
        QWidget* widgetZobrazVysledkyPB = ui->metody->itemAtPosition(1,1)->widget();
        QPushButton* EplusTPB = qobject_cast<QPushButton*>(widgetETPB);
        QPushButton* ZVPB = qobject_cast<QPushButton*>(widgetZobrazVysledkyPB);
        QObject::connect(EplusTPB,SIGNAL(clicked()),this,SLOT(EplusTPB_clicked()));
        QObject::connect(ZVPB,SIGNAL(clicked()),this,SLOT(ZVPB_clicked()));

    }
}
void LicovaniVidea::zobrazKliknutelnyDialog()
{
    QWidget* widgetCasZnacCB = ui->checkboxy->itemAtPosition(0,0)->widget();
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
}

void LicovaniVidea::vybVidPB_clicked(QWidget* W)
{
    QString referencniObrazek = QFileDialog::getOpenFileName(this,
         "Vyberte referenční obrázek", "","*.avi;;Všechny soubory (*)");
    int lastindexSlash = referencniObrazek.lastIndexOf("/");
    int lastIndexComma = referencniObrazek.length() - referencniObrazek.lastIndexOf(".");
    QString vybrana_slozka = referencniObrazek.left(lastindexSlash);
    QString vybrany_soubor = referencniObrazek.mid(lastindexSlash+1,
         (referencniObrazek.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = referencniObrazek.right(lastIndexComma-1);
    if (vybraneVideoETSingle.length() == 0)
    {
        vybraneVideoETSingle.push_back(vybrana_slozka);
        vybraneVideoETSingle.push_back(vybrany_soubor);
        vybraneVideoETSingle.push_back(koncovka);
    }
    else
    {
        vybraneVideoETSingle.clear();
        vybraneVideoETSingle.push_back(vybrana_slozka);
        vybraneVideoETSingle.push_back(vybrany_soubor);
        vybraneVideoETSingle.push_back(koncovka);
    }
    QLineEdit* vybVidLE = qobject_cast<QLineEdit*>(W);
    vybVidLE->setText(vybraneVideoETSingle[1]);
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        vybVidLE->setStyleSheet("QLineEdit#VideoLE{color: #FF0000}");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        vybVidLE->setStyleSheet("color: #339900");
        spravnostVideaETSingle = true;
        QWidget* widgetCasZnacCB = ui->checkboxy->itemAtPosition(0,0)->widget();
        QCheckBox* CasZnacCB = qobject_cast<QCheckBox*>(widgetCasZnacCB);
        CasZnacCB->setEnabled(true);
        QWidget* widgetSvetAnCB = ui->checkboxy->itemAtPosition(0,2)->widget();
        QCheckBox* SvetAnCB = qobject_cast<QCheckBox*>(widgetSvetAnCB);
        SvetAnCB->setEnabled(true);

        QWidget* widgetETPB = ui->metody->itemAtPosition(0,1)->widget();
        QPushButton* EplusTPB = qobject_cast<QPushButton*>(widgetETPB);
        EplusTPB->setEnabled(true);

    }
}

void LicovaniVidea::EplusTPB_clicked()
{
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    int uspech_analyzy = entropie_tennengrad_videa(cap,hodnoty_entropie,hodnoty_tennengrad,ui->prubehVypoctu);
    if (uspech_analyzy == 0)
        qDebug()<<"Výpočty skončily chybou.";
    else
    {
        QWidget* widgetZobrazVysledkyPB = ui->metody->itemAtPosition(1,1)->widget();
        QPushButton* ZVPB = qobject_cast<QPushButton*>(widgetZobrazVysledkyPB);
        ZVPB->setEnabled(true);
    }
}

void LicovaniVidea::ZVPB_clicked()
{
    GrafET* graf_ET = new GrafET(hodnoty_entropie,hodnoty_tennengrad,this);
    graf_ET->setModal(true);
    graf_ET->show();
}
