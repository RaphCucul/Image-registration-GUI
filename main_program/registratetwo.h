#ifndef REGISTRATETWO_H
#define REGISTRATETWO_H

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QGridLayout>
#include <QJsonObject>

#include "dialogs/errordialog.h"
#include "shared_staff/sharedvariables.h"

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>

using namespace frangiEnums;
using namespace clickImageEnums;

namespace Ui {
class RegistrateTwo;
}

/**
 * @class RegistrateTwo
 * @brief The RegistrateTwo class provides widgets and functions to registrate two frames of the video or two images (with
 * format bmt, jpg and png).
 *
 * A user can choose what should be registrated - frames or images.
 */
class RegistrateTwo : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrateTwo(QWidget *parent = nullptr);
    ~RegistrateTwo();

    /**
     * @brief Clears options from Box layout connected to the combobox. When a different option in combobox is selected, this
     * function is called to clear the content of the box layout to enable loading of new widgets.
     * @param layout - specific grid layout on the GUI
     */
    void clearLayout(QGridLayout *layout);

    /**
     * @brief Checks, if the path to the video folder exists. If so, the first video from the list of files
     * found in the folder is placed into the line edit.
     */
    void checkPaths();
protected:
public slots:
    /**
     * @brief Opens a file dialogue to let a user choose an avi video file for analysis.
     */
    void chosenVideoPB_clicked();
    /**
     * @brief Opens a file dialogue to let a user choose an bmp/pnh/jpg file (as a referential) for analysis.
     */
    void chosenReferenceImgPB_clicked();
    /**
     * @brief Opens a file dialogue to let a user choose an bmp/pnh/jpg file (as a shifted) for analysis.
     */
    void chosenTranslatedImgPB_clicked();
    /**
     * @brief Checks if the image (as a referential) with given name exists.
     * @param arg1 - name of the image
     */
    void ReferenceImgLE_textChanged(const QString &arg1);
    /**
     * @brief Checks if the image (as a shifted) with given name exists.
     * @param arg1 - name of the image
     */
    void TranslatedImgLE_textChanged(const QString &arg1);

signals:
    /**
     * @brief Instructs a program to check if all necessary parameters are filled to enable/disable final widgets.
     * @sa evaluateCorrectValues()
     */
    void checkRegistrationPass();
    /**
     * @brief Indicates start of the registration process. All widgets and tabs are disabled.
     */
    void calculationStarted();
    /**
     * @brief Indicates stop of the registration process. All widgets and tabs are enabled.
     */
    void calculationStopped();

private slots:
    /**
     * @brief Checks if the video with given name exists.
     * @param s - new name of the video
     */
    void Slot_VideoLE_textChanged(const QString &s);
    /**
     * @brief Checks if the frame index representing referential frame is usable.
     *
     * If the frame index is lower than 0 or bigger than total number of frames of the video, the number turns red.
     * If the video was analysed before and a *.dat file exists, it tries to check what the referential frame is in this *.dat file
     * and informs a user if the actual frame index does not correspond with the index of the referential frame in the file.
     * @param arg1 - new index
     */
    void ReferenceLE_textChanged(const QString &arg1);
    /**
     * @brief Checks if the frame index representing shifted frame is usable.
     * @param arg1 - new index
     */
    void TranslatedLE_textChanged(const QString &arg1);
    /**
     * @brief Checks if the "areaMaximum" parameter fits the criteria.
     *
     * range <1.0,20.0>
     */
    void on_areaMaximum_editingFinished();
    /**
     * @brief Checks if the "rotationAngle" parameter fits the criteria.
     *
     * range <0.0,5.0>
     */
    void on_rotationAngle_editingFinished();
    /**
     * @brief Checks if the "iterationCount" parameter fits the criteria.
     *
     * If -1, the program itself set the iteration count. Otherwise, range from 1.0 to infinite.
     */
    void on_iterationCount_editingFinished();
    /**
     * @brief Starts the registration process.
     */
    void on_registrateTwo_clicked();
    /**
     * @brief Creates a ClickImageEvent class object to enable a user to mark standard/extra cutout before the
     * registration process starts.
     */
    void showDialog();
    /**
     * @brief Checks if all necessary inputs (maximum area, iteration count and rotation angle limit) are correct and
     * disables/enables the rest of widgets.
     */
    void evaluateCorrectValues();
    /**
     * @brief Dynamically changes the layout of widgets - a user can quickly choose between images or video analysis.
     * @param index - actual chosen index of the combo list
     *
     * @sa initChoiceOneInnerWidgets
     * @sa placeChoiceOneWidgets()
     * @sa initChoiceTwoInnerWidgets()
     * @sa placeChoiceTwoWidgets()
     */
    void on_selectInput_activated(int index);

private:
    Ui::RegistrateTwo *ui;

    /**
     * @brief Initalizes all widgets from "choice one" option
     *  which may appear in the horizontal box layout.
     */
    void initChoiceOneInnerWidgets();

    /**
     * @brief Initalizes all widgets from "choice two" option
     *  which may appear in the horizontal box layout.
     */
    void initChoiceTwoInnerWidgets();

    /**
     * @brief Populates the layout box with widgets used for frames registration.
     */
    void placeChoiceOneWidgets();

    /**
     * @brief Populates the layout box with widgets used for image registration.
     */
    void placeChoiceTwoWidgets();

    /**
     * @brief Checks the content of the folder to discover images (specific formats) and places the name of the first discovered
     * image to appropriate widget.
     * @param i_analysedFolder - complete folder path
     * @param i_whereToSave - QMap dedicated to store file info
     */
    void analyseAndSave(QString i_analysedFolder, QMap<QString, QString> &i_whereToSave);

    /**
     * @brief Checks, if input video or images can be loaded for next processing step.
     * @param i_path - a path to the object (video, picture)
     * @param i_method - video, referentialImage, translatedImage
     */
    bool evaluateVideoImageInput(QString i_path,QString i_method);

    /**
     * @brief Checks if the input values of area maximum, rotation angle and iteration count
     * are correct and if so, it calls the signal to check if all inputs are correct and the button of the main
     * computation algorithm can be then enabled.
     * @param i_input - original number from the specific widget
     * @param i_lower - acceptable minimum value
     * @param i_upper - acceptable maximum value
     * @param i_editWidget - specific widget with the number
     * @param i_finalValue - returned value (-99 if the input does not fit criteria)
     * @param i_evaluation - true if the input was accepted
     */
    void checkInputNumber(double i_input,double i_lower,double i_upper,
                          QLineEdit* i_editWidget,double& i_finalValue,bool& i_evaluation);

    /**
     * @brief When a user finishes a cutout selection, the function checks if the values are correct.
     *
     * If the parameter is null or width or height of the cutout are 0 or less, the corresponding checkbox is unchecked
     * and the program will not work with this cutout at all.
     * @param i_type
     */
    void checkSelectedCutout(cutoutType i_type);

    QMap<QString,QString> chosenVideoAnalysis;
    QMap<QString,QString> chosenReferentialImgAnalysis;
    QMap<QString,QString> chosenTranslatedImgAnalysis;
    cv::VideoCapture cap;
    cv::Mat referentialImg,translatedImg;

    int referentialNumber = -1;
    int translatedNumber = -1;
    double iteration = -99.0;
    double areaMaximum = -99.0;
    double angle = -99.0;
    QString chosenVideo="";

    bool videoCorrect = false;
    bool referenceCorrect = false;
    bool referentialImgCorrect = false;
    bool translatedImgCorrect = false;
    bool translatedCorrect = false;
    bool areaMaximumCorrect = false;
    bool angleCorrect = false;
    bool iterationCorrect = false;
    bool anomalyPresent = false;
    bool scaleChanged = false;
    bool firstChoiceInitialised = false;
    bool secondChoiceInitialised = false;
    bool standardLoaded = false,extraLoaded = false;

    QLineEdit* chosenVideoLE;
    QPushButton* chosenVideoPB;
    QLineEdit* referenceNoLE;
    QLineEdit* translatedNoLE;

    QSpacerItem* horizontalSpacer1;
    QSpacerItem* horizontalSpacer2;

    QPushButton* chooseReferentialImagePB;
    QPushButton* chooseTranslatedImagePB;
    QLineEdit* referenceImgLE;
    QLineEdit* translatedImgLE;

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    frangiType typeOfFrangi = frangiType::GLOBAL;
    chosenSource whatIsAnalysed = chosenSource::VIDEO;
};

#endif // REGISTRATETWO_H
