#ifndef SHAREDVARIABLES_H
#define SHAREDVARIABLES_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QRect>
#include <QLabel>
#include <QMovie>
#include <QGridLayout>
#include <QPoint>
#include <opencv2/opencv.hpp>

/**
 * @brief Namespace clickImageEnums is used in ClickImageEvent class to determine
 * how many videos will be processed and what type of cutout is being edited.
 */
namespace clickImageEnums {
    enum videoCount {ONE_VIDEO,MULTIPLE_VIDEOS,NO_VIDEO};
    enum cutoutType {STANDARD=1,EXTRA,NO_CUTOUT};
    enum chosenSource {VIDEO,IMAGE,NOTHING};
}
/**
 *
 */
namespace frangiEnums {
    enum frangiType {GLOBAL,VIDEO_SPECIFIC};
}

using namespace frangiEnums;

/**
 * @brief The FrangiThread class
 */
class FrangiThread : public QObject
{
    Q_OBJECT

public:
    FrangiThread(const cv::Mat i_inputFrame, int i_processingMode, int i_accuracy,
                 int i_showResult, QString i_windowName, int i_frameType,
                 cv::Point3d i_translation, QVector<double> i_FrangiParameters,
                 QMap<QString, int> i_margins);
    FrangiThread(const cv::Mat i_inputFrame, int i_processingMode, int i_accuracy,
                 int i_showResult, QString i_windowName, int i_frameType,
                 cv::Point3d i_translation, QMap<QString,double> i_FrangiParameters,
                 QMap<QString, int> i_margins);
    virtual ~FrangiThread(){ }
public slots:
    void startFrangiAnalysis();    
signals:
    void errorOccured(int errorIndex);
    void finishedCalculation(QPoint i_calculatedData);
private:
    cv::Point3d _translation;
    cv::Mat _inputFrameForAnalysis;
    int _processingMode=0,_accuracy=0,_showResult=0,_frameType=0;
    QString _windowName = "";
    QMap<QString, double> _FrangiParameters;
    QMap<QString, int> _margins;
};

class ProcessingIndicator : public QWidget
{
    Q_OBJECT
public:
    ProcessingIndicator(QString i_description, QWidget* i_widgetWithIndicator, QWidget* parent = nullptr);
    ~ProcessingIndicator();
    void placeIndicator();
    bool wasPlaced() { return alreadyPlaced; }
    void hideIndicator();
    void showIndicator();
public slots:
    void hideIndicatorSlot();
private:
    /**
     * @brief showMessage
     */
    void showMessage();

    QLabel* gifLabel=nullptr;
    QMovie* gifContent=nullptr;
    QWidget* widgetWithIndicator=nullptr;
    QWidget* errorWidgetParent=nullptr;
    QString indicatorText = "";
    bool alreadyPlaced = false;
};

class SharedVariables
{
public:
    Q_DISABLE_COPY(SharedVariables)

    static SharedVariables *getSharedVariables();

    /**
     * @brief Get actual path for specific path type.
     * @param type
     * @return
     */
    QString getPath(QString type) const;

    /**
     * @brief Set actual path for specific path type.
     * @param type
     * @param path
     */
    void setPath(QString type, QString path);

    /**
     * @brief Function loads data from *.json with frangi parameters. Predefined name of *.json file
     * is expected!
     * @param path
     * @return
     */
    bool processFrangiParameters(QString path);

    /**
     * @brief processVideoFrangiParameters
     * @param videoName
     * @param obtainedParameters
     * @return
     */
    bool processVideoFrangiParameters(QString videoName, QMap<QString,QVariant>& obtainedParameters);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief getFrangiParameterWrapper
     * @param i_type
     * @param i_videoName
     * @return
     */
    QMap<QString,double> getFrangiParameterWrapper(frangiType i_type, QString i_videoName) const;
    /**
     * @brief getSpecificFrangiParameterWrapper
     * @param i_type
     * @param i_videoName
     * @param parameter
     * @return
     */
    double getSpecificFrangiParameterWrapper(frangiType i_type, QString i_videoName, QString parameter);
    /**
     * @brief setFrangiParameterWrapper
     * @param i_type
     * @param i_videoName
     * @param i_parameters
     */
    void setFrangiParameterWrapper(frangiType i_type,QString i_videoName,QMap<QString,double> i_parameters);
    /**
     * @brief setSpecificFrangiParameterWrapper
     * @param i_type
     * @param i_value
     * @param i_videoName
     * @param i_parameter
     * @param i_param
     */
    void setSpecificFrangiParameterWrapper(frangiType i_type, double i_value, QString i_videoName,
                                           QString i_parameter, int i_param=-1);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief saveFrangiParametersWrapper
     * @param i_type
     * @param i_videoName
     */
    void saveFrangiParametersWrapper(frangiType i_type,QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief getFrangiMaximumWrapper
     * @param i_type
     * @param i_videoName
     * @return
     */
    cv::Point3d getFrangiMaximumWrapper(frangiType i_type,QString i_videoName);
    /**
     * @brief setFrangiMaximumWrapper
     * @param i_type
     * @param i_videoName
     */
    void setFrangiMaximumWrapper(frangiType i_type, QString i_videoName,
                                 cv::Point3d i_coordinates);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief setFrangiMarginsWrapper
     * @param i_type
     * @param i_margins
     * @param i_videoName
     */
    void setFrangiMarginsWrapper(frangiType i_type, QMap<QString,int> i_margins, QString i_videoName);
    /**
     * @brief setSpecificFrangiMarginsWrapper
     * @param i_type
     * @param i_videoName
     * @param i_value
     */
    void setSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName, int i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief getFrangiMarginsWrapper
     * @return
     */
    QMap<QString,int> getFrangiMarginsWrapper(frangiType i_type, QString i_videoName);
    /**
     * @brief getSpecificFrangiMarginWrapper
     * @param i_type
     * @param i_marginName
     * @param i_videoName
     * @return
     */
    int getSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief setFrangiRatiosWrapper
     * @param i_type
     * @param i_ratios
     * @param i_videoName
     */
    void setFrangiRatiosWrapper(frangiType i_type, QMap<QString,double> i_ratios, QString i_videoName);
    /**
     * @brief setSpecificFrangiRatioWrapper
     * @param i_type
     * @param i_ratioName
     * @param i_videoName
     * @param i_value
     */
    void setSpecificFrangiRatioWrapper(frangiType i_type,QString i_ratioName,QString i_videoName,double i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief getFrangiRatiosWrapper
     * @param i_type
     * @param i_videoName
     * @return
     */
    QMap<QString,double> getFrangiRatiosWrapper(frangiType i_type,QString i_videoName);
    /**
     * @brief getSpecificFrangiRatioWrapper
     * @param i_type
     * @param i_ratioName
     * @param i_videoName
     * @return
     */
    double getSpecificFrangiRatioWrapper(frangiType i_type,QString i_ratioName,QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function returns cv::Rect object with information about standard cutout
     * @return
     */
    cv::Rect getStandardCutout();

    /**
     * @brief Function saves QRect information about standard cutout.
     * @param i_newStandardCutout
     */
    void setStandardCutout(QRect i_newStandardCutout);

    /**
     * @brief Function returns cv::Rect object with information about extra cutout
     * @return
     */
    cv::Rect getExtraCutout();

    /**
     * @brief Function saves QRect information about extra cutout.
     * @param i_newExtraCutout
     */
    void setExtraCutout(QRect i_newExtraCutout);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief setVideoInformation
     * @param i_video
     * @param i_key
     * @param i_dataToSave
     */
    void setVideoInformation(QString i_video, QString i_key, QVariant i_dataToSave);

    /**
     * @brief getVideoInformation
     * @param i_video
     * @param i_key
     * @return
     */
    QVariant getVideoInformation(QString i_video, QString i_key);

    /**
     * @brief getCompleteVideoInformation
     * @param i_video
     * @return
     */
    QMap<QString,QVariant> getCompleteVideoInformation(QString i_video);

    /**
     * @brief checkVideoInformationPresence
     * @param i_video
     * @return
     */
    bool checkVideoInformationPresence(QString i_video);

private:
    SharedVariables();

    /**
     * @brief Function fills vector of Frangi parameters with values from JSON object
     * @param loadedObject
     * @param parameter
     * @param loadedParameters
     * @param position
     */
    bool inicialization_frangi_opt(QJsonObject loadedObject, QString parameter, QVector<double>& loadedParameters,
                                 int &position);
    /**
     * @brief Vector of Frangi parameters is initialized to have specific size
     * @param size
     * @param loadedParameters
     */
    void size_frangi_opt(int size, QVector<double>& loadedParameters);

    /**
     * @brief Get specific parameter from the Frangi parameters vector
     * @param position
     * @param loadedParameters
     * @return
     */
    double data_from_frangi_opt(int position, QVector<double>& loadedParameters);

    /**
     * @brief Function populate cv::Rect rectangle with data from QRect
     */
    void populateOutputRectangle(QRect& i_qRect,cv::Rect& i_cvRect);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function returns the vector of frangi parameters in predefined order for better processing.
     * @return
     */
    //QVector<double> getFrangiParameters() const;
    /**
     * @brief getFrangiParamters
     * @return
     */
    QMap<QString,double> getFrangiParameters() const;
    /**
     * @brief getFrangiParameters
     * @param i_videoName
     * @return
     */
    QMap<QString,double> getVideoFrangiParameters(QString i_videoName) const;
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function returns the value of specific parameter.
     * @param parameter
     * @return
     */
    double getSpecificFrangiParameter(QString parameter);
    /**
     * @brief Function returns the value of specific parameter.
     * @param parameter
     * @return
     */
    double getSpecificFrangiParameter(int parameter);
    /**
     * @brief getSpecificVideoFrangiParameter
     * @param i_videoName
     * @param parameter
     * @return
     */
    double getSpecificVideoFrangiParameter(QString i_videoName, QString parameter);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief setFrangiParameters
     * @param i_parameters
     */
    void setFrangiParameters(QMap<QString,double> i_parameters);
    /**
     * @brief setVideoFrangiParameters
     * @param i_videoName
     * @param i_parameters
     */
    void setVideoFrangiParameters(QString i_videoName,QMap<QString,double> i_parameters);
    /**
     * @brief Function set the value of specific parameter.
     * @param parameter
     * @param value
     */
    void setSpecificFrangiParameter(int parameter,double value);
    /**
     * @brief setSpecificVideoFrangiParameter
     * @param i_videoName
     * @param i_parameter
     * @param i_value
     */
    void setSpecificVideoFrangiParameter(QString i_videoName, QString i_parameter,double i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function saves actual frangi parameters to the file in the chosen actual directory.
     */
    void saveFrangiParameters();
    /**
     * @brief saveVideoFrangiParameters
     * @param i_videoName
     */
    void saveVideoFrangiParameters(QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function saves global settings of margins for frangi analysis.
     * @param i_margins
     */
    void setFrangiMargins(QMap<QString,int> i_margins);
    /**
     * @brief setVideoFrangiMargins
     * @param i_videoName
     * @param i_margins
     */
    void setVideoFrangiMargins(QString i_videoName, QMap<QString,int> i_margins);
    /**
     * @brief setSpecificFrangiMargin
     * @param i_marginName
     * @param value
     */
    void setSpecificFrangiMargin(QString i_marginName, int i_value);
    /**
     * @brief setSpecificVideoFrangiMargin
     * @param i_videoName
     * @param i_marginName
     * @param i_value
     */
    void setSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName, int i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief If Franfi filtr was applied on the frame,
     * the coordinates of the maximum can be read by this function.
     * @return
     */
    cv::Point3d getFrangiMaximum();
    /**
     * @brief getVideoFrangiMaximum
     * @param i_videoName
     * @return
     */
    cv::Point3d getVideoFrangiMaximum(QString i_videoName);
    /**
     * @brief Store the latest coordinates of Frangi filtr maximum.
     * @param coordinates
     */
    void setFrangiMaximum(cv::Point3d i_coordinates);
    /**
     * @brief setVideoFrangiMaximum
     * @param i_videoName
     * @param coordinates
     */
    void setVideoFrangiMaximum(QString i_videoName, cv::Point3d i_coordinates);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function returns saved infomation about frangi analysis margins.
     * @return
     */
    QMap<QString,int> getFrangiMargins();
    /**
     * @brief getVideoFrangiMargins
     * @param i_videoName
     * @return
     */
    QMap<QString,int> getVideoFrangiMargins(QString i_videoName);
    /**
     * @brief getSpecificFrangiMargin
     * @param i_marginName
     * @return
     */
    int getSpecificFrangiMargin(QString i_marginName);
    /**
     * @brief getSpecificVideoFrangiMargin
     * @param i_videoName
     * @param i_marginName
     * @return
     */
    int getSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function saves global settings of ratios for frangi analysis.
     * @param i_ratios
     */
    void setFrangiRatios(QMap<QString,double> i_ratios);
    /**
     * @brief setVideoFrangiRatios
     * @param i_videoName
     * @param i_ratios
     */
    void setVideoFrangiRatios(QString i_videoName, QMap<QString,double> i_ratios);
    /**
     * @brief setSpecificFrangiRatio
     * @param i_ratioName
     * @param i_value
     */
    void setSpecificFrangiRatio(QString i_ratioName,double i_value);
    /**
     * @brief setSpecificVideoFrangiRatio
     * @param i_videoName
     * @param i_ratioName
     * @param i_value
     */
    void setSpecificVideoFrangiRatio(QString i_videoName,QString i_ratioName,double i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Function returns saved infomation about frangi analysis ratios.
     * @return
     */
    QMap<QString,double> getFrangiRatios();

    /**
     * @brief getVideoFrangiRatios
     * @param i_videoName
     * @return
     */
    QMap<QString,double> getVideoFrangiRatios(QString i_videoName);
    /**
     * @brief getSpecificFrangiRatio
     * @param i_ratioName
     * @return
     */
    double getSpecificFrangiRatio(QString i_ratioName);

    /**
     * @brief getSpecificVideoFrangiRatio
     * @param i_videoName
     * @param i_ratioName
     * @return
     */
    double getSpecificVideoFrangiRatio(QString i_videoName, QString i_ratioName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    static SharedVariables *g_sharedVariables;
    QMap<QString,QString> chosenActualPathes;
    QVector<double> FrangiParameters;
    QMap<QString,double> FrangiParametersMap;
    QStringList FrangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    QStringList FrangiMarginsList = {"top_m","bottom_m","left_m","right_m"};
    QStringList FrangiRatiosList = {"top_r","bottom_r","left_r","right_r"};
    cv::Point3d detectedFrangiMaximum;

    QMap<QString,double> frangiRatios;
    QMap<QString,int> frangiMargins;
    QRect standardCutoutRectangle, extraCutoutRectangle;
    QHash<QString,QMap<QString,QVariant>> videoInformation;
    QHash<QString,QMap<QString,double>> videoFrangiParameters;
    QHash<QString,QMap<QString,double>> videoFrangiRatios;
    QHash<QString,QMap<QString,int>> videoFrangiMargins;
    QHash<QString,cv::Point3d> videoFrangiMaximum;
};

#endif // SHAREDVARIABLES_H
