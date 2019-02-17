#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

#include "util/souborove_operace.h"
using namespace std;

void analyseFileNames(QString chosenPathToFiles,
                                QStringList &filenameList,
                                int &filenameWithSuffixCount,
                                QString searchedSuffix)
{
     QDir chosenDirectory(chosenPathToFiles);
     filenameList = chosenDirectory.entryList(QStringList() << "*."+searchedSuffix << "*."+searchedSuffix.toUpper(),QDir::Files);
     filenameWithSuffixCount = filenameList.size();
}

void processFilePath(QString wholePaht, QString& folder, QString& onlyFilename, QString& suffix)
{
    int lastindexSlash = wholePaht.lastIndexOf("/");
    int lastIndexDot = wholePaht.length() - wholePaht.lastIndexOf(".");
    folder = wholePaht.left(lastindexSlash);
    onlyFilename = wholePaht.mid(lastindexSlash+1,
         (wholePaht.length()-lastindexSlash-lastIndexDot-1));
    suffix = wholePaht.right(lastIndexDot-1);
}

QJsonObject readJson(QFile& file)
{
    QByteArray val;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    //qDebug() << val;
    QJsonDocument d = QJsonDocument::fromJson(val);
    QJsonObject sett2 = d.object();
    //qDebug()<<sett2;
    //QJsonArray value = sett2["cestaKvideim"].toArray();
    //qDebug()<<value.size()<<value[0].toString();
    //QJsonValue value2 = value[0].toValue();
    return sett2;
}

void writeJson(QJsonObject &object, QJsonArray &array, QString type, QString pathAndName)
{
    object[type] = array;
    QJsonDocument document;
    document.setObject(object);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName(pathAndName);
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

QJsonArray vector2array(QVector<double> &vector)
{
    QJsonArray pole;
    copy(vector.begin(), vector.end(), back_inserter(pole));
    return pole;
}

QJsonArray vector2array(QVector<int>& vector)
{
    QJsonArray pole;
    copy(vector.begin(), vector.end(), back_inserter(pole));
    return pole;
}

QVector<int> arrayInt2vector(QJsonArray& array)
{
    QVector<int> output;
    for (int indexPole = 0; indexPole < array.size(); indexPole++){
        output.push_back(array[indexPole].toInt());
    }
    return output;
}

QVector<double> arrayDouble2vector(QJsonArray& array)
{
    QVector<double> output;
    for (int indexPole = 0; indexPole < array.size(); indexPole++){
        output.push_back(array[indexPole].toDouble());
    }
    return output;
}
