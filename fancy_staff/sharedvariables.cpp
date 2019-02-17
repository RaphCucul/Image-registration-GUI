#include "sharedvariables.h"
#include "util/souborove_operace.h"
#include "fancy_staff/globalsettings.h"

#include <QCoreApplication>
#include <QFile>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

SharedVariables* SharedVariables::g_sharedVariables = nullptr;

SharedVariables::SharedVariables()
{
    chosenActualPathes["cestaKvideim"] = "";
    chosenActualPathes["ulozeniVidea"] = "";
    chosenActualPathes["adresarTXT_nacteni"] = "";
    chosenActualPathes["adresarTXT_ulozeni"] = "";
    chosenActualPathes["parametryFrangiFiltr"] = "";
    horizontalAnomalyCoords = cv::Point2d(0.0,0.0);
    verticalAnomalyCoords = cv::Point2d(0.0,0.0);
    detectedFrangiMaximum = cv::Point3d(0.0,0.0,0.0);
}

SharedVariables *SharedVariables::getSharedVariables(){
    if (g_sharedVariables == nullptr)
        g_sharedVariables = new SharedVariables();
    return g_sharedVariables;
}

QString SharedVariables::getPath(QString type) const{
    return chosenActualPathes[type];
}

void SharedVariables::setPath(QString type, QString path){
    qDebug()<<"Settings new path "<<path<<" for "<<type;
    chosenActualPathes[type] = path;
}

void SharedVariables::processFrangiParameters(QString path){
    qDebug()<<"Processing Frangi parameters into a vector";
    QFile file;
    file.setFileName(path+"/frangiParameters.json");
    QJsonObject FrangiParametersObject;
    FrangiParametersObject = readJson(file);
    size_frangi_opt(FrangiParametersList.count(),FrangiParameters);
    for (int a = 0; a < FrangiParametersList.count(); a++)
        inicialization_frangi_opt(FrangiParametersObject,FrangiParametersList.at(a),FrangiParameters,a);
}

QVector<double> SharedVariables::getFrangiParameters() const{
    return FrangiParameters;
}

double SharedVariables::getSpecificFrangiParameter(int parameter){
    return data_from_frangi_opt(parameter,FrangiParameters);
}

void SharedVariables::size_frangi_opt(int size, QVector<double>& loadedParameters){
    loadedParameters = (QVector<double>(size));
}

void SharedVariables::inicialization_frangi_opt(QJsonObject loadedObject, QString parameter, QVector<double>& loadedParameters,
                             int &position)
{
    loadedParameters[position] = loadedObject[parameter].toDouble();
}

double SharedVariables::data_from_frangi_opt(int position, QVector<double>& loadedParameters)
{
    return loadedParameters[position];
}

void SharedVariables::setSpecificFrangiParameter(int parameter, double value){
    FrangiParameters[parameter] = value;
}

void SharedVariables::saveFrangiParameters(){
    QJsonDocument document;
    QJsonObject object;
    QString whereToSaveFrangi = chosenActualPathes["parametryFrangiFiltr"];
    qDebug()<<"Saving Frangi parameters to "<<whereToSaveFrangi;
    for (int indexParameter=0; indexParameter < FrangiParametersList.count(); indexParameter++)
        object[FrangiParametersList.at(indexParameter)] = FrangiParameters[indexParameter];
    document.setObject(object);
    QString documentString = document.toJson();
    QFile writer;
    writer.setFileName(whereToSaveFrangi);
    writer.open(QIODevice::WriteOnly);
    writer.write(documentString.toLocal8Bit());
    writer.close();
}

cv::Point3d SharedVariables::getFrangiMaximum(){
    return detectedFrangiMaximum;
}

void SharedVariables::setFrangiMaximum(cv::Point3d coordinates){
    detectedFrangiMaximum = coordinates;
}

cv::Point2d SharedVariables::getHorizontalAnomalyCoords(){
    return horizontalAnomalyCoords;
}

void SharedVariables::setHorizontalAnomalyCoords(QPointF coords){
    horizontalAnomalyCoords.x = double(coords.x());
    horizontalAnomalyCoords.y = double(coords.y());
}

cv::Point2d SharedVariables::getVerticalAnomalyCoords(){
    return verticalAnomalyCoords;
}

void SharedVariables::setVerticalAnomalyCoords(QPointF coords){
    verticalAnomalyCoords.x = double(coords.x());
    verticalAnomalyCoords.y = double(coords.y());
}
