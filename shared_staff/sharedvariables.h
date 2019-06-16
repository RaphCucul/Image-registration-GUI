#ifndef SHAREDVARIABLES_H
#define SHAREDVARIABLES_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPointF>
#include <opencv2/opencv.hpp>

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
     * @brief Function returns the vector of frangi parameters in predefined order for better processing.
     * @return
     */
    QVector<double> getFrangiParameters() const;

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
     * @brief Function set the value of specific parameter.
     * @param parameter
     * @param value
     */
    void setSpecificFrangiParameter(int parameter,double value);

    /**
     * @brief Function saves actual frangi parameters to the file in the chosen actual directory.
     */
    void saveFrangiParameters();

    /**
     * @brief If Franfi filtr was applied on the frame,
     * the coordinates of the maximum can be read by this function.
     * @return
     */
    cv::Point3d getFrangiMaximum();

    /**
     * @brief Store the latest coordinates of Frangi filtr maximum.
     * @param coordinates
     */
    void setFrangiMaximum(cv::Point3d coordinates);

    cv::Point2d getHorizontalAnomalyCoords();
    void setHorizontalAnomalyCoords(QPointF coords);
    cv::Point2d getVerticalAnomalyCoords();
    void setVerticalAnomalyCoords(QPointF coords);

    void setFrangiMargins(QMap<QString,int> i_margins);
    QMap<QString,int> getFrangiMargins();
    void setFrangiRatios(QMap<QString,double> i_ratios);
    QMap<QString,double> getFrangiRatios();

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

    static SharedVariables *g_sharedVariables;
    QMap<QString,QString> chosenActualPathes;
    QVector<double> FrangiParameters;
    QMap<QString,double> FrangiParametersMap;
    QStringList FrangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    QStringList FrangiMarginsList = {"top_m","bottom_m","left_m","right_m"};
    QStringList FrangiRatiosList = {"top_r","bottom_r","left_r","right_r"};
    cv::Point3d detectedFrangiMaximum;
    cv::Point2d horizontalAnomalyCoords;
    cv::Point2d verticalAnomalyCoords;
    QMap<QString,double> frangiRatios;
    QMap<QString,int> frangiMargins;

};

#endif // SHAREDVARIABLES_H
