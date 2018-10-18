#include "dialogy/singlevideoet.h"
#include "ui_singlevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "hlavni_program/t_b_ho.h"
#include "dialogy/grafet.h"
#include "util/souborove_operace.h"
#include "util/vicevlaknovezpracovani.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <QLabel>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

SingleVideoET::SingleVideoET(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleVideoET)
{
    ui->setupUi(this);
    ui->casovaZnacka->setEnabled(false);
    ui->svetelnaAnomalie->setEnabled(false);
    ui->vypocetET->setEnabled(false);
    ui->zobrazGrafET->setEnabled(false);

    if (videaKanalyzeAktual == "")
        ui->vybraneVideo->setPlaceholderText("Vybrane video");
    else
    {
        QString slozka,jmeno,koncovka;
        QStringList nalezeneSoubory;
        int pocetNalezenych;
        analyzuj_jmena_souboru_avi(videaKanalyzeAktual,nalezeneSoubory,pocetNalezenych,"avi");
        if (pocetNalezenych != 0)
        {
            QString celeJmeno = videaKanalyzeAktual+"/"+nalezeneSoubory.at(0);
            zpracujJmeno(celeJmeno,slozka,jmeno,koncovka);
            if (vybraneVideoETSingle.length() == 0)
            {
                vybraneVideoETSingle.push_back(slozka);
                vybraneVideoETSingle.push_back(jmeno);
                vybraneVideoETSingle.push_back(koncovka);
            }
            else
            {
                vybraneVideoETSingle.clear();
                vybraneVideoETSingle.push_back(slozka);
                vybraneVideoETSingle.push_back(jmeno);
                vybraneVideoETSingle.push_back(koncovka);
            }
            ui->vybraneVideo->setText(jmeno);
        }
    }
}

SingleVideoET::~SingleVideoET()
{
    delete ui;
}

void SingleVideoET::on_vyberVidea_clicked()
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
    ui->vybraneVideo->setText(vybraneVideoETSingle[1]);
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->vybraneVideo->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        ui->vybraneVideo->setStyleSheet("color: #00FF00");
        spravnostVideaETSingle = true;
        ui->vypocetET->setEnabled(true);
        int pocet_snimku = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        aktualniEntropie.fill(0.0,pocet_snimku);
        aktualniTennengrad.fill(0.0,pocet_snimku);
    }
}

void SingleVideoET::on_vybraneVideo_textChanged(const QString &arg1)
{
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+arg1+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->vybraneVideo->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
        spravnostVideaETSingle = false;
    }
    else
    {
        ui->vybraneVideo->setStyleSheet("color: #339900");
        spravnostVideaETSingle = true;
        vybraneVideoETSingle[1] = arg1;
    }
}

void SingleVideoET::on_casovaZnacka_stateChanged(int arg1)
{

}

void SingleVideoET::on_svetelnaAnomalie_stateChanged(int arg1)
{

}

void SingleVideoET::on_vypocetET_clicked()
{
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    /*cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    pocetSnimkuVidea = cap.get(CV_CAP_PROP_FRAME_COUNT);*/
    /*int uspech_analyzy = entropie_tennengrad_videa(cap,aktualniEntropie,aktualniTennengrad,ui->prubehVypoctu);
    if (uspech_analyzy == 0)
        qDebug()<<"Výpočty skončily chybou.";
    else
    {
        entropie.push_back(aktualniEntropie);
        tennengrad.push_back(aktualniTennengrad);
        ui->zobrazGrafET->setEnabled(true);
    }*/
    QStringList pom;
    pom.append(kompletni_cesta);
    vlaknoET = new VicevlaknoveZpracovani(pom);
    connect(vlaknoET,SIGNAL(percentageCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
    connect(vlaknoET,SIGNAL(hotovo()),this,SLOT(zpracovano()));
    vlaknoET->start();
}

void SingleVideoET::on_zobrazGrafET_clicked()
{
    QStringList vektorKzapisu;
    vektorKzapisu.append(vybraneVideoETSingle[1]);
    GrafET* graf_ET = new GrafET(entropie,tennengrad,vektorKzapisu,this);
    graf_ET->setModal(true);
    graf_ET->show();
}

void SingleVideoET::on_pushButton_clicked()
{
    QJsonDocument document;
    QJsonObject object;
    QVector<double> pomVecE = entropie[0];
    QVector<double> pomVecT = tennengrad[0];
    QJsonArray poleE = vector2array(pomVecE);
    QJsonArray poleT = vector2array(pomVecT);
    QString aktualJmeno = vybraneVideoETSingle[1];
    QString cesta = TXTulozeniAktual+"/"+aktualJmeno+".dat";
    object["entropie"] = poleE;
    object["tennengrad"] = poleT;
    document.setObject(object);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName(cesta);
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

void SingleVideoET::zpracovano()
{
    entropie = vlaknoET->vypocitanaEntropie();
    tennengrad = vlaknoET->vypocitanyTennengrad();
    ui->zobrazGrafET->setEnabled(true);
}
