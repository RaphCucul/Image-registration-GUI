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
 * @file files_folders_operations.h
 * The file contains declarations of functions supporting different operations with files and folders.
 **/

/**
 * @brief It analyses the content of the provided directory and tries to find all files with the given
 * suffix.
 * @param[in] chosenPathToFiles
 * @param[out] filenameList
 * @param[out] filenameWithSuffixCount
 * @param[in] searchedSuffix
 */
void analyseFileNames(QString i_chosenPathToFiles,
                      QStringList &i_filenameList,
                      int &i_filenameWithSuffixCount,
                      QString i_searchedSuffix);

/**
 * @brief It splits given file path into pieces to provide folder path, filename and suffix of the chosen
 * file.
 * @param[in] wholePaht
 * @param[out] folder
 * @param[out] onlyFilename
 * @param[out] suffix
 */
void processFilePath(QString i_wholePaht, QString& i_folder, QString& i_onlyFilename, QString& i_suffix);

/**
 * @brief Ut reads a file with json-style saved variables.
 * @param[in] file
 * @return Processed content of the input file.
 */
QJsonObject readJson(QFile& i_file);

/**
 * @brief It saves data in json-style format into the file.
 * @param[in] object
 * @param[in] array
 * @param[in] type
 * @param[in] pathAndName
 */
void writeJson(QJsonObject& i_object, QJsonArray& i_array, QString i_type, QString i_pathAndName);

/**
 * @brief Template function converts given vector of type T into QJsonArray.
 * @tparam T - vector data type
 */
template <class T>
QJsonArray vector2array(QVector<T> i_vector){
    QJsonArray array;
    copy(i_vector.begin(), i_vector.end(), back_inserter(array));
    return array;
}

/**
 * @brief Template function converts given QJsonArray to vector.
 * @tparam T - data type of the output vector
 * @code array2vector<double>(doubleTypeVector)
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

/**
 * @brief It converts QMap objects into a single QJsonObject file.
 * @param[in] i_parameters  - analysed parameters (POCX, POCY, standard, extra, entropy, tennengrad, ...)
 * @param[in] i_videoName - name of the video for the specification of video-related information
 * @param[in] i_mapDouble
 * @param[in] i_mapInt
 * @param[in] i_mapAnomaly
 * @return Json object with complete video information prepared to be processed
 */
QJsonObject maps2Object(QStringList i_parameters, QString i_videoName,
                        QMap<QString,QMap<QString,QVector<double>>> i_mapDouble,
                        QMap<QString,QMap<QString,QVector<int>>> i_mapInt,
                        QMap<QString,QMap<QString,cv::Rect>> i_mapAnomaly);

/**
 * @brief Template function checks the presence of specific *.dat file and tries to load data of
 * the specific saved parameter.
 * @tparam i_parameter - specifies what should be analysed
 * @tparam i_videoName - name of the video for the specification of video-related information
 * @tparam T - vector data type
 */
template <class T>
bool checkAndLoadData(QString i_parameter, QString i_videoName, QVector<T>& o_vector) {
    QString actualDatFilePath = SharedVariables::getSharedVariables()->getPath("datFilesPath");
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
 * @brief It converts QRectF object to cv::Rect object.
 * @param[in] i_input - QRectF object(s)
 */
QMap<QString,cv::Rect> convertQRectToRect(QMap<QString,QRectF> i_input);

/**
 * @brief Converts QRectF object to cv::Rect object.
 * @param i_input
 */
cv::Rect convertQRectToRect(QRect i_input);

/**
 * @brief Converts cv::Rect object into a vector.
 * @param[in] i_rectangular
 */
QVector<int> convertRect2Vector(cv::Rect i_rectangular);

/**
 * @brief Converts vector into a cv::Rect object.
 * @param i_vector
 */
cv::Rect convertVector2Rect(QVector<int> i_vector);

/**
 * @brief It checks the existence of the referential frame in the *.dat file of the given video.
 * @param[in] i_folder - specifies where the *.dat should be
 * @param[in] i_videoName - name of the video
 * @param[in] i_referentialFrame - expected index of the referential frame
 * @return True if the index of the referential frame in the file is the same as the input. Otherwise, returns false.
 */
bool checkReferentialFrameExistence(QString i_folder,QString i_videoName,int i_referentialFrame);

/**
 * @brief It checks the existence of the referential frame in the *.dat file of the given video and returns frangi
 * coordinates if present.
 * @param i_name - name of the video
 * @param i_referentialFrame - indef of the referential frame
 * @param i_point - maximum frangi coordinates
 * @return True if frangi maximum coordinates were found. Otherwise, returns false.
 */
bool findReferentialFrameData(QString i_name, int& i_referentialFrame, QPoint& i_point);
#endif // FILES_FOLDERS_OPERATIONS_H_INCLUDED
