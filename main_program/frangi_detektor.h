#ifndef FRANGI_DETEKTOR_H
#define FRANGI_DETEKTOR_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QJsonObject>
#include <opencv2/opencv.hpp>
#include "dialogs/errordialog.h"
#include "shared_staff/sharedvariables.h"

using namespace frangiEnums;

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

    /**
     * @brief setParametersToUI
     */
    void setParametersToUI();

protected:

private slots:
    void on_Frangi_filtr_clicked();
    void on_fileToAnalyse_clicked();
    void on_frameNumber_textChanged(const QString &arg1);
    void on_chosenFile_textChanged(const QString &arg1);
    void onSaveFrangiData();
    void processMargins(int i_margin);
    void processRatio(double i_ratio);
    void onFrangiSourceChosen();

signals:
    void calculationStarted();
    void calculationStopped();
private:
    /**
     * @brief Function loads frangi parameters data into the corresponding double spin box.
     */
    void setParameter(QString i_doubleSpinboxName, QString parameter);

    /**
     * @brief setParameter
     * @param _spinbox
     * @param i_value
     */
    void setParameter(QString i_doubleSpinboxName, double i_value);

    /**
     * @brief Functions save the actual value of the double spin box during the saving process.
     * @param parameter
     */
    void getParameter(frangiType i_type, QString i_parameterName);

    void setMargin(QString i_marginName, int i_value);

    void getMargin(frangiType i_type, QString i_marginName);

    void setRatio(QString i_ratioName, double i_value);

    void getRatio(frangiType i_type, QString i_ratioName);

    /**
     * @brief Function fills the private QHash variables with corresponding widgets
     */
    void initWidgetHashes();

    /**
     * @brief Function plots the chosen frame with the Frangi filter maximum mark and with the standard
     * cutout rectangular.
     */
    void showStandardCutout(cv::Mat& i_chosenFrame);

    /**
     * @brief checkStartEndValues
     */
    void checkStartEndValues();

    /**
     * @brief loadFrangiParametersForVideo
     * @param i_type
     * @return
     */
    bool loadFrangiParametersForVideo(frangiType i_type);

    /**
     * @brief enableWidgets
     */
    void enableWidgets();

    /**
     * @brief disableWidgets
     */
    void disableWidgets();

    int analyseFrame = -1;
    Ui::Frangi_detektor *ui;
    int minimum = 0;
    int maximum = 30;
    QMap<QString,QString> analyseChosenFile;
    cv::VideoCapture actualVideo;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QHash<QString,QDoubleSpinBox*> spinBoxes;
    QHash<QString,QSpinBox*> marginSpinBoxes;
    QHash<QString,QDoubleSpinBox*> ratioSpinBoxes;
    QStringList frangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two"};
    QStringList MarginsRatiosList = {"left_m","right_m","top_m","bottom_m","left_r","right_r","top_r","bottom_r"};
    QMap<QString,int> frangiMargins;
    QMap<QString,double> cutoutRatios;
    bool loading = true;
    frangiType chosenFrangiType = frangiType::GLOBAL;
};

#endif // FRANGI_DETEKTOR_H
