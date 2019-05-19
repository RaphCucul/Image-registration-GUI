#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVector>

#include "util/souborove_operace.h"
using namespace std;

void analyseFileNames(QString i_chosenPathToFiles,
                                QStringList &i_filenameList,
                                int &i_filenameWithSuffixCount,
                                QString i_searchedSuffix)
{
     QDir chosenDirectory(i_chosenPathToFiles);
     i_filenameList = chosenDirectory.entryList(QStringList() << "*."+i_searchedSuffix << "*."+i_searchedSuffix.toUpper(),QDir::Files);
     i_filenameWithSuffixCount = i_filenameList.size();
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

QJsonArray vector2array(QVector<double> &i_vector)
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
}

QVector<int> arrayInt2vector(QJsonArray& i_array)
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
