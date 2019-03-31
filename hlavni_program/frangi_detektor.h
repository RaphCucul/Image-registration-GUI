#ifndef FRANGI_DETEKTOR_H
#define FRANGI_DETEKTOR_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QJsonObject>
#include <opencv2/opencv.hpp>
#include "dialogy/errordialog.h"

namespace Ui {
class Frangi_detektor;
}

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
    void changeValue_slider_start();
    void changeValue_slider_end();
    void changeValue_slider_step();
    void changeValue_slider_one();
    void changeValue_slider_two();

    void on_Frangi_filtr_clicked();
    void on_fileToAnalyse_clicked();
    void on_frameNumber_textChanged(const QString &arg1);
    void on_chosenFile_textChanged(const QString &arg1);
    void on_saveParameters_clicked();

private:
    /**
     * @brief Function loads frangi parameters data into the corresponding double spin box.
     */
    void setDSBInput(QDoubleSpinBox*_spinbox, QString parameter);

    /**
     * @brief Functions save the actual value of the double spin box during the saving process.
     * @param parameter
     */
    void getDSBInput(QString parameter);

    /**
     * @brief Function loads frangi parameters data into the corresponding slider.
     * @param _slider
     * @param parameter
     */
    void analyseSliderInput(QSlider* _slider, QString parameter);

    void setSliderValue(QDoubleSpinBox* _spinbox, QSlider *_slider);

    int analyseFrame = -1;
    Ui::Frangi_detektor *ui;
    int minimum = 0;
    int maximum = 30;
    QVector<QString> analyseChosenFile;
    cv::VideoCapture actualVideo;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QHash<QString,QDoubleSpinBox*> spinBoxes;
    QHash<QString,QSlider*> sliders;
    QStringList frangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
};

#endif // FRANGI_DETEKTOR_H
