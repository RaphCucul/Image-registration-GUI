#ifndef FRANGI_DETEKTOR_H
#define FRANGI_DETEKTOR_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QJsonObject>
#include <opencv2/opencv.hpp>

namespace Ui {
class Frangi_detektor;
}

extern cv::Point3d detekovane_frangiho_maximum;

class Frangi_detektor : public QWidget
{
    Q_OBJECT

public:
    explicit Frangi_detektor(QWidget *parent = nullptr);
    ~Frangi_detektor();
    void checkPaths();
    void setParametersToUI();
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

    void on_cisloSnimku_textChanged(const QString &arg1);

private:
    int analyzujSnimek = -1;
    Ui::Frangi_detektor *ui;
    int minimum = 0;
    int maximum = 30;
    QVector<QString> rozborVybranehoSouboru;
    QVector<double> FrangiParametrySouboru;
    QJsonObject nacteneFrangihoParametry;

};

#endif // FRANGI_DETEKTOR_H
