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


class IntegratedFrangiOptions : public QWidget
{
    Q_OBJECT

public:
    IntegratedFrangiOptions();
    ~IntegratedFrangiOptions();
    void GetSetValues(QString method, QMap<QString,int>& m, QMap<QString,double>& r, QMap<QString,double>& p);
    QMap<QString,int> getIntegratedMargins() {return marginsInternal;}
    QMap<QString,double> getInternalParameters() {return parametersInternal;}
    QMap<QString,double> getInternalRatios() {return ratiosInternal;}
public slots:
    void saveButtonClicked();
    void setButtonClicked();
signals:
    void frangiParametersSelected(QMap<QString,int> _margins,QMap<QString,double> ratios,QMap<QString,double> _parameters);
    void applyFrangiParameters();
private:
    void createWidget();
    void addWidgetToGrid(QWidget* widget,int row,int col);

    QStringList FrangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    QStringList FrangiMarginsList = {"top_m","bottom_m","left_m","right_m"};
    QStringList FrangiRatiosList = {"top_r","bottom_r","left_r","right_r"};
    QStringList labelsTexts = {tr("top margin"),tr("bottom margin"),tr("left margin"),tr("right margin"),
                               tr("top ration"),tr("bottom ratio"),tr("left ration"),tr("right ration"),
                               tr("sigma start"),tr("sigma stop"),tr("sigma step"),tr("beta one"),tr("beta two"),tr("zpracovani")};

    /*QSpinBox* leftMargin = nullptr;
    QSpinBox* rightMargin = nullptr;
    QSpinBox* topMargin = nullptr;
    QSpinBox* bottomMargin = nullptr;
    QDoubleSpinBox* sigmaStart = nullptr;
    QDoubleSpinBox* sigmaEnd = nullptr;
    QDoubleSpinBox* sigmaStep = nullptr;
    QDoubleSpinBox* betaOne = nullptr;
    QDoubleSpinBox* betaTwo = nullptr;*/
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
 * @brief The ClickImageEvent class
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
    void saveSelectedParameters(QMap<QString,int> _margins,QMap<QString,double> _ratios, QMap<QString,double> _parameters);
    void obtainFrangiCoordinates(QPoint i_calculatedData);
    void onShowFrangiOptions();
    void onApplyFrangiParameters();
protected:
    void mousePressEvent(QMouseEvent *press) override;
    void mouseMoveEvent(QMouseEvent *move) override;
    void mouseReleaseEvent(QMouseEvent *release) override;
    void closeEvent(QCloseEvent *e) override;
signals:
    //void saveImageCutout(QRect cutout);
    void saveVideoCutout();
    void dataExtracted();
private slots:
    /**
     * @brief Function process the input of referencial frame line edit and enables the QGraphicsView if
     * the frame number is correct and frame can be loaded.
     * @param arg1
     */
    void referencialFrameChosen();

    /**
     * @brief Function process the chosen video, checks if it is loadable and enables other widgets.
     * @param videoIndex
     */
    void processChosenVideo(int videoIndex);

    /**
     * @brief drawGraphicsScene
     * @param i_result
     */
    void drawGraphicsScene(QPoint i_result);

    /**
     * @brief showFrangiOptions
     */
    void showFrangiOptions();

    /**
     * @brief onDataExtracted
     */
    void onDataExtracted();

    /**
     * @brief onShown
     */
    void onShown();

    /**
     * @brief onHidden
     */
    void onHidden();

private:
    /**
     * @brief Function paint the cross using QPainterPath
     * @param path
     * @param x
     * @param y
     */
    void paintCross(QPainterPath &path,double x, double y);

    /**
     * @brief Function loads detected coordinates of frangi filter maximum.
     * @return
     */
    QPointF loadFrangiMaxCoordinates();

    /**
     * @brief Function fills the graphic scene of graphics view with the proper frame at the dialog start up.
     */
    void fillGraphicScene(bool i_initCutouts);

    /**
     * @brief Function adds standard widgets to the optionalContent gridLayout.
     */
    void initStandardVideoWidgets(videoCount i_count,cutoutType i_cutout,bool i_loadFrangiCoordinates);

    void initStandardImageWidgets(cutoutType i_cutout);

    /**
     * @brief Function is used for collecting QGraphicsView mouse events.
     * @param obj
     * @param event
     * @return
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief initCutouts
     * @param i_inputFrame
     */
    void initCutouts(cv::Mat i_inputFrame);

    /**
     * @brief Function redraw standard cutout rectangle when mouse is hold and moved.
     * @param i_C
     * @param i_anomalyType
     */
    void updateCutoutStandard();

    /**
     * @brief Function redraw extra cutout rectangle when mouse is hold and moved.
     * @param i_clickCoordinates
     * @param i_anomalyType
     */
    void updateCutoutExtra();

    /**
     * @brief Function is called when extra cutout has been changed. Frangi point coordinates are used for
     * this operation, so it is neccesary to include frangi point into the extra cutout.
     */
    void recalculateStandardCutout();

    /**
     * @brief Function is check, if the frangi point is inside the given area.
     * @param i_area
     */
    bool checkFrangiMaximumPresenceInCutout();

    /**
     * @brief startFrangiAnalysis
     */
    void startFrangiAnalysis();

    /**
     * @brief revertCutoutChange
     */
    void revertCutoutChange();

    /**
     * @brief saveCutouts
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
    bool disabled = false, mousePressed = false, modified = false, frangiOptionsShown = false;
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
