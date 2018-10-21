#include "dialogy/singlevideolicovani.h"
#include "ui_singlevideolicovani.h"
#include "hlavni_program/t_b_ho.h"
#include "util/licovaniparent.h"
#include "util/souborove_operace.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QVector>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>

SingleVideoLicovani::SingleVideoLicovani(QWidget *parent) : LicovaniParent(parent),
    ui(new Ui::SingleVideoLicovani)
{
    ui->setupUi(this);
    ui->slicovatPB->setEnabled(false);
    ui->grafickeHodnoceniPB->setEnabled(false);
    ui->slozkaParametruPB->setEnabled(false);
    ui->ulozitPB->setEnabled(false);
    ui->zobrazVysledekLicovaniPB->setEnabled(false);
}

SingleVideoLicovani::~SingleVideoLicovani()
{
    delete ui;
}

void SingleVideoLicovani::on_vybraneVideo_textChanged(const QString &arg1)
{
    QString kompletni_cesta = vybraneVideoLicovaniSingle[0]+"/"+arg1+"."+vybraneVideoLicovaniSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->vybraneVideo->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        ui->vybraneVideo->setStyleSheet("color: #339900");
        vybraneVideoLicovaniSingle[1] = arg1;
    }
}

void SingleVideoLicovani::on_vyberVidea_clicked()
{
    QString kompletnicesta = QFileDialog::getOpenFileName(this,
         "Vyberte referenční obrázek", videaKanalyzeAktual,"*.avi;;Všechny soubory (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    zpracujJmeno(kompletnicesta,vybrana_slozka,vybrany_soubor,koncovka);
    /*int lastindexSlash = referencniObrazek.lastIndexOf("/");
    int lastIndexComma = referencniObrazek.length() - referencniObrazek.lastIndexOf(".");
    QString vybrana_slozka = referencniObrazek.left(lastindexSlash);
    QString vybrany_soubor = referencniObrazek.mid(lastindexSlash+1,
         (referencniObrazek.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = referencniObrazek.right(lastIndexComma-1);*/

    cv::VideoCapture cap = cv::VideoCapture(kompletnicesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->vybraneVideo->setText(vybrany_soubor);
        ui->vybraneVideo->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        if (vybraneVideoLicovaniSingle.length() == 0)
        {
            vybraneVideoLicovaniSingle.push_back(vybrana_slozka);
            vybraneVideoLicovaniSingle.push_back(vybrany_soubor);
            vybraneVideoLicovaniSingle.push_back(koncovka);
        }
        else
        {
            vybraneVideoLicovaniSingle.clear();
            vybraneVideoLicovaniSingle.push_back(vybrana_slozka);
            vybraneVideoLicovaniSingle.push_back(vybrany_soubor);
            vybraneVideoLicovaniSingle.push_back(koncovka);
        }
        ui->vybraneVideo->setText(vybraneVideoLicovaniSingle[1]);
        ui->vybraneVideo->setStyleSheet("color: #339900");
        ui->slozkaParametruPB->setEnabled(true);
        QString dir = TXTnacteniAktual;
        QDir chosenDirectory(dir);
        QStringList JsonInDirectory = chosenDirectory.entryList(QStringList() << "*.dat" << "*.DAT",QDir::Files);
        for (int a = 0; a < JsonInDirectory.count(); a++)
        {
            if (JsonInDirectory.at(a) == (vybraneVideoLicovaniSingle[1]+".dat"))
            {
                if (vybranyJSONLicovaniSingle.length() == 0)
                {
                    vybranyJSONLicovaniSingle.push_back(TXTnacteniAktual);
                    vybranyJSONLicovaniSingle.push_back(vybraneVideoLicovaniSingle[1]);
                    vybranyJSONLicovaniSingle.push_back("dat");
                }
                else
                {
                    vybranyJSONLicovaniSingle.clear();
                    vybranyJSONLicovaniSingle.push_back(TXTnacteniAktual);
                    vybranyJSONLicovaniSingle.push_back(vybraneVideoLicovaniSingle[1]);
                    vybranyJSONLicovaniSingle.push_back("dat");
                }
                ui->slozkaParametruLE->setText(vybraneVideoLicovaniSingle[1]);
            }
            else
            {
                ui->slozkaParametruLE->setText("Vybrany parametricky soubor");
                ui->slozkaParametruPB->setEnabled(true);
            }
        }

    }
}

void SingleVideoLicovani::on_slozkaParametruLE_textChanged(const QString &arg1)
{

}

void SingleVideoLicovani::on_slozkaParametruPB_clicked()
{
    QString kompletnicesta = QFileDialog::getOpenFileName(this,
                                                          "Vyberte soubor s parametry", videaKanalyzeAktual,"*.avi;;Všechny soubory (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    zpracujJmeno(kompletnicesta,vybrana_slozka,vybrany_soubor,koncovka);
    QString dir = TXTnacteniAktual;
    QDir chosenDirectory(dir);
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.json" << "*.JSON",QDir::Files);

    ui->slozkaParametruLE->setStyleSheet("color: #339900");
    if (vybranyJSONLicovaniSingle.length() == 0)
    {
        vybranyJSONLicovaniSingle.push_back(vybrana_slozka);
        vybranyJSONLicovaniSingle.push_back(vybrany_soubor);
        vybranyJSONLicovaniSingle.push_back(koncovka);
    }
    else
    {
        vybranyJSONLicovaniSingle.clear();
        vybranyJSONLicovaniSingle.push_back(vybrana_slozka);
        vybranyJSONLicovaniSingle.push_back(vybrany_soubor);
        vybranyJSONLicovaniSingle.push_back(koncovka);
    }
    ui->vybraneVideo->setText(vybranyJSONLicovaniSingle[1]);


}

void SingleVideoLicovani::on_slicovatPB_clicked()
{

}

void SingleVideoLicovani::on_zobrazVysledekLicovaniPB_clicked()
{

}

void SingleVideoLicovani::on_grafickeHodnoceniPB_clicked()
{

}

void SingleVideoLicovani::on_ulozitPB_clicked()
{

}
