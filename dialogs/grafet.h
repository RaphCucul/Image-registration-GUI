#ifndef GRAFET_H
#define GRAFET_H

#include <QDialog>
#include <vector>
#include <QHash>
#include "shared_staff/qcustomplot.h"
#include "util/util_grafet.h"

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>

namespace Ui {
class GrafET;
}

enum class ColorTheme
{
    Light,
    Dark,
    TOS
};

class SW_VerticalQCPItemLine : public QCPItemLine
{
public:
    SW_VerticalQCPItemLine(QCustomPlot *parentPlot, ColorTheme color);
    virtual ~SW_VerticalQCPItemLine();

    void UpdateLabel(double x, double y, QPixmap i_framePixmap);
    void SetVisibility(bool i_status);

    QCPItemText* m_lineLabel;
    QCPItemPixmap* m_linePixmap;
    QLabel* entropyLabel = nullptr;
    QLabel* tennengradLabel = nullptr;
    QLabel* evalIndex = nullptr;
};

class GrafET : public QDialog
{
    Q_OBJECT

public:
    enum ThresholdType{
        UPPER=5, LOWER=6
    };
    enum ValueType{
        ENTROPY=10, ENTROPY_STANDARD=20, TENNENGRAD=30, TENNENGRAD_STANDARD=40, BOTH=50
    };

    enum ExtremeType{
        MAX=7,MIN=8
    };

    enum SaveOption{
        ALL,EXTREMES
    };

    enum EvaluationFrames{
        ENTROPY_FIRST=8,TENNENGRAD_FIRST,DECISION_FIRST,DECISION_SECOND,EVALUATION_COMPLETE,
        EVALUATION_1,EVALUATION_4,EVALUATION_5
    };

    explicit GrafET(QVector<double> i_entropy,
                    QVector<double> i_tennengrad,
                    QVector<double> i_thresholds,
                    QVector<int> i_FirstEvalEntropy,
                    QVector<int> i_FirstEvalTennengrad,
                    QVector<int> i_FirstDecisionResults,
                    QVector<int> i_SecondDecisionResults,
                    QVector<int> i_CompleteEvaluation,
                    QString i_videoName, QString i_suffix,
                    QWidget *parent = nullptr);
    ~GrafET() override;
    /**
     * @brief getVideoName
     * @return
     */
    QString getVideoName() { return videoName; }
    /**
     * @brief getReturnJSONObject
     * @return
     */
    QJsonObject getReturnJSONObject(){ return returnObject; }
signals:
    void saveCalculatedData(QString videoName, QJsonObject data);
    void resizeWindow();
private slots:
    /*void showEntropy();
    void showTennengrad();
    void showEntropyUpperThreshold();
    void showEntropyLowerThreshold();
    void showTennengradUpperThreshold();
    void showTennengradLowerThreshold();
    void firstEvaluationEntropy_f();
    void firstEvaluationTennengrad_f();
    void firstEvaluation_f();
    void secondEvaluation_f();
    void on_ohodnocKomplet_stateChanged(int arg1);
    void on_IV1_stateChanged(int arg1);
    void on_IV4_stateChanged(int arg1);
    void on_IV5_stateChanged(int arg1);*/
    void showVals(QMouseEvent* event);
    void on_showFrameOverview_stateChanged(int arg1);
    void fixSize();
    void on_saveResults_clicked();
    void on_saveThresholds_clicked();
    void showContextMenu(QMouseEvent* i_point);
    void onReactOnDisplayedParameterChange();

private:
    /**
     * @brief Function adds bad frame markers (evalType) into the graph.
     * @param i_graphLine
     * @param i_tabIndex
     * @param i_y_coords
     * @param i_x_coords
     * @param i_evalType
     */
    void populateGraph(int i_graphLine,QVector<int>& i_y_coords,
                       QVector<double>& i_x_coords,int i_evalType);

    /**
     * @brief Function hides all elements if neither entropy nor tennegrad are visible.
     */
    void hideAll();

    /**
     * @brief hideSpecificGraph
     */
    void hideSpecificGraph(int i_identificatorValueType, int i_identificator, double newThresholdValue);

    /**
     * @brief Function return the maximum or the minimum of the input vector.
     * @param i_analysedVector
     * @param typExtremu
     * @return
     */
    void findExtremesAndThresholds(QVector<double> i_calculatedThresholds);

    /**
     * @brief findExtreme
     * @param i_vectorForSearch
     * @param extreme
     * @return
     */
    double findExtreme(QVector<double> i_vectorForSearch, ExtremeType extreme);

    /**
     * @brief Function creates the threshold of entropy or tennengrad for the graphical purpose.
     * @param i_extrem
     * @param threshold
     * @param type
     * @return
     */
    double createThreshold(double i_extrem, ThresholdType threshold, ValueType type);

    /**
     * @brief Function calculates standardized value of the input value.
     * @param i_inputVector
     * @param i_standardizedVector
     * @param i_max
     * @param i_min
     */
    void valueStandardization(ValueType type, double i_max, double i_min);

    /**
     * @brief Function calculates standardized value of the input value.
     * @param i_originalValue
     * @param i_max
     * @param i_min
     * @return
     */
    double valueStandardization(double i_originalValue, double i_max, double i_min);

    /**
     * @brief Function creates the vector for the graph.
     * @param i_originalValueForVector
     * @param i_frameCount
     * @return
     */
    QVector<double> thresholdLine(double i_originalValueForVector, int i_frameCount);

    /**
     * @brief findReferentialFrame
     * @return
     */
    int findReferentialFrame();

    /**
     * @brief reactionOnFrameVisibility
     */
    //void reactionOnFrameVisibility();

    /**
     * @brief getAxesValues
     * @param i_mouseX_coordinates
     * @return
     */
    double getAxesValues(double i_mouseX_coordinates);

    /**
     * @brief showAffectedFrames
     * @param i_affectedValues
     * @param i_valueToCompareWith
     * @param i_criterium
     * @param i_graphLine
     */
    void showAffectedFrames(QVector<double> i_affectedValues, double i_valueToCompareWith,
                            QString i_criterium, int i_graphLine);

    /**
     * @brief makeReferential
     * @param i_frameIndex
     */
    void makeReferential(int i_frameIndex);

    /**
     * @brief changeEvalIndex
     * @param i_frameIndex
     */
    void changeEvalIndex(int i_frameIndex);

    /**
     * @brief closeEvent
     * @param event
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief saveData
     * @param i_saveOption
     */
    void saveData(SaveOption i_saveOption);

    /**
     * @brief test
     */
    template <typename T>
    void prepareDataForSaving(QVector<T> i_vector,QJsonObject& i_objectToFill, QString i_parameterName) {
        QJsonArray pomArray = vector2array(i_vector);
        i_objectToFill[i_parameterName] = pomArray;
    }

    /**
     * @brief thresholdsToVector
     * @return
     */
    QVector<double> thresholdsToVector();

    /**
     * @brief prepareDataForUtilGraphET
     * @return
     */
    QMap<QString,QVector<double>> prepareDataForUtilGraphET();

    /**
     * @brief reactOnDisplayedParameterChange
     * @param i_valueType
     */
    void reactOnDisplayedParameterChange(ValueType i_valueType, bool standardizationRequested);

    /**
     * @brief onReactOnThresholdParameterChange
     * @param thresholdType
     * @param thresholdValue
     * @param isChecked
     */
    void onReactOnThresholdParameterChange(ValueType i_type,
                                           ThresholdType i_thresholdType,
                                           bool isCheckbox);

    /**
     * @brief reactOnThresholdParameterChange
     * @param i_valueType
     * @param standardizationRequested
     */
    void reactOnThresholdParameterChange(ValueType i_affectedType,
                                         ThresholdType i_actuallyDisplayed, double i_thresholdValue,
                                         bool i_showAffectedFrames, bool i_standardizationRequested);

    /**
     * @brief adjustThresholdVectors
     * @param i_graphIdentifier
     * @param i_mapIdentifier
     * @param i_thresholdIdentifier
     */
    void adjustThresholdVectors(int i_mapIdentifier, int i_thresholdIdentifier,
                                bool i_setVisible=false);

    /**
     * @brief initDataMaps
     */
    void initDataMaps(QVector<double> i_entropy,QVector<double> i_tennengrad, QVector<int> i_firstEvalEntropy,
                      QVector<int> i_firstEvalTennengrad, QVector<int> i_firstDecisionResults,
                      QVector<int> i_secondDecisionResults, QVector<int> i_completeEvaluation);

    /**
     * @brief initAffectedFramesConnections
     */
    void initAffectedFramesConnections();

    /**
     * @brief entropyRelatedWidgetsController
     * @param status
     */
    void entropyRelatedWidgetsController(bool status);

    /**
     * @brief tennengradRelatedWidgetsController
     * @param status
     */
    void tennengradRelatedWidgetsController(bool status);

    /**
     * @brief showCorrespondingFrames
     * @param i_valueType
     * @param i_evaluationType
     * @param showFramesInGraph
     * @param ETdependency
     * @param evaluationType
     */
    void showCorrespondingFrames(int i_valueType, int i_evaluationType,
                                 bool showFramesInGraph, bool ETdependency, int i_evaluatioTypeIndex);

    Ui::GrafET *ui;
    QCustomPlot* ActualGraphicalObject = nullptr;
    util_GraphET* utilGraph = nullptr;
    SW_VerticalQCPItemLine *cursor = nullptr;
    QMap<QString,QVector<double>> ETparametersDoubleMap;
    QMap<QString,QVector<int>> ETparametersIntMap;
    QMap<int,QString> ETconnections;
    QMap<int,QString> EFconnections;
    QMap<int,int> affectedFramesConnections;
    QHash<int,QDoubleSpinBox*> DSBreferences;
    QHash<int,QCheckBox*> CBreferences;
    QHash<QDoubleSpinBox*,QCheckBox*> interWidgetsReferences;

    int frameCount = 0, frameRows = 0, frameCols = 0;
    QMap<int,double> thresholds;
    QMap<int,double> extremes;
    QVector<double> valueRange;
    bool originFound = false,showFrames = false, dataChanged = false, extremesChanged = false;
    cv::VideoCapture cap;
    QGraphicsView* framePreview = nullptr;
    QGraphicsScene* scene = nullptr;
    ValueType _displayedGraphs = ValueType::ENTROPY;
    QMap<QString,QVector<double>> affectedFrames;
    QString videoName = "";
    QJsonObject returnObject;
};
#endif // GRAFET_H
