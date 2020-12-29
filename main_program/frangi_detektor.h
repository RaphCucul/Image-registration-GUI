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
 * @brief The Frangi_detektor class enables a user to test different Frangi parameters settings.
 *
 * A user can choose a video and a frame for testing. There is also an option to set margins of a frame which will be processed to eliminate and
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
    /**
     * @brief Starts frangi analysis.
     */
    void on_Frangi_filtr_clicked();
    /**
     * @brief Opens a file dialog so a user can choose a video for the analysis.
     *
     * Tries to find *.dat file of the video. If the file exists, tries to find frangi parameters saved in the file.
     * If no parameters are found, global parameters are used instead.
     */
    void on_fileToAnalyse_clicked();
    /**
     * @brief Applies the change of frame index.
     * @param arg1 - new frame index
     */
    void on_frameNumber_textChanged(const QString &arg1);
    /**
     * @brief Applies the change of a video name.
     *
     * The existence of the video is checked everytime when the video name is changed. If the video cannot be loaded,
     * the user cannot set important parameters.
     * @param arg1 - changed video name
     */
    void on_chosenFile_textChanged(const QString &arg1);
    /**
     * @brief Saves frangi parameters.
     */
    void onSaveFrangiData();
    /**
     * @brief Stores a margin value into an internal variable.
     * @param i_margin - current value of specific margin
     */
    void processMargins(int i_margin);
    /**
     * @brief Stores a ratio value into an internal variable.
     * @param i_ratio - current value of specific ratio
     */
    void processRatio(double i_ratio);
    /**
     * @brief Dynamically loads values of frangi parameters based on the user's choice - global or video specific parameters
     * (if exists).
     */
    void onFrangiSourceChosen();
    /**
     * @brief Opens ClickImage dialogue, when an "extra cutout" widget is checked.
     * @param checked - the current state of the checkbox
     */
    void openClickImageDialogue(int checked);
    /**
     * @brief Checks if the video has *.dat file and if so, tries to load saved "extra" cutout.
     *
     * If the "extra" cutout is not present, "extra cutout" widget is not checked. Otherwise, it is checked.
     */
    void checkSelectedCutout();
    /**
     * @brief Saves selected "extra" cutout.
     */
    void saveSelectedCutout();
    /**
     * @brief Saves selected referential frame of the selected video.
     */
    void saveChosenReferentialFrame();
signals:
    /**
     * @brief Indicates the start of frangi analysis. All widgets and tabs are disabled.
     */
    void calculationStarted();
    /**
     * @brief Indicates the finish of frangi analysis. All widgets and tabs are enabled.
     */
    void calculationStopped();
private:
    /**
     * @brief Loads specific frangi parameter value into the corresponding double spin box.
     * @param[in] i_doubleSpinboxName - identification name of the widget
     * @param[in] parameter - name of the parameter
     */
    void setParameter(QString i_doubleSpinboxName, QString parameter);

    /**
     * @brief Sets value of a double spin box.
     * @param[in] i_doubleSpinboxName - identification name of the widget
     * @param[in] i_value - a value of the parameter
     */
    void setParameter(QString i_doubleSpinboxName, double i_value);

    /**
     * @brief Saves the actual value of the double spin box during the saving process.
     * @param[in] i_type - type of the Frangi parameter
     * @param[in] i_parameterName - identification name of the widget
     */
    void getParameter(frangiType i_type, QString i_parameterName);

    /**
     * @brief Sets margins values.
     * @param[in] i_marginName - identification name of the widget
     * @param[in] i_value - a value of the margin
     */
    void setMargin(QString i_marginName, int i_value);

    /**
     * @brief Returns margins values.
     * @param[in] i_type - type of the Frangi parameter
     * @param[in] i_marginName - identification name of the widget
     */
    void getMargin(frangiType i_type, QString i_marginName);

    /**
     * @brief Sets ratios values.
     * @param[in] i_ratioName
     * @param[in] i_value
     */
    void setRatio(QString i_ratioName, double i_value);

    /**
     * @brief Returns ratios values.
     * @param[in] i_type
     * @param[in] i_ratioName
     */
    void getRatio(frangiType i_type, QString i_ratioName);

    /**
     * @brief Fills private QHash variables with corresponding widgets
     */
    void initWidgetHashes();

    /**
     * @brief Shows the chosen frame with the Frangi filter maximum mark and with the standard.
     * cutout rectangular.
     * @param[in] i_chosenFrame
     */
    void showStandardCutout(cv::Mat& i_chosenFrame);

    /**
     * @brief Checks the start and stop frangi parameter value. It is not acceptable to allow a user to set stop value
     * lower than start value.
     */
    void checkStartEndValues();

    /**
     * @brief checkPresenceOfExtraCutout
     * @param[in] i_filename
     * @param[in] videoInformationOnly
     * @return
     */
    bool checkPresenceOfExtraCutout(QString i_filename, bool videoInformationOnly);

    /**
     * @brief Loads frangi parameters - if the program is able to find video *.dat file and if the file has necessary information,
     *  video-related frangi parameters are loaded to the program. Otherwise, global-related parameters are loaded.
     * @param[in] i_type
     * @return True if loading video-related parameters was successful.
     */
    bool loadFrangiParametersForVideo(frangiType i_type);

    /**
     * @brief Changes "enabled" status of designated widgets.
     * @param[in] status
     */
    void changeStatus(bool status);

    /**
     * @brief Changes "enabled" status of saving buttons.
     * @param[in] status
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
