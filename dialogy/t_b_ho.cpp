#include "t_b_ho.h"
#include "ui_t_b_ho.h"
#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QLineEdit>
#include <fstream>
#include <iostream>

t_b_HO::t_b_HO(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::t_b_HO)
{
    ui->setupUi(this);

    // načtení dat v případě, že se ve složce nachází soubor s uloženými cestami
    QFile soubor(QDir::currentPath()+"/"+"cesty.txt");
    if (soubor.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&soubor);
        int pocitadlo = 0;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (pocitadlo < 3)
            {
                nacteni_cest_z_txt(line,ui->CB_cesta_k_videim,pocitadlo,pocet_mist_combobox);
            }
            if (pocitadlo >=3 && pocitadlo < 6)
            {
                nacteni_cest_z_txt(line,ui->CB_ulozeni_videa,pocitadlo,pocet_mist_combobox);
            }
            if (pocitadlo >=6 && pocitadlo < 9)
            {
                nacteni_cest_z_txt(line,ui->CB_slozka_txt,pocitadlo,pocet_mist_combobox);
            }
            if (pocitadlo >= 9 && pocitadlo < 12)
            {
                nacteni_cest_z_txt(line,ui->CB_ulozeni_txt,pocitadlo,pocet_mist_combobox);
            }
            pocitadlo+=1;
        }
    }
    else
    {
        ui->CB_cesta_k_videim->addItems(seznam_cest_prazdny); // cesta k videim
        ui->CB_ulozeni_videa->addItems(seznam_cest_prazdny); // cesta k ulozeni videi
        ui->CB_slozka_txt->addItems(seznam_cest_prazdny); // cesta k txt souborum
        ui->CB_ulozeni_txt->addItems(seznam_cest_prazdny); // cesta k ulozeni txt

    }
}

void t_b_HO::on_cesta_k_videim_clicked()
{
    QString cesta_k_videim = QFileDialog::getExistingDirectory(0,"Vyberte složku obsahující videa",QDir::currentPath());
    if (ui->CB_cesta_k_videim->itemText(0) == '-'){
        ui->CB_cesta_k_videim->insertItem(0,cesta_k_videim);
        QObject::connect(ui->CB_cesta_k_videim, QOverload<int>::of(&QComboBox::currentIndexChanged),
                         [=](int index = 0){zmena_stavu_CB(index);});
    }
    else if (ui->CB_cesta_k_videim->itemText(1) == '-' && ui->CB_cesta_k_videim->itemText(0) != '-'){
        ui->CB_cesta_k_videim->insertItem(1,cesta_k_videim);
        QObject::connect(ui->CB_cesta_k_videim, QOverload<int>::of(&QComboBox::currentIndexChanged),
                         [=](int index = 1){zmena_stavu_CB(index);});
    }
    else if (ui->CB_cesta_k_videim->itemText(2) == '-' && ui->CB_cesta_k_videim->itemText(1) != '-'
             && ui->CB_cesta_k_videim->itemText(0) != '-'){
        ui->CB_cesta_k_videim->insertItem(2,cesta_k_videim);
        QObject::connect(ui->CB_cesta_k_videim, QOverload<int>::of(&QComboBox::currentIndexChanged),
                         [=](int index = 2){zmena_stavu_CB(index);});
    }

}

void t_b_HO::on_ulozeni_videa_clicked()
{
    QString cesta_k_ulozeni_videi = QFileDialog::getExistingDirectory(0,"Vyberte složku pro uložení slícovaného videa",QDir::currentPath());
    ui->CB_ulozeni_videa->insertItem(1,cesta_k_ulozeni_videi);
}

void t_b_HO::on_txt_slozka_clicked()
{
    QString cesta_k_txt = QFileDialog::getExistingDirectory(0,"Vyberte složku obsahující txt soubory",QDir::currentPath());
    ui->CB_slozka_txt->insertItem(1,cesta_k_txt);
}

void t_b_HO::on_ulozeni_txt_clicked()
{
    QString cesta_k_ulozeni_txt = QFileDialog::getExistingDirectory(0,"Vyberte složku pro uložení txt souborů",QDir::currentPath());
    ui->CB_ulozeni_txt->insertItem(1,cesta_k_ulozeni_txt);
}

QStringList t_b_HO::ziskej_seznam_cest(QComboBox *box)
{
    QStringList itemsInComboBox;
    for (int index = 0; index < box->count(); index++)
    {itemsInComboBox << box->itemText(index);}
    return itemsInComboBox;
}

t_b_HO::~t_b_HO()
{
    delete ui;
}

void t_b_HO::on_ulozeni_cest_clicked()
{
    QString slozka_s_programem = QDir::currentPath();
    QString jmeno_souboru_cest=slozka_s_programem + "/" + "cesty.txt";
    QFile soubor(jmeno_souboru_cest);

    QStringList seznam_combobox1 = ziskej_seznam_cest(ui->CB_cesta_k_videim);
    QStringList seznam_combobox2 = ziskej_seznam_cest(ui->CB_ulozeni_videa);
    QStringList seznam_combobox3 = ziskej_seznam_cest(ui->CB_slozka_txt);
    QStringList seznam_combobox4 = ziskej_seznam_cest(ui->CB_ulozeni_txt);

    if ( soubor.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream stream( &soubor );
        zapis_cest_do_txt(stream,pocet_mist_combobox,seznam_combobox1);
        zapis_cest_do_txt(stream,pocet_mist_combobox,seznam_combobox2);
        zapis_cest_do_txt(stream,pocet_mist_combobox,seznam_combobox3);
        zapis_cest_do_txt(stream,pocet_mist_combobox,seznam_combobox4);
    }
}
void t_b_HO::zmena_stavu_CB(int index_zmeny){
    index_zmeny+=1;
}
void t_b_HO::zapis_cest_do_txt(QTextStream &s, int pmCB, QStringList &seznam)
{
    if (seznam.length() == 1)
    {
        for (int a = 0; a < pmCB; a++)
        {
            s << '-' << endl;
        }
    }
    else if (seznam.length() == 2)
    {
        for (int c = 0; c < 2; c++)
        {
            s << seznam.at(c) << endl;
        }
        s << '-' << endl;
    }
    else if (seznam.length() == 3 || seznam.length() > 3){
        for (int b = 0; b < pmCB; b++)
        {
            s << seznam.at(b) << endl;
        }
    }
}
void t_b_HO::nacteni_cest_z_txt(QString &l, QComboBox *box, int &poc, int &pmCB)
{
    if (l == '-'){
        box->insertItem(0,l);
        if (poc%pmCB == 0) poc+=3;
        if (poc%pmCB == 1) poc+=2;
        if (poc%pmCB == 2) poc+=1;
    }
    else{
        box->insertItem(0,l);
    }
}
