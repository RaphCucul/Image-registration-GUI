#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVector>

#include "util/files_folders_operations.h"
using namespace std;

void analyseFileNames(QString i_chosenPathToFiles,
                                QStringList &i_filenameList,
                                int &i_filenameWithSuffixCount,
                                QString i_searchedSuffix)
{
     QDir chosenDirectory(i_chosenPathToFiles);
     i_filenameList = chosenDirectory.entryList(QStringList() << "*."+i_searchedSuffix << "*."+i_searchedSuffix.toUpper(),QDir::Files);
     i_filenameWithSuffixCount = i_filenameList.count();
}

void processFilePath(QString i_wholePaht, QString& i_folder, QString& i_onlyFilename, QString& i_suffix)
{
    int lastindexSlash = i_wholePaht.lastIndexOf("/");
    int lastIndexDot = i_wholePaht.length() - i_wholePaht.lastIndexOf(".");
    i_folder = i_wholePaht.left(lastindexSlash);
    i_onlyFilename = i_wholePaht.mid(lastindexSlash+1,
         (i_wholePaht.length()-lastindexSlash-lastIndexDot-1));
    i_suffix = i_wholePaht.right(lastIndexDot-1);
}

QJsonObject readJson(QFile& i_file)
{
    QByteArray val;
    QJsonParseError* error = nullptr;
    i_file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = i_file.readAll();
    i_file.close();
    //qDebug() << val;
    QJsonDocument d = QJsonDocument::fromJson(val,error);
    QJsonObject sett2 = d.object();
    return sett2;
}

void writeJson(QJsonObject &i_object, QJsonArray &i_array, QString i_type, QString i_pathAndName)
{
    i_object[i_type] = i_array;
    QJsonDocument document;
    document.setObject(i_object);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName(i_pathAndName);
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

/*QJsonArray vector2array(QVector<double> &i_vector)
{
    QJsonArray array;
    copy(i_vector.begin(), i_vector.end(), back_inserter(array));
    return array;
}

QJsonArray vector2array(QVector<int>& i_vector)
{
    QJsonArray array;
    copy(i_vector.begin(), i_vector.end(), back_inserter(array));
    return array;
}*/

/*QVector<int> arrayInt2vector(QJsonArray& i_array)
{
    QVector<int> output;
    for (int indexarray = 0; indexarray < i_array.size(); indexarray++){
        output.push_back(i_array[indexarray].toInt());
    }
    return output;
}

QVector<double> arrayDouble2vector(QJsonArray& i_array)
{
    QVector<double> output;
    for (int indexarray = 0; indexarray < i_array.size(); indexarray++){
        output.push_back(i_array[indexarray].toDouble());
    }
    return output;
}*/

QJsonObject maps2Object(QStringList i_parameters,
                        QString i_videoName,
                        QMap<QString,QMap<QString,QVector<double>>> i_mapDouble,
                        QMap<QString,QMap<QString,QVector<int>>> i_mapInt,
                        QMap<QString, QMap<QString, cv::Rect> > i_mapAnomaly) {

    QJsonObject _returnObject;
    for (int parameter = 0; parameter < i_parameters.count(); parameter++){
        //qDebug()<<videoParameters.at(parameter);
        if (parameter <= 8){
            QVector<double> pomDouble = i_mapDouble[i_parameters.at(parameter)][i_videoName];
            QJsonArray pomArray = vector2array(pomDouble);
            _returnObject[i_parameters.at(parameter)] = pomArray;
        }
        else if (parameter > 8 && parameter <= 13){
            QVector<int> pomInt = i_mapInt[i_parameters.at(parameter)][i_videoName];
            /*if (i_parameters.at(parameter) == "evaluation")
                pomInt[framesReferencial[i_videoName]]=2;*/
            QJsonArray pomArray = vector2array(pomInt);
            _returnObject[i_parameters.at(parameter)] = pomArray;
        }
        else if (parameter > 13 && parameter <= 15) {
            QVector<int> pomInt = convertRect2Vector(i_mapAnomaly[i_parameters.at(parameter)][i_videoName]);
            QJsonArray pomArray = vector2array(pomInt);
            _returnObject[i_parameters.at(parameter)] = pomArray;
        }
    }
    return _returnObject;
}

QMap<QString,cv::Rect> convertQRectToRect(QMap<QString,QRectF> i_input){
    QMap<QString,cv::Rect> _output;
    _output["extra"].x = int(i_input["extra"].x());
    _output["extra"].y = int(i_input["extra"].y());
    _output["extra"].width = int(i_input["extra"].width());
    _output["extra"].height = int(i_input["extra"].height());

    _output["standard"].x = int(i_input["standard"].x());
    _output["standard"].y = int(i_input["standard"].y());
    _output["standard"].width = int(i_input["standard"].width());
    _output["standard"].height = int(i_input["standard"].height());

    return _output;
}

cv::Rect convertQRectToRect(QRect i_input) {
    cv::Rect _output;
    _output.x = i_input.x();
    _output.y = i_input.y();
    _output.width = i_input.width();
    _output.height = i_input.height();

    return _output;
}

QVector<int> convertRect2Vector(cv::Rect i_rectangular) {
    QVector<int> _returnVector;

    _returnVector.push_back(i_rectangular.x);
    _returnVector.push_back(i_rectangular.y);
    _returnVector.push_back(i_rectangular.width);
    _returnVector.push_back(i_rectangular.height);

    return _returnVector;
}

cv::Rect convertVector2Rect(QVector<int> i_vector) {
    cv::Rect _returnRect;
    _returnRect.x = i_vector[0];
    _returnRect.y = i_vector[1];
    _returnRect.width = i_vector[2];
    _returnRect.height = i_vector[3];
    return _returnRect;
}

bool checkReferentialFrameExistence(QString i_folder, QString i_videoName, int i_referentialFrame){
    QFile file;
    file.setFileName(i_folder+"/"+i_videoName+".dat");
    if (file.exists()){
        // if the file exists, try to find referential frame info
        QJsonObject data = readJson(file);
        QJsonArray dataArray = data["evaluation"].toArray();

        // if the user set referential frame number, check it first
        if (i_referentialFrame != -1){
            if (dataArray.at(i_referentialFrame).toInt() == 2)
                return true;
            else
                return false;
        }
        else
            return false;
    }
    else // the dat file not found -> terminate with true -> the error label will not be activated
        return true;
}

bool findReferentialFrameData(QString i_name, int &i_referentialFrame, QPoint& i_point){
    QString actualDatFilePath = SharedVariables::getSharedVariables()->getPath("datFilesPath");
    // get *.dat file
    QFile file;
    file.setFileName(actualDatFilePath+"/"+i_name+".dat");
    if (file.exists()){
        // if the file exists, try to find referential frame info
        QJsonObject data = readJson(file);
        QJsonArray dataArray = data["evaluation"].toArray();

        // user set referential frame - it can be true referential frame
        // but it could not be -> -1 indicates a user does not know referential frame
        if (i_referentialFrame == -1){
            for (int i=0;i<dataArray.count();i++){
                if (dataArray.at(i).toInt() == 2){
                    i_referentialFrame = i;
                    break;
                }
                else
                    continue;
            }
        }

        if (i_referentialFrame == -1) // referential frame was not found in the file
            return false;
        else{
            QJsonArray _frangiX = data["FrangiX"].toArray();
            QJsonArray _frangiY = data["FrangiY"].toArray();
            // the frangi coordinates still can be (0,0)
            if (!_frangiX.at(i_referentialFrame).isUndefined() && !_frangiY.at(i_referentialFrame).isUndefined()) {
                if (_frangiX.at(i_referentialFrame).toDouble() != 0.0 && _frangiY.at(i_referentialFrame).toDouble() != 0.0){
                    i_point.setX(_frangiX.at(i_referentialFrame).toInt());
                    i_point.setY(_frangiY.at(i_referentialFrame).toInt());
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
        }
    }
    else
        return false;
}
