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

    QString getPath(QString type) const; /// functions enable to save path into *.json file
    void setPath(QString type, QString path);

    void processFrangiParameters(QString path);
    QVector<double> getFrangiParameters() const;
    double getSpecificFrangiParameter(int parameter);
    void setSpecificFrangiParameter(int parameter,double value);
    void saveFrangiParameters();

    cv::Point3d getFrangiMaximum();
    void setFrangiMaximum(cv::Point3d coordinates);

    cv::Point2d getHorizontalAnomalyCoords();
    void setHorizontalAnomalyCoords(QPointF coords);
    cv::Point2d getVerticalAnomalyCoords();
    void setVerticalAnomalyCoords(QPointF coords);

private:
    SharedVariables();
    /**
     * @brief Function fills vector of Frangi parameters with values from JSON object
     * @param loadedObject
     * @param parameter
     * @param loadedParameters
     * @param position
     */
    void inicialization_frangi_opt(QJsonObject loadedObject, QString parameter, QVector<double>& loadedParameters,
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
    QStringList FrangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    cv::Point3d detectedFrangiMaximum;
    cv::Point2d horizontalAnomalyCoords;
    cv::Point2d verticalAnomalyCoords;

};

#endif // SHAREDVARIABLES_H
