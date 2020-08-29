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

/**
 * @class Frangi_detektor
 * @brief The Frangi_detektor class enables a user to test different Frangi parameters settings. A user can choose
 * a video and a frame for testing. There is also an option to set margins of a frame which will be processed to eliminate and
 * existing artefact in this area.
 */
class Frangi_detektor : public QWidget
{
    Q_OBJECT

public:
    explicit Frangi_detektor(QWidget *parent = nullptr);
    ~Frangi_detektor();

    /**
     * @brief Checks, if the path to the video folder exists. If so, the first video from the list of files
     * found in the folder is placed into the line edit.
     */
    void checkPaths();

    /**
     * @brief Loads values of frangi parameters to appropriate widgets.
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
    void openClickImageDialogue(int checked);
    void checkSelectedCutout();
    void saveSelectedCutout();
    void saveChosenReferentialFrame();
signals:
    void calculationStarted();
    void calculationStopped();
private:
    /**
     * @brief Loads specific frangi parameter value into the corresponding double spin box.
     * @param i_doubleSpinboxName
     * @param parameter
     */
    void setParameter(QString i_doubleSpinboxName, QString parameter);

    /**
     * @brief Sets value of a double spin box.
     * @param i_doubleSpinboxName
     * @param i_value
     */
    void setParameter(QString i_doubleSpinboxName, double i_value);

    /**
     * @brief Saves the actual value of the double spin box during the saving process.
     * @param i_type
     * @param i_parameterName
     */
    void getParameter(frangiType i_type, QString i_parameterName);

    /**
     * @brief Sets margins values.
     * @param i_marginName
     * @param i_value
     */
    void setMargin(QString i_marginName, int i_value);

    /**
     * @brief Returns margins values.
     * @param i_type
     * @param i_marginName
     */
    void getMargin(frangiType i_type, QString i_marginName);

    /**
     * @brief Sets ratios values.
     * @param i_ratioName
     * @param i_value
     */
    void setRatio(QString i_ratioName, double i_value);

    /**
     * @brief Returns ratios values.
     * @param i_type
     * @param i_ratioName
     */
    void getRatio(frangiType i_type, QString i_ratioName);

    /**
     * @brief Fills private QHash variables with corresponding widgets
     */
    void initWidgetHashes();

    /**
     * @brief Shows the chosen frame with the Frangi filter maximum mark and with the standard.
     * cutout rectangular.
     * @param i_chosenFrame
     */
    void showStandardCutout(cv::Mat& i_chosenFrame);

    /**
     * @brief Checks the start and stop frangi parameter value. It is not acceptable to allow a user to set stop value
     * lower than start value.
     */
    void checkStartEndValues();

    /**
     * @brief checkPresenceOfExtraCutout
     * @param i_filename
     * @param videoInformationOnly
     * @return
     */
    bool checkPresenceOfExtraCutout(QString i_filename, bool videoInformationOnly);

    /**
     * @brief Loads frangi parameters - if the program is able to find video *.dat file and if the file has necessary information,
     *  video-related frangi parameters are loaded to the program. Otherwise, global-related parameters are loaded.
     * @param i_type
     * @return True if loading video-related parameters was successful.
     */
    bool loadFrangiParametersForVideo(frangiType i_type);

    /**
     * @brief Changes "enabled" status of designated widgets.
     * @param status
     */
    void changeStatus(bool status);

    /**
     * @brief Changes "enabled" status of saving buttons.
     * @param status
     */
    void changeSaveButtonsStatus(bool status);

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
    bool loading = true,previousExtraCutoutFound = false;
    frangiType chosenFrangiType = frangiType::VIDEO_SPECIFIC;
    cv::Rect extraCutout;
};

#endif // FRANGI_DETEKTOR_H
