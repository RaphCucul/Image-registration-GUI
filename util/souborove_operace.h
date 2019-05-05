#ifndef SOUBOROVE_OPERACE_H_INCLUDED
#define SOUBOROVE_OPERACE_H_INCLUDED

#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonArray>
#include <QVector>
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
QJsonArray vector2array(QVector<double>& i_vector);

/**
 * @brief
 * @param vector
 * @return
 */
QJsonArray vector2array(QVector<int>& i_vector);

/**
 * @brief
 * @param array
 * @return
 */
QVector<int> arrayInt2vector(QJsonArray& i_array);

/**
 * @brief
 * @param array
 * @return
 */
QVector<double> arrayDouble2vector(QJsonArray& i_array);
#endif // SOUBOROVE_OPERACE_H_INCLUDED
