#ifndef FRANGI_DETEKTOR_H
#define FRANGI_DETEKTOR_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>

namespace Ui {
class Frangi_detektor;
}

extern cv::Point3d detekovane_frangiho_maximum;

class Frangi_detektor : public QWidget
{
    Q_OBJECT

public:
    void inicializace_frangi_opt(QString &hodnota,int &pozice);
    double data_z_frangi_opt(int pozice);
    void velikost_frangi_opt(int velikost);
    explicit Frangi_detektor(QWidget *parent = nullptr);
    ~Frangi_detektor();

private slots:   
    void on_sigma_start_sliderMoved(int value);
    void on_sigma_end_sliderMoved(int value);
    void on_sigma_step_sliderMoved(int value);
    void on_beta_one_sliderMoved(int value);
    void on_beta_two_sliderMoved(int value);
    void zmena_hodnoty_slider_start();
    void zmena_hodnoty_slider_end();
    void zmena_hodnoty_slider_step();
    void zmena_hodnoty_slider_one();
    void zmena_hodnoty_slider_two();

    void on_Frangi_filtr_clicked();

    void on_souborKAnalyzePB_clicked();

private:
    Ui::Frangi_detektor *ui;
    int minimum = 0;
    int maximum = 30;
    QVector<QString> rozborVybranehoSouboru;
    QVector<double> FrangiParametrySouboru;
};

#endif // FRANGI_DETEKTOR_H
