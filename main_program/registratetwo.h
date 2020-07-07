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
 * format bmt, jpg and png). A user can choose what should be registrated - frames or images.
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
     * @param layout.
     */
    void clearLayout(QGridLayout *layout);

    /**
     * @brief Checks, if the path to the video folder exists. If so, the first video from the list of files
     * found in the folder is placed into the line edit.
     */
    void checkPaths();
protected:
public slots:
    void chosenVideoPB_clicked();
    void chosenReferenceImgPB_clicked();
    void chosenTranslatedImgPB_clicked();
    void ReferenceImgLE_textChanged(const QString &arg1);
    void TranslatedImgLE_textChanged(const QString &arg1);

signals:
    void checkRegistrationPass();
    void calculationStarted();
    void calculationStopped();

private slots:
    void Slot_VideoLE_textChanged(const QString &s);
    void ReferenceLE_textChanged(const QString &arg1);
    void TranslatedLE_textChanged(const QString &arg1);
    void on_areaMaximum_editingFinished();
    void on_rotationAngle_editingFinished();
    void on_iterationCount_editingFinished();
    void on_registrateTwo_clicked();
    void showDialog();
    void evaluateCorrectValues();
    void on_comboBox_activated(int index);

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
     * @param i_analysedFolder
     * @param i_whereToSave
     */
    void analyseAndSave(QString i_analysedFolder, QMap<QString, QString> &i_whereToSave);

    /**
     * @brief Checks, if input video or images can be loaded for next processing step.
     * @param i_path
     * @param i_method
     */
    bool evaluateVideoImageInput(QString i_path,QString i_method);

    /**
     * @brief Checks if the input values of area maximum, rotation angle and iteration count
     * are correct and if so, it calls the signal to check if all inputs are correct and the button of the main
     * computation algorithm can be then enabled.
     * @param input
     * @param lower
     * @param upper
     * @param editWidget
     * @param finalValue
     * @param evaluation
     */
    void checkInputNumber(double i_input,double i_lower,double i_upper,
                          QLineEdit* i_editWidget,double& i_finalValue,bool& i_evaluation);

    QMap<QString,QString> chosenVideoAnalysis;
    QMap<QString,QString> chosenReferencialImgAnalysis;
    QMap<QString,QString> chosenTranslatedImgAnalysis;
    cv::VideoCapture cap;
    cv::Mat referencialImg,translatedImg;

    int referencialNumber = -1;
    int translatedNumber = -1;
    double iteration = -99.0;
    double areaMaximum = -99.0;
    double angle = -99.0;
    QString chosenVideo="";

    bool videoCorrect = false;
    bool referenceCorrect = false;
    bool referencialImgCorrect = false;
    bool translatedImgCorrect = false;
    bool translatedCorrect = false;
    bool areaMaximumCorrect = false;
    bool angleCorrect = false;
    bool iterationCorrect = false;
    bool anomalyPresent = false;
    bool scaleChanged = false;
    bool firstChoiceInitialised = false;
    bool secondChoiceInitialised = false;

    QLineEdit* chosenVideoLE;
    QPushButton* chosenVideoPB;
    QLineEdit* referenceNoLE;
    QLineEdit* translatedNoLE;

    QSpacerItem* horizontalSpacer1;
    QSpacerItem* horizontalSpacer2;

    QPushButton* chooseReferencialImagePB;
    QPushButton* chooseTranslatedImagePB;
    QLineEdit* referenceImgLE;
    QLineEdit* translatedImgLE;

    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    frangiType typeOfFrangi = frangiType::GLOBAL;
    chosenSource whatIsAnalysed = chosenSource::VIDEO;
};

#endif // REGISTRATETWO_H
