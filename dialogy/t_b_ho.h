#ifndef T_B_HO_H
#define T_B_HO_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include <QStringList>
#include <QVector>
#include "mainwindow.h"
namespace Ui {
class t_b_HO;
}

class t_b_HO : public QWidget
{
    Q_OBJECT

public:
    QStringList seznam_cest_prazdny=(QStringList()<<"-");
    QStringList ziskej_seznam_cest(QComboBox* box);
    int pocet_mist_combobox = 3;
    explicit t_b_HO(QWidget *parent = 0);
    void zapis_cest_do_txt(QTextStream &s,int pmCB,QStringList &seznam);
    void nacteni_cest_z_txt(QString &l, QComboBox *box, int &poc, int &pmCB);
    ~t_b_HO();
private slots:
    void on_cesta_k_videim_clicked();
    void on_ulozeni_videa_clicked();
    void on_txt_slozka_clicked();
    void on_ulozeni_txt_clicked();
    void on_ulozeni_cest_clicked();
    void zmena_stavu_CB(int index_zmeny);

    //void on_CB_cesta_k_videim_currentIndexChanged(int index);

private:
    Ui::t_b_HO *ui;
    QVector<int> zaplnenost_CB_cesta_k_videi;

};

#endif // T_B_HO_H
