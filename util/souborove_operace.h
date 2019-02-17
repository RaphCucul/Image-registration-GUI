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
void analyseFileNames(QString chosenPathToFiles,
                                QStringList &filenameList,
                                int &filenameWithSuffixCount,
                                QString searchedSuffix);

/**
 * @brief Function split given file path into pieces to provide folder path, filename and suffix of the chosen
 * file
 * @param wholePaht
 * @param folder
 * @param onlyFilename
 * @param suffix
 */
void processFilePath(QString wholePaht, QString& folder, QString& onlyFilename, QString& suffix);

QJsonObject readJson(QFile& file);
void writeJson(QJsonObject& object, QJsonArray& array, QString type, QString pathAndName);
QJsonArray vector2array(QVector<double>& vector);
QJsonArray vector2array(QVector<int>& vector);
QVector<int> arrayInt2vector(QJsonArray& array);
QVector<double> arrayDouble2vector(QJsonArray& array);
#endif // SOUBOROVE_OPERACE_H_INCLUDED
