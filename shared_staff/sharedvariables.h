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
 * how many videos will be processed and what type of cutout is being edited. Because RegistrateTwo class
 * enables to registrate two images, ClickImageEvent class must be prepared for this case too.
 * @namespace clickImageEnums
 */
namespace clickImageEnums {
    /**
     * @enum videoCount
     * @brief videoCount enum helps to determine what type of ClickImageEvent constructor should be used. The constructor
     * then defines everything else.
     */
    enum videoCount {ONE_VIDEO,MULTIPLE_VIDEOS,NO_VIDEO};
    /**
     * @enum cutoutType
     * @brief The cutoutType enum helps to determine which cutout is being determined
     */
    enum cutoutType {STANDARD=1,EXTRA,NO_CUTOUT};
    /**
     * @enum chosenSource
     * @brief The chosenSource enum helps to determine what layout in RegistrateTwo class was activated
     */
    enum  chosenSource {VIDEO,IMAGE,NOTHING};
}
/**
 * @namespace frangiEnums
 * @brief frangiEnums namespace helps to define if the requested/provided frangi parameter(s) should be global
 * or video specific.
 */
namespace frangiEnums {
    /**
     * @enum frangiType
     * @brief The frangiType enum helps to determine which frangi parameters are requested - global or video specific.
     * This enum is used together with wrapper functions described below.
     */
    enum frangiType {GLOBAL,VIDEO_SPECIFIC};
}

using namespace frangiEnums;

/**
 * @class FrangiThread
 * @brief The FrangiThread class can be used as an independent caller of the Frangi analysis functions.
 * The class was created for the purpose of the ClickImageEvent class and the need to do Frangi analysis of a frame
 * chose by a user.
 * The main advantage of this class is the object can be moved to the separate thread so the main app windows remain
 * active and the application does not "freeze" during the calculation.
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
    /**
     * @brief Slot function initiates the start of the Frangi analysis.
     */
    void startFrangiAnalysis();    
signals:
    /**
     * @brief The signal emitted when the analysis is finished and the results can be processed.
     * @param[in] i_calculatedData - Frangi coordinates
     */
    void finishedCalculation(QPoint i_calculatedData);
private:
    cv::Point3d _translation;
    cv::Mat _inputFrameForAnalysis;
    int _processingMode=0, /**<Standard or reverse */
        _accuracy=0, /**<Subpixel or pixel precision */
        _showResult=0, /**< Detected coordinates should be shown */
        _frameType=0; /**< Determines how the frame should be preprocessed in terms of borders (margins) */
    QString _windowName = "";
    QMap<QString, double> _FrangiParameters;
    QMap<QString, int> _margins;
};

/**
 * @class ProcessingIndicator
 * @brief The ProcessingIndicator class is used in the ClickImageEvent class and fills the QGraphicsView during the
 * calculation of the Frangi maximum coordinates. The indicator should give a user the feeling the program is doing its job
 * and it is not irresponsive. The class works with the gif element from the resources.
 */
class ProcessingIndicator : public QWidget
{
    Q_OBJECT
public:
    ProcessingIndicator(QString i_description, QWidget* i_widgetWithIndicator, QWidget* parent = nullptr);
    ~ProcessingIndicator();
    /**
     * @brief The function adds the indicator to the QGraphicsView widget.
     * @sa wasPlaced()
     */
    void placeIndicator();
    /**
     * @brief The function enables to check if the indicator was/is placed or not.
     * @return Was placed status
     * @sa placeIndicator()
     */
    bool  wasPlaced(){ return alreadyPlaced; }
    /**
     * @brief The function hides the indicator if it is not necessary to show it.
     * @sa showIndicator()
     */
    void hideIndicator();
    /**
     * @brief The function shows the indicator if necessary.
     * @sa hideIndicator()
     */
    void showIndicator();
public slots:
    /**
     * @brief Slot function reacting on the signal the indicator can be hidden.
     */
    void hideIndicatorSlot();
private:
    /**
     * @brief The function adds a tooltip message with the defined text. If a mouse is moved over the indicator,
     * the message appears.
     */
    void showMessage();

    QLabel* gifLabel=nullptr;
    QMovie* gifContent=nullptr;
    QWidget* widgetWithIndicator=nullptr;
    QWidget* errorWidgetParent=nullptr;
    QString indicatorText = "";
    bool alreadyPlaced = false;
};

/**
 * @brief The SharedVariables class is provided as a singleton storing variables which must exist in the one
 *  instance only. Use the public functions of the class to access and modify private variables.
 * The functions are mostly setters and getters. The class is aimed to provide data of the frangi parameters, margins and ratios
 * (so-called frangi functions). It can be used to manipulate with actual directory paths too.
 * Frangi functions are encapsulated into the wrapper functions accessible publicly. This enables a user to choose
 * if the program should work with the global frangi parameters or video-specific parameters.
 * If the video-specific parameter is not found, the global parameter is used then.
 */
class SharedVariables
{
public:
    Q_DISABLE_COPY(SharedVariables)

    static SharedVariables *getSharedVariables();

    /**
     * @brief Get actual path for specific path type.
     * @param[in] type
     * @sa setPath(QString type, QString path)
     * @return actual path for the given type
     */
    QString getPath(QString type) const;

    /**
     * @brief Set actual path for specific path type.
     * @param[in] type
     * @param[in] path
     * @sa getPath(QString type)
     */
    void setPath(QString type, QString path);

    /**
     * @brief Loads data from *.json with frangi parameters and stores the data as global frangi parameters.
     * Predefined name of *.json file is expected!
     * @param[in] path
     * @return the result of the parameters processing
     */
    bool processFrangiParameters(QString path);

    /**
     * @brief Processes video-related frangi parameters stored in the *.dat file.
     * @param[in] videoName - name of the video
     * @param[in] obtainedParameters - complete QMap with all parameters
     * @return The result of the parameters processing
     */
    bool processVideoFrangiParameters(QString videoName, QMap<QString,QVariant>& obtainedParameters);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns frangi parameters - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_videoName
     * @sa getSpecificFrangiParameterWrapper(frangiType i_type, QString i_videoName, QString parameter)
     * @sa getFrangiParameters()
     * @sa getVideoFrangiParameters(QString i_videoName)
     */
    QMap<QString,double> getFrangiParameterWrapper(frangiType i_type, QString i_videoName) const;
    /**
     * @brief Returns specific frangi parameter - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_videoName
     * @param[in] parameter
     * @sa getFrangiParameterWrapper(frangiType i_type, QString i_videoName)
     * @sa getSpecificFrangiParameter(QString parameter)
     * @sa getSpecificFrangiParameter(int parameter)
     * @sa getSpecificVideoFrangiParameter(QString i_videoName, QString parameter)
     */
    double getSpecificFrangiParameterWrapper(frangiType i_type, QString i_videoName, QString parameter);
    /**
     * @brief Sets all frangi parameters at once - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_videoName
     * @param[in] i_parameters
     * @sa setSpecificFrangiParameterWrapper(frangiType i_type, double i_value, QString i_videoName,QString i_parameter, int i_param=-1)
     * @sa setFrangiParameters(QMap<QString,double> i_parameters)
     * @sa setVideoFrangiParameters(QString i_videoName,QMap<QString,double> i_parameters)
     */
    void setFrangiParameterWrapper(frangiType i_type,QString i_videoName,QMap<QString,double> i_parameters);
    /**
     * @brief Sets one specific frangi parameter - global-related or video-related. The specific parameter can be represented by a string
     * or number. Threfore, i_param is prepared for this situation.
     * @param[in] i_type
     * @param[in] i_value
     * @param[in] i_videoName
     * @param[in] i_parameter
     * @param[in] i_param
     * @sa setFrangiParameterWrapper(frangiType i_type,QString i_videoName,QMap<QString,double> i_parameters)
     * @sa setSpecificFrangiParameter(int parameter,double value)
     * @sa setSpecificVideoFrangiParameter(QString i_videoName, QString i_parameter,double i_value)
     */
    void setSpecificFrangiParameterWrapper(frangiType i_type, double i_value, QString i_videoName,
                                           QString i_parameter, int i_param=-1);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Saves stored frangi parameters - global-related or video-related - to a file. If global, main frangi JSON file is updated.
     * If video specific, specific video *.dat file is updated.
     * @param[in] i_type
     * @param[in] i_videoName
     * @sa saveFrangiParameters()
     * @sa saveVideoFrangiParameters(QString i_videoName)
     */
    void saveFrangiParametersWrapper(frangiType i_type,QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns detected frangi maximum coordinates - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_videoName
     * @return coordinates of the frangi maximum
     * @sa setFrangiMaximumWrapper(frangiType i_type, QString i_videoName,cv::Point3d i_coordinates);
     * @sa getFrangiMaximum()
     * @sa getVideoFrangiMaximum(QString i_videoName)
     */
    cv::Point3d getFrangiMaximumWrapper(frangiType i_type,QString i_videoName);
    /**
     * @brief Temporally saves detected frangi maximum coordinates - global-related or video-related - to a file. But the coordinates are
     * saved temporally in the memory, not into a file!
     * @param[in] i_type
     * @param[in] i_videoName
     * @param[in] i_coordinates
     * @sa getFrangiMaximumWrapper(frangiType i_type,QString i_videoName)
     * @sa setFrangiMaximum(cv::Point3d i_coordinates)
     * @sa setVideoFrangiMaximum(QString i_videoName, cv::Point3d i_coordinates)
     */
    void setFrangiMaximumWrapper(frangiType i_type, QString i_videoName,cv::Point3d i_coordinates);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Temporally saves all frangi margins - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_margins
     * @param[in] i_videoName
     * @sa setSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName, int i_value)
     * @sa setFrangiMargins(QMap<QString,int> i_margins)
     * @sa setVideoFrangiMargins(QString i_videoName, QMap<QString,int> i_margins)
     */
    void setFrangiMarginsWrapper(frangiType i_type, QMap<QString,int> i_margins, QString i_videoName);
    /**
     * @brief Temporally saves one specific frangi margin - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_marginName
     * @param[in] i_videoName
     * @param[in] i_value
     * @sa setFrangiMarginsWrapper(frangiType i_type, QMap<QString,int> i_margins, QString i_videoName)
     * @sa setSpecificFrangiMargin(QString i_marginName, int i_value)
     * @sa setSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName, int i_value)
     */
    void setSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName, int i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns all available frangi margins - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_videoName
     * @sa getSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName)
     * @sa getFrangiMargins()
     * @sa getVideoFrangiMargins(QString i_videoName)
     */
    QMap<QString,int> getFrangiMarginsWrapper(frangiType i_type, QString i_videoName);
    /**
     * @brief Returns the value of one specific margin - global or video specific.
     * @param[in] i_type
     * @param[in] i_marginName
     * @param[in] i_videoName
     * @sa getFrangiMarginsWrapper(frangiType i_type, QString i_videoName)
     * @sa getSpecificFrangiMargin(QString i_marginName)
     * @sa getSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName)
     */
    int getSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Temporally saves all frangi ratios - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_ratios
     * @param[in] i_videoName
     * @sa setSpecificFrangiRatioWrapper(frangiType i_type,QString i_ratioName,QString i_videoName,double i_value)
     * @sa setFrangiRatios(QMap<QString,double> i_ratios)
     * @sa setVideoFrangiRatios(QString i_videoName, QMap<QString,double> i_ratios)
     */
    void setFrangiRatiosWrapper(frangiType i_type, QMap<QString,double> i_ratios, QString i_videoName);
    /**
     * @brief Temporally saves one specific frangi ratio - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_ratioName
     * @param[in] i_videoName
     * @param[in] i_value
     * @sa setFrangiRatiosWrapper(frangiType i_type, QMap<QString,double> i_ratios, QString i_videoName)
     * @sa setSpecificFrangiRatio(QString i_ratioName,double i_value)
     * @sa setSpecificVideoFrangiRatio(QString i_videoName,QString i_ratioName,double i_value)
     */
    void setSpecificFrangiRatioWrapper(frangiType i_type,QString i_ratioName,QString i_videoName,double i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns all available frangi ratios - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_videoName
     * @sa getSpecificFrangiRatioWrapper(frangiType i_type,QString i_ratioName,QString i_videoName)
     * @sa getFrangiRatios()
     * @sa getVideoFrangiRatios(QString i_videoName)
     */
    QMap<QString,double> getFrangiRatiosWrapper(frangiType i_type,QString i_videoName);
    /**
     * @brief Returns one specific frangi ratio - global-related or video-related.
     * @param[in] i_type
     * @param[in] i_ratioName
     * @param[in] i_videoName
     * @sa getFrangiRatiosWrapper(frangiType i_type,QString i_videoName)
     * @sa getSpecificFrangiRatio(QString i_ratioName)
     * @sa getSpecificVideoFrangiRatio(QString i_videoName, QString i_ratioName)
     */
    double getSpecificFrangiRatioWrapper(frangiType i_type,QString i_ratioName,QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns cv::Rect object with information about standard cutout
     * @return standard cutout rectangular coordinates
     */
    cv::Rect getStandardCutout();

    /**
     * @brief Temporally saves QRect information about standard cutout.
     * @param[in] i_newStandardCutout
     */
    void setStandardCutout(QRect i_newStandardCutout);

    /**
     * @brief Returns cv::Rect object with information about extra cutout
     * @return extra cutout rectangular coordinates
     */
    cv::Rect getExtraCutout();

    /**
     * @brief Temporally saves QRect information about extra cutout.
     * @param[in] i_newExtraCutout
     */
    void setExtraCutout(QRect i_newExtraCutout);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Temporally saves necessary information about a video. This information is used
     * in different situations and also during the final saving of computed parameters.
     * @param[in] i_video
     * @param[in] i_key
     * @param[in] i_dataToSave
     * @sa getVideoInformation(QString i_video, QString i_key)
     */
    void setVideoInformation(QString i_video, QString i_key, QVariant i_dataToSave);

    /**
     * @brief Returns a specific video information of the defined video.
     * @param[in] i_video
     * @param[in] i_key
     * @sa setVideoInformation(QString i_video, QString i_key, QVariant i_dataToSave)
     */
    QVariant getVideoInformation(QString i_video, QString i_key);

    /**
     * @brief Returns all data connected to the given video in one variable.
     * @param[in] i_video
     */
    QMap<QString,QVariant> getCompleteVideoInformation(QString i_video);

    /**
     * @brief Makes sure that the video has its video information prepared to be processed.
     * @param[in] i_video
     */
    bool checkVideoInformationPresence(QString i_video);

private:
    SharedVariables();

    /**
     * @brief Fills vector of Frangi parameters with values from JSON object
     * @param[in] loadedObject
     * @param[in] parameter
     * @param[in] loadedParameters
     * @param[in] position
     */
    bool inicialization_frangi_opt(QJsonObject loadedObject, QString parameter, QVector<double>& loadedParameters,
                                 int &position);
    /**
     * @brief Vector of Frangi parameters is initialized to have specific size for better processing.
     * @param[in] size - requested size of the vector
     * @param[in] loadedParameters output vector prepared to be filled
     */
    void size_frangi_opt(int size, QVector<double>& loadedParameters);

    /**
     * @brief Returns a value of the specific parameter from the Frangi parameters vector.
     * @param[in] position
     * @param[in] loadedParameters
     */
    double data_from_frangi_opt(int position, QVector<double>& loadedParameters);

    /**
     * @brief Creates cv::Rect object with parameters of the QRect object. Basically conversion function.
     * @param[in] i_qRect
     * @param[in] i_cvRect
     */
    void populateOutputRectangle(QRect& i_qRect,cv::Rect& i_cvRect);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns the vector of frangi parameters in predefined order for better processing.
     * @return
     */
    //QVector<double> getFrangiParameters() const;
    /**
     * @brief Returns global frangi parameters.
     * @sa getVideoFrangiParameters(QString i_videoName) const
     */
    QMap<QString,double> getFrangiParameters() const;
    /**
     * @brief Returns video specific frangi parameters if available, otherwise the function returns global parameters.
     * @param[in] i_videoName
     * @sa getFrangiParameters()
     */
    QMap<QString,double> getVideoFrangiParameters(QString i_videoName) const;
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns the value of the specific global parameter identified by a name.
     * @param[in] parameter
     * @sa getSpecificFrangiParameter(int parameter)
     * @sa getSpecificVideoFrangiParameter(QString i_videoName, QString parameter)
     */
    double getSpecificFrangiParameter(QString parameter);
    /**
     * @brief Returns the value of the specific global parameter identified by the index in the vector.
     * @param[in] parameter
     * @sa getSpecificFrangiParameter(QString parameter)
     * @sa getSpecificVideoFrangiParameter(QString i_videoName, QString parameter)
     */
    double getSpecificFrangiParameter(int parameter);
    /**
     * @brief Returns the value of the video specific parameter identified by a name.
     * @param[in] i_videoName
     * @param[in] parameter
     * @sa getSpecificFrangiParameter(QString parameter)
     * @sa getSpecificFrangiParameter(int parameter)
     */
    double getSpecificVideoFrangiParameter(QString i_videoName, QString parameter);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Temporally saves global frangi parameters.
     * @param[in] i_parameters
     * @sa setVideoFrangiParameters(QString i_videoName,QMap<QString,double> i_parameters)
     */
    void setFrangiParameters(QMap<QString,double> i_parameters);
    /**
     * @brief Temporally saves video specific frangi parameters.
     * @param[in] i_videoName
     * @param[in] i_parameters
     * @sa setFrangiParameters(QMap<QString,double> i_parameters)
     */
    void setVideoFrangiParameters(QString i_videoName,QMap<QString,double> i_parameters);
    /**
     * @brief Sets the value of specific global parameter.
     * @param[in] parameter
     * @param[in] value
     * @sa setSpecificVideoFrangiParameter(QString i_videoName, QString i_parameter,double i_value)
     */
    void setSpecificFrangiParameter(int parameter,double value);
    /**
     * @brief Sets the value of specific video-related parameter.
     * @param[in] i_videoName
     * @param[in] i_parameter
     * @param[in] i_value
     * @sa setSpecificFrangiParameter(int parameter,double value)
     */
    void setSpecificVideoFrangiParameter(QString i_videoName, QString i_parameter,double i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Saves actual global frangi parameters to the file in the chosen directory.
     * @sa saveVideoFrangiParameters(QString i_videoName);
     */
    void saveFrangiParameters();
    /**
     * @brief saveVideoFrangiParameters
     * @param[in] i_videoName
     * @sa saveFrangiParameters()
     */
    void saveVideoFrangiParameters(QString i_videoName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Temporally saves global margins for frangi analysis.
     * @param[in] i_margins
     * @sa setVideoFrangiMargins(QString i_videoName, QMap<QString,int> i_margins)
     */
    void setFrangiMargins(QMap<QString,int> i_margins);
    /**
     * @brief Temporally saves video-related margins for frangi analysis.
     * @param[in] i_videoName
     * @param[in] i_margins
     * @sa setFrangiMargins(QMap<QString,int> i_margins)
     */
    void setVideoFrangiMargins(QString i_videoName, QMap<QString,int> i_margins);
    /**
     * @brief Sets a value to specific global-related frangi margin.
     * @param[in] i_marginName
     * @param[in] value
     * @sa setSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName, int i_value)
     */
    void setSpecificFrangiMargin(QString i_marginName, int i_value);
    /**
     * @brief Sets a value to specific video-related frangi margin.
     * @param[in] i_videoName
     * @param[in] i_marginName
     * @param[in] i_value
     * @sa setSpecificFrangiMargin(QString i_marginName, int i_value)
     */
    void setSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName, int i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns coordinates of the global-related frangi maximum.
     * @sa getVideoFrangiMaximum(QString i_videoName)
     */
    cv::Point3d getFrangiMaximum();
    /**
     * @brief Returns coordinates of the video-related frangi maximum.
     * @param[in] i_videoName
     * @sa getFrangiMaximum()
     */
    cv::Point3d getVideoFrangiMaximum(QString i_videoName);
    /**
     * @brief Temporally saves coordinates of the global-related frangi maximum.
     * @param[in] coordinates
     * @sa setVideoFrangiMaximum(QString i_videoName, cv::Point3d i_coordinates)
     */
    void setFrangiMaximum(cv::Point3d i_coordinates);
    /**
     * @brief Temporally saves coordinates of the video-related frangi maximum.
     * @param[in] i_videoName
     * @param[in] coordinates
     * @sa setFrangiMaximum(cv::Point3d i_coordinates)
     */
    void setVideoFrangiMaximum(QString i_videoName, cv::Point3d i_coordinates);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns global-related frangi margins.
     * @sa getVideoFrangiMargins(QString i_videoName)
     */
    QMap<QString,int> getFrangiMargins();
    /**
     * @brief Returns video-related frangi margins.
     * @param[in] i_videoName
     * @sa getFrangiMargins()
     */
    QMap<QString,int> getVideoFrangiMargins(QString i_videoName);
    /**
     * @brief Returns global-related specific frangi margin.
     * @param[in] i_marginName
     * @sa getSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName)
     */
    int getSpecificFrangiMargin(QString i_marginName);
    /**
     * @brief Returns video-related specific frangi margin.
     * @param[in] i_videoName
     * @param[in] i_marginName
     * @sa getSpecificFrangiMargin(QString i_marginName)
     */
    int getSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Temporally saves global-related frangi ratios.
     * @param[in] i_ratios
     * @sa setVideoFrangiRatios(QString i_videoName, QMap<QString,double> i_ratios)
     */
    void setFrangiRatios(QMap<QString,double> i_ratios);
    /**
     * @brief Temporally saves video-related frangi ratios.
     * @param[in] i_videoName
     * @param[in] i_ratios
     * @sa setFrangiRatios(QMap<QString,double> i_ratios)
     */
    void setVideoFrangiRatios(QString i_videoName, QMap<QString,double> i_ratios);
    /**
     * @brief Temporally saves global-related specific frangi ratio.
     * @param[in] i_ratioName
     * @param[in] i_value
     * @sa setSpecificVideoFrangiRatio(QString i_videoName,QString i_ratioName,double i_value)
     */
    void setSpecificFrangiRatio(QString i_ratioName,double i_value);
    /**
     * @brief Temporally saves global-related frangi ratios.
     * @param[in] i_videoName
     * @param[in] i_ratioName
     * @param[in] i_value
     * @sa setSpecificFrangiRatio(QString i_ratioName,double i_value)
     */
    void setSpecificVideoFrangiRatio(QString i_videoName,QString i_ratioName,double i_value);
    //---------------------------------------------------------------------------//
    //-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
    //---------------------------------------------------------------------------//
    /**
     * @brief Returns global-related frangi ratios.
     * @sa getVideoFrangiRatios(QString i_videoName)
     */
    QMap<QString,double> getFrangiRatios();

    /**
     * @brief Returns video-related frangi ratios.
     * @param[in] i_videoName
     * @sa getFrangiRatios()
     */
    QMap<QString,double> getVideoFrangiRatios(QString i_videoName);
    /**
     * @brief Returns global-related specific frangi ratio.
     * @param[in] i_ratioName
     * @sa getSpecificVideoFrangiRatio(QString i_videoName, QString i_ratioName)
     */
    double getSpecificFrangiRatio(QString i_ratioName);

    /**
     * @brief Returns video-related specific frangi ratios.
     * @param[in] i_videoName
     * @param[in] i_ratioName
     * @sa getSpecificFrangiRatio(QString i_ratioName)
     */
    double getSpecificVideoFrangiRatio(QString i_videoName, QString i_ratioName);


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
