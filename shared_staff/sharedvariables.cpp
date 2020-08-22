#include "shared_staff/sharedvariables.h"
#include "util/files_folders_operations.h"
#include "shared_staff/globalsettings.h"
#include "image_analysis/frangi_utilization.h"

#include <QCoreApplication>
#include <QFile>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <exception>

SharedVariables* SharedVariables::g_sharedVariables = nullptr;

SharedVariables::SharedVariables()
{
    chosenActualPathes["videosPath"] = "";
    chosenActualPathes["saveVideosPath"] = "";
    chosenActualPathes["datFilesPath"] = "";
    chosenActualPathes["parametersFrangiFiltr"] = "";

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
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
bool SharedVariables::processFrangiParameters(QString path){
    qDebug()<<"Processing Frangi parameters into a vector from path "<<path;
    bool processingResult = true;

    QFile file;
    file.setFileName(path+"/frangiParameters.json");
    QJsonObject FrangiParametersObject;
    FrangiParametersObject = readJson(file);
    if (FrangiParametersObject.isEmpty()){
        processingResult = false;
    }
    if (processingResult){
        size_frangi_opt(FrangiParametersList.count(),FrangiParameters);
        for (int a = 0; a < FrangiParametersList.count(); a++){
            if (!inicialization_frangi_opt(FrangiParametersObject,FrangiParametersList.at(a),FrangiParameters,a)){
                processingResult = false;
                break;
            }
        }
        foreach (QString margin,FrangiMarginsList){
            frangiMargins[margin] = FrangiParametersObject[margin].toInt();
        }
        foreach (QString ratio,FrangiRatiosList){
            frangiRatios[ratio] = FrangiParametersObject[ratio].toDouble();
        }
    }    
    return processingResult;
}

bool SharedVariables::processVideoFrangiParameters(QString videoName, QMap<QString, QVariant> &obtainedParameters) {
    qDebug()<<"Processing frangi parameters for video "<<videoName;
    bool processingResult = false;
    QFile file;
    QString path = chosenActualPathes["datFilesPath"];
    file.setFileName(path+"/"+videoName+".dat");
    if (file.exists()) {
        QJsonObject VideoParametersObject = readJson(file);
        if (VideoParametersObject.isEmpty() || VideoParametersObject["FrangiInformation"].isNull())
            return processingResult;
        else {
            // Frangi information is added to videoInformation variable as a QMap
            QJsonObject frangiParametersObject = VideoParametersObject["FrangiInformation"].toObject();
            foreach(QString name,FrangiParametersList){
                if (!frangiParametersObject[name].isNull()) {
                    obtainedParameters[name] = frangiParametersObject[name].toDouble();
                    videoFrangiParameters[videoName][name] = frangiParametersObject[name].toDouble();
                }
                else
                    return false;
            }
            foreach(QString name,FrangiMarginsList){
                if (!frangiParametersObject[name].isNull()) {
                    obtainedParameters[name] = frangiParametersObject[name].toInt();
                    videoFrangiMargins[videoName][name] = frangiParametersObject[name].toInt();
                }
                else
                    return false;
            }
            foreach(QString name,FrangiRatiosList){
                if (!frangiParametersObject[name].isNull()) {
                    obtainedParameters[name] = frangiParametersObject[name].toDouble();
                    videoFrangiRatios[videoName][name] = frangiParametersObject[name].toDouble();
                }
                else
                    return false;
            }

            processingResult = true;
            return processingResult;
        }
    }
    else return processingResult;
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
/*QVector<double> SharedVariables::getFrangiParameters() const{
    return FrangiParameters;
}*/

QMap<QString,double> SharedVariables::getFrangiParameters() const{
    return FrangiParametersMap;
}

QMap<QString,double> SharedVariables::getVideoFrangiParameters(QString i_videoName) const {
    if (videoFrangiParameters.contains(i_videoName))
        return videoFrangiParameters[i_videoName];
    else
        return getFrangiParameters();
}

QMap<QString,double> SharedVariables::getFrangiParameterWrapper(frangiType i_type, QString i_videoName) const{
    if (i_type == frangiType::GLOBAL)
        return getFrangiParameters();
    else
        return getVideoFrangiParameters(i_videoName);
}

double SharedVariables::getSpecificFrangiParameter(int parameter) {
    return data_from_frangi_opt(parameter,FrangiParameters);
}

double SharedVariables::getSpecificFrangiParameter(QString parameter) {
    return FrangiParametersMap[parameter];
}

double SharedVariables::getSpecificVideoFrangiParameter(QString i_videoName, QString parameter) {
    if (videoFrangiParameters.contains(i_videoName)) {
        if (videoFrangiParameters[i_videoName].contains(parameter))
            return videoFrangiParameters[i_videoName][parameter];
        else
            return getSpecificFrangiParameter(parameter);
    }
    else
        return getSpecificFrangiParameter(parameter);
}

double SharedVariables::getSpecificFrangiParameterWrapper(frangiType i_type, QString i_videoName, QString parameter) {
    if (i_type == frangiType::GLOBAL)
        return getSpecificFrangiParameter(parameter);
    else
        return getSpecificVideoFrangiParameter(i_videoName,parameter);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::size_frangi_opt(int size, QVector<double>& loadedParameters){
    loadedParameters = (QVector<double>(size));
}

bool SharedVariables::inicialization_frangi_opt(QJsonObject loadedObject, QString parameter, QVector<double>& loadedParameters,
                             int &position)
{
    try {
        loadedParameters[position] = loadedObject[parameter].toDouble();
        FrangiParametersMap[parameter] = loadedObject[parameter].toDouble();
        return true;
    } catch (std::exception& e) {
        qDebug()<<"An error occured when loading frangi parameters: "<<e.what();
        return false;
    }
}

double SharedVariables::data_from_frangi_opt(int position, QVector<double>& loadedParameters) {
    return loadedParameters[position];
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::setSpecificFrangiParameter(int parameter, double value){
    FrangiParameters[parameter] = value;
    FrangiParametersMap[FrangiParametersList.at(parameter)] = value;
}

void SharedVariables::setSpecificVideoFrangiParameter(QString i_videoName, QString i_parameter, double i_value) {
    videoFrangiParameters[i_videoName][i_parameter] = i_value;
}

void SharedVariables::setSpecificFrangiParameterWrapper(frangiType i_type, double i_value, QString i_videoName,
                                                        QString i_parameter, int i_param) {
    if (i_type == frangiType::GLOBAL)
        setSpecificFrangiParameter(i_param, i_value);
    else
        setSpecificVideoFrangiParameter(i_videoName,i_parameter,i_value);
}

void SharedVariables::setFrangiParameters(QMap<QString, double> i_parameters) {
    foreach(QString parameter,FrangiParametersList) {
        if (i_parameters.contains(parameter)) {
            FrangiParametersMap[parameter] = i_parameters[parameter];
        }
    }
}

void SharedVariables::setVideoFrangiParameters(QString i_videoName, QMap<QString, double> i_parameters) {
    videoFrangiParameters[i_videoName] = i_parameters;
}

void SharedVariables::setFrangiParameterWrapper(frangiType i_type, QString i_videoName,
                                                QMap<QString, double> i_parameters) {
    if (i_type == frangiType::GLOBAL)
        setFrangiParameters(i_parameters);
    else
        setVideoFrangiParameters(i_videoName,i_parameters);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::saveFrangiParameters(){
    QJsonDocument document;
    QJsonObject object;
    QString whereToSaveFrangi = chosenActualPathes["parametersFrangiFiltr"];
    qDebug()<<"Saving Frangi parameters to "<<whereToSaveFrangi;
    for (int indexParameter=0; indexParameter < FrangiParametersList.count(); indexParameter++)
        object[FrangiParametersList.at(indexParameter)] = FrangiParameters[indexParameter];
    for (int marginIndex = 0; marginIndex < FrangiMarginsList.count(); marginIndex++)
        object[FrangiMarginsList.at(marginIndex)] = frangiMargins[FrangiMarginsList.at(marginIndex)];
    for (int ratioIndex = 0; ratioIndex < FrangiMarginsList.count(); ratioIndex++)
        object[FrangiRatiosList.at(ratioIndex)] = frangiRatios[FrangiRatiosList.at(ratioIndex)];
    document.setObject(object);
    QString documentString = document.toJson();
    QFile writer;
    writer.setFileName(whereToSaveFrangi+"/frangiParameters.json");
    writer.open(QIODevice::WriteOnly);
    writer.write(documentString.toLocal8Bit());
    writer.close();
}

void SharedVariables::saveVideoFrangiParameters(QString i_videoName) {
    QJsonDocument document;
    QJsonObject object;
     QJsonObject FrangiInformation;
    QString whereToSaveFrangi = chosenActualPathes["datFilesPath"];
    // first check if video file already exists
    QFile file(whereToSaveFrangi+"/"+i_videoName+".dat");
    if (file.exists()) {
        object = readJson(file);
        foreach (QString parameter, FrangiParametersList)
            FrangiInformation[parameter] = videoFrangiParameters[i_videoName][parameter];
        foreach (QString margin, FrangiMarginsList)
            FrangiInformation[margin] = videoFrangiMargins[i_videoName][margin];
        foreach (QString ratio, FrangiRatiosList)
            FrangiInformation[ratio] = videoFrangiRatios[i_videoName][ratio];
    }
    object["FrangiInformation"] = FrangiInformation;
    document.setObject(object);
    QString documentString = document.toJson();
    QFile writer;
    writer.setFileName(whereToSaveFrangi+"/"+i_videoName+".dat");
    writer.open(QIODevice::WriteOnly);
    writer.write(documentString.toLocal8Bit());
    writer.close();
}

void SharedVariables::saveFrangiParametersWrapper(frangiType i_type, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        saveFrangiParameters();
    else
        saveVideoFrangiParameters(i_videoName);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
cv::Point3d SharedVariables::getFrangiMaximum(){
    return detectedFrangiMaximum;
}

cv::Point3d SharedVariables::getVideoFrangiMaximum(QString i_videoName) {
    if (videoFrangiMaximum.contains(i_videoName))
        return videoFrangiMaximum[i_videoName];
    else
        return getFrangiMaximum();
}

cv::Point3d SharedVariables::getFrangiMaximumWrapper(frangiType i_type, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        return getFrangiMaximum();
    else
        return getVideoFrangiMaximum(i_videoName);
}

void SharedVariables::setFrangiMaximum(cv::Point3d i_coordinates){
    detectedFrangiMaximum = i_coordinates;
}

void SharedVariables::setVideoFrangiMaximum(QString i_videoName, cv::Point3d i_coordinates) {
    videoFrangiMaximum[i_videoName] = i_coordinates;
}

void SharedVariables::setFrangiMaximumWrapper(frangiType i_type, QString i_videoName,
                                              cv::Point3d i_coordinates) {
    if (i_type == frangiType::GLOBAL)
        setFrangiMaximum(i_coordinates);
    else
        setVideoFrangiMaximum(i_videoName,i_coordinates);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
cv::Rect SharedVariables::getStandardCutout(){
    cv::Rect outputRectangle;
    populateOutputRectangle(standardCutoutRectangle,outputRectangle);
    return outputRectangle;
}

void SharedVariables::setStandardCutout(QRect i_newStandardCutout){
    qDebug()<<"Setting standard cutout: "<<i_newStandardCutout;
    standardCutoutRectangle = i_newStandardCutout;
}

cv::Rect SharedVariables::getExtraCutout(){
    cv::Rect outputRectangle;
    populateOutputRectangle(standardCutoutRectangle,outputRectangle);
    return outputRectangle;
}

void SharedVariables::setExtraCutout(QRect i_newExtraCutout){
    qDebug()<<"Setting extra cutout: "<<i_newExtraCutout;
    extraCutoutRectangle = i_newExtraCutout;
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::populateOutputRectangle(QRect &i_qRect, cv::Rect &i_cvRect){
    i_cvRect.x = i_qRect.x();
    i_cvRect.y = i_qRect.y();
    i_cvRect.width = i_qRect.width();
    i_cvRect.height = i_qRect.height();
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
QMap<QString,double> SharedVariables::getFrangiRatios() {
    return frangiRatios;
}

QMap<QString,double> SharedVariables::getVideoFrangiRatios(QString i_videoName) {
    if (videoFrangiRatios.contains(i_videoName))
        return videoFrangiRatios[i_videoName];
    else
        return getFrangiRatios();
}

QMap<QString,double> SharedVariables::getFrangiRatiosWrapper(frangiType i_type, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        return getFrangiRatios();
    else
        return getVideoFrangiRatios(i_videoName);
}

double SharedVariables::getSpecificFrangiRatio(QString i_ratioName) {
    return frangiRatios[i_ratioName];
}

double SharedVariables::getSpecificVideoFrangiRatio(QString i_videoName, QString i_ratioName) {
    if (videoFrangiRatios.contains(i_videoName) && videoFrangiRatios.contains(i_ratioName))
        return videoFrangiRatios[i_videoName][i_ratioName];
    else
        return getSpecificFrangiRatio(i_ratioName);
}

double SharedVariables::getSpecificFrangiRatioWrapper(frangiType i_type, QString i_ratioName, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        return getSpecificFrangiRatio(i_ratioName);
    else
        return getSpecificVideoFrangiRatio(i_videoName,i_ratioName);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::setFrangiRatios(QMap<QString, double> i_ratios){
    frangiRatios = i_ratios;
}

void SharedVariables::setVideoFrangiRatios(QString i_videoName, QMap<QString, double> i_ratios) {
    videoFrangiRatios[i_videoName] = i_ratios;
}

void SharedVariables::setFrangiRatiosWrapper(frangiType i_type, QMap<QString, double> i_ratios, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        setFrangiRatios(i_ratios);
    else
        setVideoFrangiRatios(i_videoName,i_ratios);
}

void SharedVariables::setSpecificFrangiRatio(QString i_ratioName, double i_value) {
    frangiRatios[i_ratioName] = i_value;
}

void SharedVariables::setSpecificVideoFrangiRatio(QString i_videoName, QString i_ratioName, double i_value) {
    videoFrangiRatios[i_videoName][i_ratioName] = i_value;
}

void SharedVariables::setSpecificFrangiRatioWrapper(frangiType i_type, QString i_ratioName,
                                                    QString i_videoName, double i_value) {
    if (i_type == frangiType::GLOBAL)
        setSpecificFrangiRatio(i_ratioName,i_value);
    else
        setSpecificVideoFrangiRatio(i_videoName,i_ratioName,i_value);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
QMap<QString,int> SharedVariables::getFrangiMargins(){
    return frangiMargins;
}

QMap<QString,int> SharedVariables::getVideoFrangiMargins(QString i_videoName) {
    if (videoFrangiMargins.contains(i_videoName))
        return videoFrangiMargins[i_videoName];
    else
        return getFrangiMargins();
}

QMap<QString,int> SharedVariables::getFrangiMarginsWrapper(frangiType i_type, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        return getFrangiMargins();
    else
        return getVideoFrangiMargins(i_videoName);
}

int SharedVariables::getSpecificFrangiMargin(QString i_marginName) {
    return frangiMargins[i_marginName];
}

int SharedVariables::getSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName) {
    if (videoFrangiMargins.contains(i_videoName) && videoFrangiRatios.contains(i_marginName))
        return videoFrangiMargins[i_videoName][i_marginName];
    else
        return getSpecificFrangiMargin(i_marginName);
}

int SharedVariables::getSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        return getSpecificFrangiMargin(i_marginName);
    else
        return getSpecificVideoFrangiMargin(i_videoName,i_marginName);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::setFrangiMargins(QMap<QString, int> i_margins){
    frangiMargins = i_margins;
}

void SharedVariables::setVideoFrangiMargins(QString i_videoName, QMap<QString, int> i_margins) {
    videoFrangiMargins[i_videoName] = i_margins;
}

void SharedVariables::setFrangiMarginsWrapper(frangiType i_type, QMap<QString, int> i_margins, QString i_videoName) {
    if (i_type == frangiType::GLOBAL)
        setFrangiMargins(i_margins);
    else
        setVideoFrangiMargins(i_videoName,i_margins);
}

void SharedVariables::setSpecificFrangiMargin(QString i_marginName, int i_value) {
    frangiMargins[i_marginName] = i_value;
}

void SharedVariables::setSpecificVideoFrangiMargin(QString i_videoName, QString i_marginName, int i_value) {
    videoFrangiMargins[i_videoName][i_marginName] = i_value;
}

void SharedVariables::setSpecificFrangiMarginWrapper(frangiType i_type, QString i_marginName,
                                                     QString i_videoName, int i_value) {
    if (i_type == frangiType::GLOBAL)
        setSpecificFrangiMargin(i_marginName,i_value);
    else
        setSpecificVideoFrangiMargin(i_videoName,i_marginName,i_value);
}
//---------------------------------------------------------------------------//
//-^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^-//
//---------------------------------------------------------------------------//
void SharedVariables::setVideoInformation(QString i_video, QString i_key, QVariant i_dataToSave){
    videoInformation[i_video].insert(i_key,i_dataToSave);
}

QVariant SharedVariables::getVideoInformation(QString i_video, QString i_key){
    return videoInformation[i_video][i_key];
}

QMap<QString, QVariant> SharedVariables::getCompleteVideoInformation(QString i_video){
    return videoInformation[i_video];
}

bool SharedVariables::checkVideoInformationPresence(QString i_video){
    if (videoInformation.contains(i_video))
        return true;
    else
        return false;
}

//**************************************************************************************************//
//**************************************************************************************************//

FrangiThread::FrangiThread(const cv::Mat i_inputFrame, int i_processingMode, int i_accuracy,
                           int i_showResult, QString i_windowName, int i_frameType,
                           cv::Point3d i_translation, QVector<double> i_FrangiParameters,
                           QMap<QString, int> i_margins)
{
    _translation = (i_translation);
    _inputFrameForAnalysis = (i_inputFrame);
    _processingMode = (i_processingMode);
    _accuracy = (i_accuracy);
    _showResult = (i_showResult);
    _frameType = (i_frameType);
    _windowName = (i_windowName);
    QStringList _l = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    for (int index = 0; index < i_FrangiParameters.length(); index++) {
        _FrangiParameters[_l.at(index)] = i_FrangiParameters[index];
    }
    _margins = (i_margins);
}

FrangiThread::FrangiThread(const cv::Mat i_inputFrame, int i_processingMode, int i_accuracy,
             int i_showResult, QString i_windowName, int i_frameType,
             cv::Point3d i_translation, QMap<QString,double> i_FrangiParameters,
             QMap<QString, int> i_margins)
{
    _translation = (i_translation);
    _inputFrameForAnalysis = (i_inputFrame);
    _processingMode = (i_processingMode);
    _accuracy = (i_accuracy);
    _showResult = (i_showResult);
    _frameType = (i_frameType);
    _windowName = (i_windowName);
    _margins = (i_margins);
    _FrangiParameters.clear();
    _FrangiParameters = i_FrangiParameters;
}

void FrangiThread::startFrangiAnalysis(){
    qDebug()<<"Starting calculations";
    cv::Point3d calculatedData = frangi_analysis(_inputFrameForAnalysis,_processingMode,_accuracy,_showResult,
                                                 _windowName,_frameType,_translation,_FrangiParameters,_margins);
    qDebug()<<"Done.";
    QPoint sendResult;
    if (calculatedData.z == 0.0){
        sendResult.setX(-999);
        sendResult.setY(-999);
    }
    else{
        sendResult.setX(int(calculatedData.x));
        sendResult.setY(int(calculatedData.y));
    }

    emit finishedCalculation(sendResult);
}

/*cv::Point3d FrangiThread::provideCalculatedData(){
    qDebug()<<"Retrieving data";
    return calculatedData;
}*/

//**************************************************************************************************//
//**************************************************************************************************//

ProcessingIndicator::ProcessingIndicator(QString i_description, QWidget* i_widgetWithIndicator,
                                         QWidget *parent) : QWidget(parent){
    gifContent = new QMovie(":/images/processing.gif");
    gifLabel = new QLabel();
    gifLabel->setMovie(gifContent);
    indicatorText = i_description;
    widgetWithIndicator = i_widgetWithIndicator;
    gifLabel->installEventFilter(this);
}

ProcessingIndicator::~ProcessingIndicator(){
    delete gifLabel;
    delete gifContent;
}

void ProcessingIndicator::placeIndicator(){
    // parents
    QWidget* parentOfWidget = widgetWithIndicator->parentWidget();
    errorWidgetParent = new QWidget(parentOfWidget);
    errorWidgetParent->setLayout(new QHBoxLayout());
    errorWidgetParent->layout()->setMargin(0);
    errorWidgetParent->layout()->setContentsMargins(0,0,0,0);
    // errorLabel placement
    errorWidgetParent->layout()->addWidget(gifLabel);
    // positioning
    int widgetWidth = parentOfWidget->width();
    int widgetHeight = parentOfWidget->height();
    QPoint mappingCoordinates = widgetWithIndicator->mapTo(parentOfWidget,QPoint(widgetWidth/2,widgetHeight/2));
    errorWidgetParent->move(mappingCoordinates + QPoint(-50, -50));
    qDebug()<<"Mapping coordinates "<<mappingCoordinates;

    if (!alreadyPlaced)
        alreadyPlaced=true;
}

void ProcessingIndicator::hideIndicatorSlot(){
    hideIndicator();
}

void ProcessingIndicator::hideIndicator(){
    if (alreadyPlaced){
        qDebug()<<"Hiding indicator";
        errorWidgetParent->setVisible(false);
        gifContent->stop();
    }
}

void ProcessingIndicator::showIndicator(){
    if (alreadyPlaced){
        qDebug()<<"Showing indicator";
        errorWidgetParent->setVisible(true);
        gifContent->start();
        showMessage();
    }
}

void ProcessingIndicator::showMessage(){
    gifLabel->setToolTip(indicatorText);
}
