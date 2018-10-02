#ifndef SINGLEVIDEOET_H
#define SINGLEVIDEOET_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <QVector>
namespace Ui {
class SingleVideoET;
}

class SingleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit SingleVideoET(QWidget *parent = nullptr);
    ~SingleVideoET();

private slots:
    void on_vyberVidea_clicked();
    void on_vybraneVideo_textChanged(const QString &arg1);
    void on_casovaZnacka_stateChanged(int arg1);
    void on_svetelnaAnomalie_stateChanged(int arg1);
    void on_vypocetET_clicked();
    void on_zobrazGrafET_clicked();

private:
    Ui::SingleVideoET *ui;
    QString vybraneVideoETSingle;
    bool spravnostVideaETSingle = false;
    bool volbaCasZnackyETSingle = false;
    bool volbaSvetAnomETSingle = false;
    QVector<double> aktualniEntropie, aktualniTennengrad;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
};

#endif // SINGLEVIDEOET_H
