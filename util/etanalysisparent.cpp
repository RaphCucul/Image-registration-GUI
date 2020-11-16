#include "etanalysisparent.h"
#include "util/files_folders_operations.h"
#include "shared_staff/sharedvariables.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

ETanalysisParent::ETanalysisParent(QWidget *parent) : QWidget(parent)
{

}

void ETanalysisParent::checkInputNumber(double i_input, double i_lower, double i_upper, QLineEdit *i_editWidget, double &i_finalValue, bool &i_evaluation){
    if (i_input < i_lower || (i_input > i_upper && i_upper != 0.0)){
        i_editWidget->setStyleSheet("color: #FF0000");
        i_editWidget->setText("");
        i_finalValue = -99;
        i_evaluation = false;
    }
    else if (i_input < i_lower || (i_upper == 0.0 && i_input == 0.0)){
        i_editWidget->setStyleSheet("color: #FF0000");
        i_editWidget->setText("");
        i_finalValue = -99;
        i_evaluation = false;
    }
    else{
        i_editWidget->setStyleSheet("color: #33aa00");
        i_evaluation = true;
        i_finalValue = i_input;
    }
}

void ETanalysisParent::initMaps(){
    //QVector<double> pomD;
    //QVector<int> pomI;
    // QMap<i_videoName,QMap<videoParameter,QVector<T>>>
    //for (int index = 0; index < videoParameters.count(); index++){
        //foreach (QString name, i_videoNames){
            //if (index < 8){ // entropy -> angle
            //if (index <= 8) {
                /*QMap<QString,QVector<double>> _pom;
                _pom.insert(name,pomD);
                mapDouble.insert(videoParameters.at(index),_pom);*/
                mapDouble = initMap<double>(videoParameters,videoNamesList,0,8);
            //}
            //else if (index >= 8 && index < 13){ // evaluation -> secondEval
            //else if (index > 8 && index <= 13) {
                /*QMap<QString,QVector<int>> _pom;
                _pom.insert(name,pomI);
                mapInt.insert(videoParameters.at(index),_pom);*/
                mapInt = initMap<int>(videoParameters,videoNamesList,9,13);
            //}
            //else if (index > 13 && index <= 15) {
                mapAnomalies = initAnomaly(videoParameters,videoNamesList,14,15);
            //}
            /*else if (index >= 13 && index <= 14){ // standard + extra
                QMap<QString,cv::Rect> _pom;
                cv::Rect _rect(0,0,0,0);
                _pom.insert(name,_rect);
                mapAnomalies.insert(videoParameters.at(index),_pom);
            }
            else if (index == 15){ // thresholds
                QMap<QString,QVector<double>> _pom;
                _pom.insert(name,pomD);
                mapThresholds.insert(videoParameters.at(index),_pom);
            }*/
        //}
    //}
}

QMap<QString,QMap<QString,cv::Rect>> ETanalysisParent::initAnomaly(QStringList parameters,
                                                        QStringList i_videoList,
                                                        int start_pos,
                                                        int end_pos) {
    QMap<QString,QMap<QString,cv::Rect>> outputMap;
    cv::Rect _r(0,0,0,0);
    for (int parameterIndex = start_pos; parameterIndex <= end_pos; parameterIndex++){
        foreach (QString name, i_videoList){
            QMap<QString,cv::Rect> _pom;
            _pom.insert(name,_r);
            outputMap.insert(parameters.at(parameterIndex),_pom);
        }
    }
    return outputMap;
}

void ETanalysisParent::cancelAllCalculations(){
    if (!First.isEmpty()) {
        if (First[1]->isRunning()){
            First[1]->terminate();
            First[1]->wait(100);
        }
        First[1]->deleteLater();
    }

    if (!Second.isEmpty()){
        if (Second[2]->isRunning()){
            Second[2]->terminate();
            Second[2]->wait(100);
        }
        Second[2]->deleteLater();
    }

    if (!Third.isEmpty()){
        if(Third[3]->isRunning()){
            Third[3]->terminate();
            Third[3]->wait(100);
        }
        Third[3]->deleteLater();
    }

    if (!Fourth.isEmpty()){
        if (Fourth[4]->isRunning()){
            Fourth[4]->terminate();
            Fourth[4]->wait(100);
        }
        Fourth[4]->deleteLater();
    }

    if (!Fifth.isEmpty()){
        if (Fifth[5]->isRunning()){
            Fifth[5]->terminate();
            Fifth[5]->wait(100);
        }
        Fifth[5]->deleteLater();
    }

    initMaps();
    emit calculationStopped();
}

void ETanalysisParent::done(int done)
{
    qDebug()<<"Thread "<<done<<" done, processing data...";
    if (done == 1)
    {
        badVideos = First[1]->unprocessableVideos();
        qDebug()<<"Bad videos list from thread "<<done<<": "<<badVideos;
        foreach (QString name,videoNamesList) {
            if (badVideos.indexOf(name) == -1){
                qDebug()<<"Processing data from thread "<<done<<" for video "<<name;
                mapDouble["entropy"][name] = First[1]->computedEntropy()[name];
                mapDouble["tennengrad"][name] = First[1]->computedTennengrad()[name];
                mapInt["firstEvalEntropy"][name] = First[1]->computedFirstEntropyEvaluation()[name];
                mapInt["firstEvalTennengrad"][name] = First[1]->computedFirstTennengradEvalueation()[name];
                mapAnomalies["standard"][name] = First[1]->computedCOstandard()[name];
                mapAnomalies["extra"][name] = First[1]->computedCOextra()[name];
                mapInt["evaluation"][name] = First[1]->computedCompleteEvaluation()[name];                
                badFramesComplete.insert(name,First[1]->computedBadFrames()[name]);
                framesReferencial.insert(name,First[1]->estimatedReferencialFrames()[name]);
                framesReferentialFrangiCoordinates.insert(name,First[1]->estimatedReferentialFrangiCoordinates()[name]);

                if (temporalySavedThresholds.keys().indexOf(name) == -1)
                    mapDouble["thresholds"][name] = First[1]->computedThresholds()[name];
                else
                    mapDouble["thresholds"][name] = temporalySavedThresholds[name];
            }
        }
        emit dataObtained_first();
    }
    if (done == 2)
    {
        badVideos.clear();
        badVideos = Second[2]->unprocessableVideos();
        qDebug()<<"Bad videos list from thread "<<done<<": "<<badVideos;
        foreach (QString name,videoNamesList) {
            if (badVideos.indexOf(name) == -1){
                qDebug()<<"Processing data from thread "<<done<<" for video "<<name;
                averageCCcomplete[name] = Second[2]->computedCC()[name];
                averageFWHMcomplete[name] = Second[2]->computedFWHM()[name];
            }
        }
        emit dataObtained_second();
    }
    if (done == 3)
    {
        badVideos.clear();
        badVideos = Third[3]->unprocessableVideos();
        qDebug()<<"Bad videos list from thread "<<done<<": "<<badVideos;
        foreach (QString name, videoNamesList){
            if (badVideos.indexOf(name) == -1){
                qDebug()<<"Processing data from thread "<<done<<" for video "<<name;
                mapInt["evaluation"][name] = Third[3]->framesUpdateEvaluation()[name];
                mapInt["firstEval"][name] = Third[3]->framesFirstEvaluationComplete()[name];
                CC_problematicFrames[name] = Third[3]->framesProblematic_CC()[name];
                FWHM_problematicFrames[name] = Third[3]->framesProblematic_FWHM()[name];
                mapDouble["FrangiX"][name] = Third[3]->framesFrangiXestimated()[name];
                mapDouble["FrangiY"][name] = Third[3]->framesFrangiYestimated()[name];
                mapDouble["FrangiEuklid"][name] = Third[3]->framesFrangiEuklidestimated()[name];
                mapDouble["POCX"][name] = Third[3]->framesPOCXestimated()[name];
                mapDouble["POCY"][name] = Third[3]->framesPOCYestimated()[name];
                mapDouble["angle"][name] = Third[3]->framesUhelestimated()[name];
            }
        }
        emit dataObtained_third();
    }
    if (done == 4)
    {
        badVideos.clear();
        badVideos = Fourth[4]->unprocessableVideos();
        qDebug()<<"Bad videos list from thread "<<done<<": "<<badVideos;
        foreach (QString name, videoNamesList){
            if (badVideos.indexOf(name) == -1){
                qDebug()<<"Processing data from thread "<<done<<" for video "<<name;
                mapInt["evaluation"][name] = Fourth[4]->framesUpdateEvaluationComplete()[name];
                mapInt["secondEval"][name] = Fourth[4]->framesSecondEvaluation()[name];
                mapDouble["FrangiX"][name] = Fourth[4]->framesFrangiXestimated()[name];
                mapDouble["FrangiY"][name] = Fourth[4]->framesFrangiYestimated()[name];
                mapDouble["FrangiEuklid"][name] = Fourth[4]->framesFrangiEuklidestimated()[name];
                mapDouble["POCX"][name] = Fourth[4]->framesPOCXestimated()[name];
                mapDouble["POCY"][name] = Fourth[4]->framesPOCYestimated()[name];
                mapDouble["angle"][name] = Fourth[4]->framesAngleestimated()[name];
            }
        }
        emit dataObtained_fourth();
    }
    if (done == 5)
    {
        badVideos.clear();
        badVideos = Fifth[5]->unprocessableVideos();
        qDebug()<<"Bad videos list from thread "<<done<<": "<<badVideos;
        foreach (QString name, videoNamesList){
            if (badVideos.indexOf(name) == -1){
                qDebug()<<"Processing data from thread "<<done<<" for video "<<name;
                mapDouble["FrangiX"][name] = Fifth[5]->framesFrangiXestimated()[name];
                mapDouble["FrangiY"][name] = Fifth[5]->framesFrangiYestimated()[name];
                mapDouble["FrangiEuklid"][name] = Fifth[5]->framesFrangiEuklidestimated()[name];
                mapDouble["POCX"][name] = Fifth[5]->framesPOCXestimated()[name];
                mapDouble["POCY"][name] = Fifth[5]->framesPOCYestimated()[name];
                mapDouble["angle"][name] = Fifth[5]->framesAngleestimated()[name];
                mapInt["evaluation"][name] = Fifth[5]->framesUpdateEvaluationComplete()[name];
            }
        }
        emit dataObtained_fifth();
    }
}

void ETanalysisParent::saveVideoAnalysisResults() {
    foreach (QString name, videoNamesList){
        if (badVideos.indexOf(name) == -1){
            QJsonDocument document;
            QJsonObject object;
            QString path = SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+name+".dat";
            mapInt["evaluation"][name][framesReferencial[name]] = 2;
            mapDouble["FrangiX"][name][framesReferencial[name]] = framesReferentialFrangiCoordinates[name].x;
            mapDouble["FrangiY"][name][framesReferencial[name]] = framesReferentialFrangiCoordinates[name].y;

            object = maps2Object(videoParameters,name,mapDouble,mapInt,mapAnomalies);

            document.setObject(object);
            QString documentString = document.toJson();
            QFile writer;
            writer.setFileName(path);
            writer.open(QIODevice::WriteOnly);
            writer.write(documentString.toLocal8Bit());
            writer.close();
        }
    }
}

void ETanalysisParent::saveVideoAnalysisResultsFromGraphET(QString i_videoName, QJsonObject i_object) {
    QJsonDocument document;
    QJsonObject object;
    QString path = SharedVariables::getSharedVariables()->getPath("datFilesPath")+"/"+i_videoName+".dat";
    if (mapDouble.contains("FrangiX") && mapDouble.contains("FrangiY")) {
        if (mapDouble["FrangiX"].contains(i_videoName) && mapDouble["FrangiY"].contains(i_videoName)) {
            if (mapDouble["FrangiX"][i_videoName].length() >= framesReferencial[i_videoName] &&
                mapDouble["FrangiY"][i_videoName].length() >= framesReferencial[i_videoName]) {
                mapDouble["FrangiX"][i_videoName][framesReferencial[i_videoName]] = framesReferentialFrangiCoordinates[i_videoName].x;
                mapDouble["FrangiY"][i_videoName][framesReferencial[i_videoName]] = framesReferentialFrangiCoordinates[i_videoName].y;
            }
        }
    }
    if (mapInt.contains("evaluation")) {
        if (mapInt["evaluation"].contains(i_videoName)) {
            if (mapInt["evaluation"][i_videoName].length() >= framesReferencial[i_videoName])
                mapInt["evaluation"][i_videoName][framesReferencial[i_videoName]] = 2;
        }
    }

    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 9){
            QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][i_videoName];
            QJsonArray pomArray = vector2array(pomDouble);
            object[videoParameters.at(parameter)] = pomArray;
        }
        else if (parameter >= 9 && parameter <= 13){
            QVector<int> pomInt = mapInt[videoParameters.at(parameter)][i_videoName];
            if (videoParameters.at(parameter) == "evaluation")
                pomInt[framesReferencial[i_videoName]]=2;

            QJsonArray pomArray = vector2array(pomInt);
            object[videoParameters.at(parameter)] = pomArray;
        }
        else if (parameter > 13) {
            cv::Rect _pomRect = mapAnomalies[videoParameters.at(parameter)][i_videoName];
            QJsonArray pomArray = vector2array(convertRect2Vector(_pomRect));
            object[videoParameters.at(parameter)] = pomArray;
        }
    }
    // it is possible that evaluation or thresholds could have been changed -> process i_object input
    foreach (QString key, i_object.keys()) {
        object[key] = i_object[key];
    }

    document.setObject(object);
    QString documentString = document.toJson();
    QFile writer;
    writer.setFileName(path);
    writer.open(QIODevice::WriteOnly);
    writer.write(documentString.toLocal8Bit());
    writer.close();
}
