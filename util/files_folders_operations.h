#ifndef FILES_FOLDERS_OPERATIONS_H_INCLUDED
#define FILES_FOLDERS_OPERATIONS_H_INCLUDED

#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonArray>
#include <QVector>
#include <QMap>
#include <QRectF>
#include <typeinfo>
#include <opencv2/opencv.hpp>

#include "shared_staff/sharedvariables.h"
#include "util/vector_operations.h"
using namespace std;
/**
 * @brief Funcion analyse the content of the provided directory, tries to find all files with the given
 * suffix
 * @param chosenPathToFiles
 * @param filenameList
 * @param filenameWithSuffixCount
 * @param searchedSuffix
 */
void analyseFileNames(QString i_chosenPathToFiles,
                      QStringList &i_filenameList,
                      int &i_filenameWithSuffixCount,
                      QString i_searchedSuffix);

/**
 * @brief Function split given file path into pieces to provide folder path, filename and suffix of the chosen
 * file
 * @param wholePaht
 * @param folder
 * @param onlyFilename
 * @param suffix
 */
void processFilePath(QString i_wholePaht, QString& i_folder, QString& i_onlyFilename, QString& i_suffix);

/**
 * @brief Function reads the file with json-style saved variables.
 * @param file
 * @return
 */
QJsonObject readJson(QFile& i_file);

/**
 * @brief Function saves the data in json-style format into the file.
 * @param object
 * @param array
 * @param type
 * @param pathAndName
 */
void writeJson(QJsonObject& i_object, QJsonArray& i_array, QString i_type, QString i_pathAndName);

/**
 * @brief
 * @param vector
 * @return
 */
//QJsonArray vector2array(QVector<double>& i_vector);

/**
 * @brief
 * @param vector
 * @return
 */
//QJsonArray vector2array(QVector<int>& i_vector);

/**
 * @brief
 * @param array
 * @return
 */
//QVector<int> arrayInt2vector(QJsonArray& i_array);

/**
 * @brief
 * @param array
 * @return
 */
//QVector<double> arrayDouble2vector(QJsonArray& i_array);

/**
 * @brief
 * @tparam T
 */
template <class T>
QJsonArray vector2array(QVector<T> i_vector){
    QJsonArray array;
    copy(i_vector.begin(), i_vector.end(), back_inserter(array));
    return array;
}

/**
 * @brief
 * @tparam T
 */
template <class T>
QVector<T> array2vector(QJsonArray i_array){
    QVector<T> output;
    for (T indexarray = T(0); indexarray < T(i_array.size()); indexarray++){
        if (typeid(T) == typeid(int))
            output.push_back(i_array[indexarray].toInt());
        else
            output.push_back(i_array[indexarray].toDouble());
    }
    return output;
}

QJsonObject maps2Object(QStringList i_parameters, QString i_videoName,
                        QMap<QString,QMap<QString,QVector<double>>> i_mapDouble,
                        QMap<QString,QMap<QString,QVector<int>>> i_mapInt,
                        QMap<QString,QMap<QString,cv::Rect>> i_mapAnomaly);

/**
 * @brief
 * @tparam i_parameter
 * @tparam i_videoName
 * @tparam T vector type to work with
 */
template <class T>
bool checkAndLoadData(QString i_parameter, QString i_videoName, QVector<T>& o_vector) {
    QString actualDatFilePath = SharedVariables::getSharedVariables()->getPath("saveDatFilesPath");
    QFile file;
    file.setFileName(actualDatFilePath+"/"+i_videoName+".dat");
    if (file.exists()) {
        QJsonObject data = readJson(file);
        if (!data[i_parameter].isUndefined()) {
            QJsonArray dataArray = data[i_parameter].toArray();
            QVector<T> _v = array2vector<T>(dataArray);
            if (vectorSum(_v) > 0) {
                o_vector = _v;
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
        return false;
}

/**
 * @brief convertQRectToRect
 * @param i_input
 * @return
 */
QMap<QString,cv::Rect> convertQRectToRect(QMap<QString,QRectF> i_input);

/**
 * @brief convertRect2Vector
 * @param i_rectangular
 * @return
 */
QVector<int> convertRect2Vector(cv::Rect i_rectangular);

/**
 * @brief checkReferentialFrameExistence
 * @param i_folder
 * @param i_videoName
 * @param i_referentialFrame
 * @return
 */
bool checkReferentialFrameExistence(QString i_folder,QString i_videoName,int i_referentialFrame);

/**
 * @brief checkAndLoadData
 * @param i_parameter
 * @return
 */
//bool checkAndLoadData(QString i_parameter, QString i_videoName, QString i_type, QVector<T> &o_vector);

/**
 * @brief findReferentialFrameData
 * @return
 */
bool findReferentialFrameData(QString i_name, int& i_referentialFrame, QPoint& i_point);
#endif // FILES_FOLDERS_OPERATIONS_H_INCLUDED
