#ifndef CLICKIMAGEEVENT_H
#define CLICKIMAGEEVENT_H

#include <QDialog>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QPoint>
#include <QPixmap>
#include <QImage>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QMap>
#include <QRadioButton>
#include <QButtonGroup>

#include <opencv2/opencv.hpp>
#include "shared_staff/sharedvariables.h"
#include "dialogs/errordialog.h"
#include <functional>
using namespace clickImageEnums;

/**
 * @class IntegratedFrangiOptions
 * @brief The IntegratedFrangiOptions class is an independent widget which can be displayed in a ClickImageEvent dialog.
 * The widgets enables to change frangi parameters during the selection of a cutout. The widget also enables to temporally set or
 * save selected parameters. When saving, parameters are always saved to video-related *.dat file.
 */
class IntegratedFrangiOptions : public QWidget
{
    Q_OBJECT

public:
    IntegratedFrangiOptions();
    ~IntegratedFrangiOptions();
    /**
     * @brief Returns or set values of each widget of the class.
     * @param method - determines if parameters will be set or returned
     * @param m - margins
     * @param r - ratios
     * @param p - parameters
     */
    void GetSetValues(QString method, QMap<QString,int>& m, QMap<QString,double>& r, QMap<QString,double>& p);

    /**
     * @brief Returns actual margins values.
     */
    QMap<QString,int> getIntegratedMargins() {return marginsInternal;}

    /**
     * @brief Returns actual parameters values.
     */
    QMap<QString,double> getInternalParameters() {return parametersInternal;}

    /**
     * @brief Returns actual ratios values.
     */
    QMap<QString,double> getInternalRatios() {return ratiosInternal;}
public slots:
    void saveButtonClicked();
    void setButtonClicked();
signals:
    void frangiParametersSelected(QMap<QString,int> _margins,QMap<QString,double> ratios,QMap<QString,double> _parameters);
    void applyFrangiParameters();
private:
    /**
     * @brief Initializes the widget. All necessary elements are added to the empty widget's grid layout.
     */
    void createWidget();

    /**
     * @brief Helper function, adds a widget to grid layout of the class.
     * @param widget - a widget to be added
     * @param row
     * @param col
     */
    void addWidgetToGrid(QWidget* widget,int row,int col);

    QStringList FrangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    QStringList FrangiMarginsList = {"top_m","bottom_m","left_m","right_m"};
    QStringList FrangiRatiosList = {"top_r","bottom_r","left_r","right_r"};
    QStringList labelsTexts = {tr("top margin"),tr("bottom margin"),tr("left margin"),tr("right margin"),
                               tr("top ration"),tr("bottom ratio"),tr("left ration"),tr("right ration"),
                               tr("sigma start"),tr("sigma stop"),tr("sigma step"),tr("beta one"),tr("beta two"),tr("zpracovani")};

    QPushButton* setParameters = nullptr;
    QPushButton* saveParameters = nullptr;
    QRadioButton* reverse = nullptr;
    QRadioButton* standard = nullptr;
    QGridLayout* layoutForIntegratedFrangi = nullptr;
    QMap<QString,int> marginsInternal;
    QMap<QString,double> parametersInternal;
    QMap<QString,double> ratiosInternal;
    QMap<QString,QSpinBox*> marginsConnections;
    QMap<QString,QDoubleSpinBox*> ratiosConnections;
    QMap<QString,QDoubleSpinBox*> parametersConnections;
    QMap<QString,QLabel*> labels;
    QMap<QString,QString> labelsConnections;
    QButtonGroup *BG = nullptr;
};

namespace Ui {
class ClickImageEvent;
}

/**
 * @class ClickImageEvent
 * @brief The ClickImageEvent class enables to select standard or extra cutout of the frame. The part of the program from where
 * ClickImageEvent is called affects the options what can be selected in the dialog.
 * 1. If called from SVET tab when a video or image is analysed, it is not possible to change analysed video and frame.
 * 2. If called from RegistrateTwo, it is also not possible to change analysed video and frame.
 * 3. If called from MVET tab when a videos are analysed, it is possible to choose the video to be analysed and the specific
 * frame. This settings can be changed anytime.
 */
class ClickImageEvent : public QDialog
{
    Q_OBJECT

public:
    ClickImageEvent(QString i_fullPath,
                    cutoutType i_cutoutType,
                    bool i_referentialFrameFound = false,
                    QDialog *parent = nullptr);
    ClickImageEvent(QString i_fullPath,
                    int i_referFrameNo,
                    cutoutType i_cutoutType,
                    bool i_referentialFrameFound = false,
                    chosenSource i_chosenType= chosenSource::VIDEO,
                    QDialog *parent = nullptr);
    ClickImageEvent(QStringList i_fullPaths, cutoutType i_type, QDialog *parent = nullptr);
    ~ClickImageEvent() override;    
public slots:
    /**
     * @brief Saves selected parameters. If a frame of a video is analysed, parameters are marked as video-related. If an image
     * is analysed, parameters are marked as global-related.
     * @param _margins
     * @param _ratios
     * @param _parameters
     */
    void saveSelectedParameters(QMap<QString,int> _margins,QMap<QString,double> _ratios, QMap<QString,double> _parameters);

    /**
     * @brief Initializes drawing of the graphics content.
     * @param i_calculatedData
     * @sa drawGraphicsScene(QPoint i_result)
     */
    void obtainFrangiCoordinates(QPoint i_calculatedData);

    /**
     * @brief Provices a reaction on IntegratedFrangiOptions-related button click.
     */
    void onShowFrangiOptions();

    /**
     * @brief Calls the function to start frangi analysis.
     * @sa startFrangiAnalysis()
     */
    void onApplyFrangiParameters();
protected:
    void mousePressEvent(QMouseEvent *press) override;
    void mouseMoveEvent(QMouseEvent *move) override;
    void mouseReleaseEvent(QMouseEvent *release) override;
    void closeEvent(QCloseEvent *e) override;
signals:
    void saveVideoCutout();
    void dataExtracted();
private slots:
    /**
     * @brief Processes the input of the referential frame line edit and enables the QGraphicsView object if
     * the frame number is correct and can be loaded.
     */
    void referencialFrameChosen();

    /**
     * @brief Processes the chosen video, checks if it is loadable and enables other widgets.
     * @param videoIndex - index of the video in the combobox
     */
    void processChosenVideo(int videoIndex);

    /**
     * @brief Draws frangi maximum coordinates.
     * @param i_result - calculated coordinates
     */
    void drawGraphicsScene(QPoint i_result);

    /**
     * @brief Shows or hides IntegratedFrangiOptions object.
     */
    void showFrangiOptions();

    /**
     * @brief Terminates independent thread where frangi maximum was calculated.
     */
    void onDataExtracted();

    /**
     * @brief Changes the icon of the button when IntegratedFrangiOptions object has status "shown".
     */
    void onShown();

    /**
     * @brief Changes the icon of the button when IntegratedFrangiOptions object has status "hidden".
     */
    void onHidden();

private:
    /**
     * @brief Defines painted cross (using QPainterPath) parameters depending on received frangi maximum coordinates
     * @param path
     * @param x - frangi x coordinate
     * @param y - frangi y coordinate
     */
    void paintCross(QPainterPath &path,double x, double y);

    /**
     * @brief Returns detected coordinates of frangi maximum.
     */
    QPointF loadFrangiMaxCoordinates();

    /**
     * @brief Fills the graphic scene object of graphics view object with proper frame defined at the dialog initialization.
     * @param i_initCutouts - if true, all cutouts are initialized automatically. Frangi maximum coordinates are used together with
     * default ratios.
     */
    void fillGraphicScene(bool i_initCutouts);

    /**
     * @brief Adds standard widgets to the optionalContent gridLayout when a video is analysed.
     * @param i_count - one or multiple video
     * @param i_cutout
     * @param i_loadFrangiCoordinates - true if a referential frame was found because the analysis was
     */
    void initStandardVideoWidgets(videoCount i_count,cutoutType i_cutout,bool i_loadFrangiCoordinates);

    /**
     * @brief Adds standard widgets to the optionalContent gridLayout when an image is analysed.
     * @param i_cutout
     */
    void initStandardImageWidgets(cutoutType i_cutout);

    /**
     * @brief Collects QGraphicsView mouse events.
     * @param obj
     * @param event
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief Initializes cutouts.
     * @param i_inputFrame
     */
    void initCutouts(cv::Mat i_inputFrame);

    /**
     * @brief Redraws standard cutout rectangle when mouse is hold and moved.
     * @param i_C
     * @param i_anomalyType
     */
    void updateCutoutStandard();

    /**
     * @brief Redraw extra cutout rectangle when mouse is hold and moved.
     * @param i_clickCoordinates
     * @param i_anomalyType
     */
    void updateCutoutExtra();

    /**
     * @brief The function is called when extra cutout has been changed. Frangi maximum coordinates are used for
     * this operation.
     */
    void recalculateStandardCutout();

    /**
     * @brief Checks, if the frangi maximum coordinates are inside the given area.
     * @param i_area
     */
    bool checkFrangiMaximumPresenceInCutout();

    /**
     * @brief Starts frangi analysis.
     */
    void startFrangiAnalysis();

    /**
     * @brief If cutout estimation fails or the result is not acceptable, the cutout parameters are discared and previous
     * correct parameters are loaded.
     */
    void revertCutoutChange();

    /**
     * @brief Saves selected cutouts.
     * @param saveNew - if true, saves newly selected cutout(s). If false, saves previously automatically calculated cutout(s).
     */
    void saveCutouts(bool saveNew);

    // Graphic part of class
    Ui::ClickImageEvent *ui;
    QPixmap image;
    QImage  *imageObject = nullptr;
    QGraphicsScene *scene = nullptr;

    // Graphic elements and their properties
    double height, width;
    QGraphicsPathItem* pathItem = nullptr;
    QGraphicsPathItem* frangiCoords = nullptr;
    QGraphicsRectItem* standardCutout_GRI = nullptr;
    QGraphicsRectItem* extraCutout_GRI = nullptr;
    QMap<QString,QRect> originalStandardCutout,originalExtraCutout,standardCutout,extraCutout;
    QRect tempStandardCutout,tempExtraCutout;

    // Helping properties
    QString filePath = "",videoName = "";
    cutoutType cutout = EXTRA;
    double frameCount = 0.0;
    QMap<QString,int> referencialFrameNo;
    bool disabled = true, mousePressed = false, modified = false, frangiOptionsShown = false;
    QPointF selectionOrigin,lastDragPosition,selectionEnd;
    QMap<QString,QPoint> frangiCoordinates;
    QPoint lastSelectionPosition;

    QStringList filePaths,videoNames;
    cv::VideoCapture cap;
    std::function<void(ClickImageEvent*)> cutoutFunctionPointer;
    enum updateType{UPDATE_DIRECTLY,RECALCULATE} cutoutUpdate;
    QMap<QWidget*,ErrorDialog*> localErrorDialogHandling;
    IntegratedFrangiOptions* _integratedFrangiOptionsObject = nullptr;
    QHash<int,QThread*> threadPool;

    // Possible grid layout widgets
    QComboBox* videoSelection = nullptr;
    QLineEdit* referentialFrameNumber = nullptr;
    QLabel* chosenVideo = nullptr;
    QLabel* chosenCutout = nullptr;
    QPushButton* revealFrangiOptions = nullptr;
    chosenSource whatIsAnalysed = chosenSource::VIDEO;
    videoCount actualVideoCount = videoCount::NO_VIDEO;
};

#endif // CLICKIMAGEEVENT_H
