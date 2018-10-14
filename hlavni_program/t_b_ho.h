#ifndef T_B_HO_H
#define T_B_HO_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include <QStringList>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include "mainwindow.h"
namespace Ui {
class t_b_HO;
}

extern QString videaKanalyzeAktual;
extern QString ulozeniVideiAktual;
extern QString TXTnacteniAktual;
extern QString TXTulozeniAktual;

class t_b_HO : public QWidget
{
    Q_OBJECT

public:
    QStringList seznam_cest_prazdny=(QStringList()<<"-");
    int pocet_mist_combobox = 3;
    explicit t_b_HO(QWidget *parent = nullptr);
    QJsonObject readJson(QFile& soubor);
    void writeJson(QJsonArray pole, QString typ);
    void ziskejCestyZJson(QJsonArray& poleCest,QComboBox* box, QStringList& list);


    ~t_b_HO();
private slots:
    void on_cesta_k_videim_clicked();
    void on_ulozeni_videa_clicked();
    void on_txt_slozka_clicked();
    void on_ulozeni_txt_clicked();
    void on_ulozeni_cest_clicked();
    void vybranaCesta(int index);
    void vybranaCestaString(QString cesta);

private:
    Ui::t_b_HO *ui;
    QVector<int> zaplnenost_CB_cesta_k_videi;
    QJsonObject souborScestami;
    QJsonArray videaKanalyze;
    QJsonArray ulozeniVidei;
    QJsonArray TXTnacteni;
    QJsonArray TXTulozeni;
    QStringList videaKanalyzeList;
    QStringList ulozeniVideiList;
    QStringList TXTnacteniList;
    QStringList TXTulozeniList;
};

#endif // T_B_HO_H
