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
    velikost_frangi_opt(FrangiParametersList.count(),FrangiParameters);
    for (int a = 0; a < FrangiParametersList.count(); a++)
        inicializace_frangi_opt(FrangiParametersObject,FrangiParametersList.at(a),FrangiParameters,a);
}

QVector<double> SharedVariables::getFrangiParameters() const{
    return FrangiParameters;
}

double SharedVariables::getSpecificFrangiParameter(int parameter){
    return data_z_frangi_opt(parameter,FrangiParameters);
}

void SharedVariables::velikost_frangi_opt(int velikost,QVector<double>& nacteneParametry){
    nacteneParametry = (QVector<double>(velikost));
}

void SharedVariables::inicializace_frangi_opt(QJsonObject nactenyObjekt, QString parametr, QVector<double>& nacteneParametry,
                             int &pozice)
{
    nacteneParametry[pozice] = nactenyObjekt[parametr].toDouble();
}

double SharedVariables::data_z_frangi_opt(int pozice,QVector<double>& nacteneParametry)
{
    return nacteneParametry[pozice];
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
