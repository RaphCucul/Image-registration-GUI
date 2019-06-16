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
#include "dialogs/errordialog.h"

namespace Ui {
class Frangi_detektor;
}

class Frangi_detektor : public QWidget
{
    Q_OBJECT

public:
    explicit Frangi_detektor(QWidget *parent = nullptr);
    ~Frangi_detektor();

    /**
     * @brief The function checks, if the path to the video folder exists. If so, the first video from the list of files
     * found in the folder is placed into the line edit.
     */
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

    void processMargins(int i_margin);
    void processRatio(double i_ratio);

signals:
    void calculationStarted();
    void calculationStopped();
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

    /**
     * @brief Function sets the given value to Slider and double spin boxes for synchronized changes.
     * @param _spinbox
     * @param _slider
     */
    void setSliderValue(QDoubleSpinBox* _spinbox, QSlider *_slider);

    /**
     * @brief Function fills the private QHash variables with corresponding widgets
     */
    void initWidgetHashes();

    /**
     * @brief Function plots the chosen frame with the Frangi filter maximum mark and with the standard
     * cutout rectangular.
     */
    void showStandardCutout(cv::Mat& i_chosenFrame);

    void checkStartEndValues();

    int analyseFrame = -1;
    Ui::Frangi_detektor *ui;
    int minimum = 0;
    int maximum = 30;
    QVector<QString> analyseChosenFile;
    cv::VideoCapture actualVideo;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QHash<QString,QDoubleSpinBox*> spinBoxes;
    QHash<QString,QSlider*> sliders;
    QHash<QString,QSpinBox*> marginSpinBoxes;
    QHash<QString,QDoubleSpinBox*> ratioSpinBoxes;
    QStringList frangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    QStringList MarginsRatiosList = {"left_m","right_m","top_m","bottom_m","left_r","right_r","top_r","bottom_r"};
    QMap<QString,int> frangiMargins;
    QMap<QString,double> cutoutRatios;
    bool loading = true;
    bool readyToCalculate = false;
};

#endif // FRANGI_DETEKTOR_H
