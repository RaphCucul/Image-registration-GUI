#include "hlavni_program/t_b_ho.h"
#include "ui_t_b_ho.h"
#include "util/souborove_operace.h"

#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QSettings>

QString videaKanalyzeAktual;
QString ulozeniVideiAktual;
QString TXTnacteniAktual;
QString TXTulozeniAktual;

t_b_HO::t_b_HO(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::t_b_HO)
{
    ui->setupUi(this);

    // načtení dat v případě, že se ve složce nachází soubor s uloženými cestami
    //QFile soubor(QDir::currentPath()+"/"+"cesty.txt");
    QFile file;
    file.setFileName("D:/Qt_projekty/Licovani_videa_GUI/seznamCest.json");
    souborScestami = readJson(file);
    videaKanalyze = souborScestami["cestaKvideim"].toArray();
    ulozeniVidei = souborScestami["ulozeniVidea"].toArray();
    TXTnacteni = souborScestami["adresarTXT_nacteni"].toArray();
    TXTulozeni = souborScestami["adresarTXT_ulozeni"].toArray();

    //ui->CB_cesta_k_videim->addItem(videaKanalyze[0].toString());
    ziskejCestyZJson(videaKanalyze,ui->CB_cesta_k_videim,videaKanalyzeList);
    ziskejCestyZJson(ulozeniVidei,ui->CB_ulozeni_videa,ulozeniVideiList);
    ziskejCestyZJson(TXTnacteni,ui->CB_slozka_txt,TXTnacteniList);
    ziskejCestyZJson(TXTulozeni,ui->CB_ulozeni_txt,TXTulozeniList);

    videaKanalyzeAktual = videaKanalyzeList.at(0);
    ulozeniVideiAktual = ulozeniVideiList.at(0);
    TXTnacteniAktual = TXTnacteniList.at(0);
    TXTulozeniAktual = TXTulozeniList.at(0);

    connect(ui->CB_cesta_k_videim,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    connect(ui->CB_cesta_k_videim,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->CB_ulozeni_videa,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    connect(ui->CB_ulozeni_videa,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->CB_slozka_txt,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    connect(ui->CB_slozka_txt,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->CB_ulozeni_txt,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    connect(ui->CB_ulozeni_txt,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
}

void t_b_HO::on_cesta_k_videim_clicked()
{
    QString cesta_k_videim = QFileDialog::getExistingDirectory(this,"Vyberte složku obsahující videa",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_videim);
    videaKanalyze.append(novaCesta);
    int velikostPole = videaKanalyze.size();
    QString t = "D:/Qt_projekty/Licovani_videa_GUI/seznamCest.json";
    writeJson(souborScestami,videaKanalyze,"cestaKvideim",t);
    ui->CB_cesta_k_videim->addItem(videaKanalyze[velikostPole-1].toString());

    //qDebug()<<videaKanalyze;
}

void t_b_HO::on_ulozeni_videa_clicked()
{
    QString cesta_k_ulozeni_videi = QFileDialog::getExistingDirectory(this,"Vyberte složku pro uložení slícovaného videa",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_ulozeni_videi);
    ulozeniVidei.append(novaCesta);
    int velikostPole = ulozeniVidei.size();
    QString t = "D:/Qt_projekty/Licovani_videa_GUI/seznamCest.json";
    writeJson(souborScestami,ulozeniVidei,"ulozeniVidea",t);
    ui->CB_ulozeni_videa->addItem(ulozeniVidei[velikostPole-1].toString());
}

void t_b_HO::on_txt_slozka_clicked()
{
    QString cesta_k_txt = QFileDialog::getExistingDirectory(this,"Vyberte složku obsahující txt soubory",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_txt);
    TXTnacteni.append(novaCesta);
    int velikostPole = TXTnacteni.size();
    QString t = "D:/Qt_projekty/Licovani_videa_GUI/seznamCest.json";
    writeJson(souborScestami,TXTnacteni,"adresarTXT_nacteni",t);
    ui->CB_ulozeni_videa->addItem(TXTnacteni[velikostPole-1].toString());
}

void t_b_HO::on_ulozeni_txt_clicked()
{
    QString cesta_k_ulozeni_txt = QFileDialog::getExistingDirectory(this,"Vyberte složku pro uložení txt souborů",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_ulozeni_txt);
    TXTulozeni.append(novaCesta);
    int velikostPole = TXTulozeni.size();
    QString t = "D:/Qt_projekty/Licovani_videa_GUI/seznamCest.json";
    writeJson(souborScestami,TXTulozeni,"adresarTXT_ulozeni",t);
    ui->CB_ulozeni_videa->addItem(TXTulozeni[velikostPole-1].toString());
}

t_b_HO::~t_b_HO()
{
    delete ui;
}

void t_b_HO::on_ulozeni_cest_clicked()
{
    souborScestami["cestaKvideim"] = videaKanalyze;
    souborScestami[ "ulozeniVidea"] = ulozeniVidei;
    souborScestami["adresarTXT_nacteni"] = TXTnacteni;
    souborScestami["adresarTXT_ulozeni"] = TXTulozeni;
    QJsonDocument document;
    document.setObject(souborScestami);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName("D:/Qt_projekty/Licovani_videa_GUI/seznamCest.json");
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

void t_b_HO::ziskejCestyZJson(QJsonArray& poleCest, QComboBox *box, QStringList &list)
{
    int velikostPole = poleCest.size();
    if (velikostPole == 1)
    {
        box->addItem(poleCest[0].toString());
        list.append(poleCest[0].toString());
    }
    if (velikostPole > 1)
    {
        QStringList seznamCest;
        for (int a = 0; a < velikostPole; a++)
        {
            QString pom = poleCest[a].toString();
            seznamCest.append(pom);
        }
        box->addItems(seznamCest);
        list = seznamCest;
    }
    if (velikostPole == 0)
    {
        box->addItem("");
        list.append("");
    }
}
void t_b_HO::vybranaCesta(int index)
 {
     if (QObject::sender() == ui->CB_cesta_k_videim)
         videaKanalyzeAktual = videaKanalyzeList.at(index).toLocal8Bit().constData();
     if (QObject::sender() == ui->CB_ulozeni_videa)
         ulozeniVideiAktual = ulozeniVideiList.at(index).toLocal8Bit().constData();
     if (QObject::sender() == ui->CB_slozka_txt)
         TXTnacteniAktual = TXTnacteniList.at(index).toLocal8Bit().constData();
     if (QObject::sender() == ui->CB_ulozeni_txt)
         TXTulozeniAktual = TXTulozeniList.at(index).toLocal8Bit().constData();

     //qDebug()<<index<<videaKanalyzeAktual;
 }

void t_b_HO::vybranaCestaString(QString cesta)
{
    if (QObject::sender() == ui->CB_cesta_k_videim)
        videaKanalyzeAktual = cesta;
    if (QObject::sender() == ui->CB_ulozeni_videa)
        ulozeniVideiAktual = cesta;
    if (QObject::sender() == ui->CB_slozka_txt)
        TXTnacteniAktual = cesta;
    if (QObject::sender() == ui->CB_ulozeni_txt)
        TXTulozeniAktual = cesta;

    //qDebug()<<cesta;
}
