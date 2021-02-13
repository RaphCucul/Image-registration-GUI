#ifndef GRAFET_H
#define GRAFET_H

#include <QDialog>
#include <vector>
#include <QHash>
#include "shared_staff/qcustomplot.h"
#include "util/util_grafet.h"

#include <opencv2/opencv.hpp>

namespace Ui {
class GrafET;
}

/**
 * @enum ColorTheme
 * @brief The ColorTheme enum helps to use ColorTheme of the QCPItemLine
 */
enum class ColorTheme
{
    Light,
    Dark,
    TOS
};

/**
 * @class SW_VerticalQCPItemLine
 * @brief The SW_VerticalQCPItemLine class is derived from the QCPItemLine and it is used to display vertical line under mouse
 * cursor. This helps a user to work with the graph.
 */
class SW_VerticalQCPItemLine : public QCPItemLine
{
public:
    SW_VerticalQCPItemLine(QCustomPlot *parentPlot, ColorTheme color);
    virtual ~SW_VerticalQCPItemLine();

    /**
     * @brief Unused function - could be used to show label of the line.
     * @param x
     * @param y
     * @param i_framePixmap
     */
    void UpdateLabel(double x, double y, int frameIndex);

    /**
     * @brief Sets the visibility of the cursor.
     * @param i_status
     */
    void SetVisibility(bool i_status);

    QCPItemText* m_lineLabel;
    QCPItemPixmap* m_linePixmap;
    QLabel* entropyLabel = nullptr;
    QLabel* tennengradLabel = nullptr;
    QLabel* evalIndex = nullptr;
};

/**
 * @class GrafET
 * @brief The GrafET class visualizes calculated entropy and tennengrad data. It is also possible to highlight frames
 * according to their evaluation indexes.
 */
class GrafET : public QDialog
{
    Q_OBJECT

public:
    /**
     * @enum ThresholdType
     * @brief The ThresholdType enum represents type of a threshold.
     */
    enum ThresholdType{
        UPPER=5, LOWER=6
    };
    /**
     * @enum ValueType
     * @brief The ValueType enum represents plotted graphs.
     */
    enum ValueType{
        ENTROPY=10, ENTROPY_STANDARD=20, TENNENGRAD=30, TENNENGRAD_STANDARD=40, BOTH=50
    };
    /**
     * @enum ExtremeType
     * @brief The ExtremeType enum represents maximum and minimum of values.
     */
    enum ExtremeType{
        MAX=7,MIN=8
    };
    /**
     * @enum SaveOption
     * @brief The SaveOption enum represents two possible save options - all values or just extremes can be saved.
     */
    enum SaveOption{
        ALL,EXTREMES
    };
    /**
     * @enum EvaluationFrames
     * @brief The EvaluationFrames enum represents all evaluation indexes and frame categories.
     */
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
     * @brief Returns videoname of GrafET object
     */
    QString getVideoName() { return videoName; }
    /**
     * @brief Returns JSON object with data prepared to be saved.
     * @return
     */
    QJsonObject getReturnJSONObject(){ return returnObject; }
signals:
    void saveCalculatedData(QString videoName, QJsonObject data);
    /**
     * @brief Informs the program to resize the dialogue.
     *
     * The signal is connected to the choice to show/hide actually selected frame.
     */
    void resizeWindow();
private slots:
    /**
     * @brief Shows values of entropy/tennengrad/both (depends what graph is activated) of the frame under the cursor.
     * @param event
     */
    void showVals(QMouseEvent* event);
    /**
     * @brief Shows/hides an overview of the frame under the cursor.
     * @param arg1 - unused checkbox status
     */
    void on_showFrameOverview_stateChanged(int arg1);
    /**
     * @brief Calls save function to save complete loaded information. If a user changes an evaluation index
     * of a frame, this function helps to save it.
     * @sa saveData(SaveOption i_saveOption)
     */
    void on_saveResults_clicked();
    /**
     * @brief Calls save function saving E+T thresholds only.
     * @sa saveData(SaveOption i_saveOption)
     */
    void on_saveThresholds_clicked();
    /**
     * @brief Invokes context menu - it is possible to change evaluation index of a frame or it is possible to change
     * referential frame.
     * @param i_point - mouse click coordinates - helps to determine a frame
     */
    void showContextMenu(QMouseEvent* i_point);
    /**
     * @brief Changes a graph visibility. Shows/hides graphs of entropy, tennengrad and corresponding thresholds.
     * @sa reactOnDisplayedParameterChange(ValueType i_valueType, bool standardizationRequested)
     */
    void onReactOnDisplayedParameterChange();

private:
    /**
     * @brief Adds bad frame markers (evalType) into the graph.
     * @param i_graphLine - numeric representation of a graph line
     * @param i_y_coords
     * @param i_x_coords
     * @param i_evalType
     */
    void populateGraph(int i_graphLine,QVector<int>& i_y_coords,
                       QVector<double>& i_x_coords,int i_evalType);

    /**
     * @brief Hides all elements if neither entropy nor tennegrad are visible.
     */
    void hideAll();

    /**
     * @brief Hides a specific graph of a thresholds and sets new value of the threshold.
     * @param i_identificatorValueType - entropy/tennengrad
     * @param i_identificator - upper/lower
     * @param newThresholdValue - new value
     */
    void hideSpecificGraph(int i_identificatorValueType, int i_identificator, double newThresholdValue);

    /**
     * @brief Returns the maximum or minimum of the input vector.
     * @param i_calculatedThresholds
     */
    void findExtremesAndThresholds(QVector<double> i_calculatedThresholds);

    /**
     * @brief Returns the value of an extreme of the input vector.
     * @param i_vectorForSearch
     * @param extreme
     */
    double findExtreme(QVector<double> i_vectorForSearch, ExtremeType extreme);

    /**
     * @brief Creates the threshold of entropy or tennengrad for the graphical purpose.
     * @param i_extrem
     * @param threshold
     * @param type
     * @return Value of the threshold.
     */
    double createThreshold(double i_extrem, ThresholdType threshold, ValueType type);

    /**
     * @brief Calculates standardized value of the input value.
     * @param type - type of the graph
     * @param i_max
     * @param i_min
     */
    void valueStandardization(ValueType type, double i_max, double i_min);

    /**
     * @brief Calculates standardized value of the input value.
     * @param i_originalValue
     * @param i_max
     * @param i_min
     * @return Standardized value.
     */
    double valueStandardization(double i_originalValue, double i_max, double i_min);

    /**
     * @brief Creates a vector for the graph. The vector length corresponds with the number of frames of a video
     * and all vector elements have the same value.
     * @param i_originalValueForVector
     * @param i_frameCount
     * @return Created vector.
     */
    QVector<double> thresholdLine(double i_originalValueForVector, int i_frameCount);

    /**
     * @brief Looks for the referential frame of the video.
     * @return Referential frame index.
     */
    int findReferentialFrame();

    /**
     * @brief Identifies the frame under the mouse cursor.
     * @param i_mouseX_coordinates
     * @return Index of the chosen frame - it can be used to find entropy and tennengrad value of this frame.
     */
    double getAxesValues(double i_mouseX_coordinates);

    /**
     * @brief Shows frame with entropy value greater or lesser than the given criterion.
     * @param i_affectedValues - entropy/tennengrad dvalues of frames
     * @param i_valueToCompareWith - criterion
     * @param i_criterion - define if greater or lesser values should be highlighted
     * @param i_graphLine - index representation of the graph
     */
    void showAffectedFrames(QVector<double> i_affectedValues, double i_valueToCompareWith,
                            QString i_criterion, int i_graphLine);

    /**
     * @brief Marks the given frame as referential.
     * @param i_frameIndex
     */
    void makeReferential(int i_frameIndex);

    /**
     * @brief Changes the evaluation index of the given frame. The new EI value is defined by a user in a
     * invoked dialog.
     * @param i_frameIndex
     */
    void changeEvalIndex(int i_frameIndex);

    /**
     * @brief Overridden close events checks if any value was changed - if so, saving dialog appears to give a user a choice
     * to save data.
     * @param event
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Saves available data. It can save all data or just thresholds values.
     * @param i_saveOption
     */
    void saveData(SaveOption i_saveOption);

    /**
     * @brief Template function prepares data for savings. Vector with values is converted to JSON array. The array is then
     * appended to final JSON object.
     * @tparam i_vector - input vector of type T
     * @tparam i_objectToFill - final JSON object with all values
     * @tparam i_parameterName - key in JSON object which will represent added array
     */
    template <typename T>
    void prepareDataForSaving(QVector<T> i_vector,QJsonObject& i_objectToFill, QString i_parameterName) {
        QJsonArray pomArray = vector2array(i_vector);
        i_objectToFill[i_parameterName] = pomArray;
    }

    /**
     * @brief Converts a threshold value into a vector.
     * @return
     */
    QVector<double> thresholdsToVector();

    /**
     * @brief Updates the appearance of all shown threshold graphs and highlights frames out of threshold range.
     * @param i_valueType
     * @param standardizationRequested
     */
    void reactOnDisplayedParameterChange(ValueType i_valueType, bool standardizationRequested);

    /**
     * @brief Takes a threshold value and modifies a threshold graph if visible. The function is called when a threshold value
     * is changed or when a threshold graph visibility is changed.
     * @param i_type - entropy/tennengrad
     * @param i_thresholdType - upper/lower
     * @param isCheckbox - visibility checkbox or double spin box
     */
    void onReactOnThresholdParameterChange(ValueType i_type,
                                           ThresholdType i_thresholdType,
                                           bool isCheckbox);

    /**
     * @brief Updates a threshold graph.
     * @param i_affectedType - entropy/tennengrad
     * @param i_actuallyDisplayed - upper/lower
     * @param i_thresholdValue - actual value
     * @param i_showAffectedFrames - highlight frames out of the threshold range
     * @param i_standardizationRequested - if it is necessary to standardize threshold vectors.
     */
    void reactOnThresholdParameterChange(ValueType i_affectedType,
                                         ThresholdType i_actuallyDisplayed, double i_thresholdValue,
                                         bool i_showAffectedFrames, bool i_standardizationRequested);

    /**
     * @brief When a threshold value is changed, it is necessary to change the graph and save the value.
     * @param i_mapIdentifier - numeric representation of type and threshold
     * @param i_thresholdIdentifier - numeric representation of type and threshold which considers possible standardization
     * @param i_setVisible
     */
    void adjustThresholdVectors(int i_mapIdentifier, int i_thresholdIdentifier,
                                bool i_setVisible=false);

    /**
     * @brief Converts given vectors into a QMap objects. Internal connections between data and widgets are created for
     * much faster and cleaner processing of commands.
     * @param i_entropy
     * @param i_tennengrad
     * @param i_firstEvalEntropy
     * @param i_firstEvalTennengrad
     * @param i_firstDecisionResults
     * @param i_secondDecisionResults
     * @param i_completeEvaluation
     */
    void initDataMaps(QVector<double> i_entropy,QVector<double> i_tennengrad, QVector<int> i_firstEvalEntropy,
                      QVector<int> i_firstEvalTennengrad, QVector<int> i_firstDecisionResults,
                      QVector<int> i_secondDecisionResults, QVector<int> i_completeEvaluation);

    /**
     * @brief Creates inner connections between thresholds and their graphs.
     */
    void initAffectedFramesConnections();

    /**
     * @brief Enables/disables entropy-related widgets.
     * @param status
     */
    void entropyRelatedWidgetsController(bool status);

    /**
     * @brief Enables/disables tennengrad-related widgets.
     * @param status
     */
    void tennengradRelatedWidgetsController(bool status);

    /**
     * @brief Shows/hides a graph. This function reacts on user interaction with the dialog.
     * @param i_valueType - entropy/tennengrad
     * @param i_evaluationType - evaluation index of affected frames
     * @param showFramesInGraph - the graph is meant to be shown (true) or hidden (false)
     * @param ETdependency - if graph is dependent on entropy or tennengrad graph
     * @param i_evaluatioTypeIndex - numeric representation of evaluation index
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
