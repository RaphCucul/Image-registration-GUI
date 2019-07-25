#include "globalsettings.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

#include <iostream>
#include <fstream>
#include <windows.h>
#include <io.h>

#include "util/files_folders_operations.h"
using std::wstring;

QStringList readAndConvert(std::string i_fullFilePath){
    QStringList results;
    string line = "";
    ifstream myfile (i_fullFilePath,ios::binary);
    std::filebuf* pbuf = myfile.rdbuf();
    std::size_t size = pbuf->pubseekoff(0,myfile.end,myfile.in);
    pbuf->pubseekpos (0,myfile.in);
    char* buffer=new char[size];
    pbuf->sgetn (buffer,size);
    //std::cout.write (buffer,size);
    char * pch,*pch2;
    pch=strchr(buffer,'[')+1;
    unsigned int start = pch-buffer;
    pch2 = strchr(buffer,']');
    unsigned int end = pch2-buffer;
    char *newChar = new char[end-start];
    newChar[end-start]='\0';
    strncpy(newChar,pch,(end-start));
    QString str = QString::fromUtf8(newChar);
    qDebug()<<str;
    results.append(str);
    int i=1;
    while (i < 2)
    {
        char *pch3,*pch4;
        pch3=strchr(buffer+start,'[')+1;
        unsigned int start2 = pch3-buffer;
        pch4 = strchr(pch3,']');
        unsigned int end2 = pch4-buffer;
        newChar[end2-start2]='\0';
        strncpy(newChar,pch3,(end2-start2));
        str = QString::fromUtf8(newChar);
        qDebug()<<str;
        i++;
    }
    delete[] buffer;
    results.append(str);
    return results;

}

GlobalSettings * GlobalSettings::m_Settings = nullptr;

GlobalSettings::GlobalSettings()
{
    iniPath = QCoreApplication::applicationDirPath();
    QString pom = iniPath+"/"+iniFileName;
    qDebug()<<"Expected *.ini path: "<<pom;
    settings = new QSettings(pom, QSettings::IniFormat);
}

GlobalSettings *GlobalSettings::getSettings()
{
    if (m_Settings == nullptr)
    {
        m_Settings =  new GlobalSettings();
    }
    return m_Settings;
}

QString GlobalSettings::getFileFolderDirectoriesPath() const
{
    qDebug()<<"Actual file folder directory: "<<settings->value("fileFolderDirectoryPath").toString();
    QString pom = settings->value("fileFolderDirectoryPath").toString();
    return pom;
}

void GlobalSettings::setFileFolderDirectoriesPath(const QString &input)
{
    settings->setValue("fileFolderDirectoryPath", input);
    qDebug()<<"Saving file folder directory: "<<settings->value("fileFolderDirectoryPath").toString();
    settings->sync();
}

QString GlobalSettings::getIniPath() const
{
    return iniPath;
}

void GlobalSettings::setIniPath(const QString &inputPath, const QString &inputFile){
    iniPath = inputPath;
    iniFileName = inputFile;
    QString pom = iniPath+"/"+iniFileName;
    qDebug()<<"User choice expected *.ini path: "<<pom;
    settings = new QSettings(pom, QSettings::IniFormat);
}

QString GlobalSettings::getLanguage(){
    QString pom = settings->value("language").toString();
    if (pom == ""){
        pom = "EN";
        setLanguage("EN");
    }
    return pom;
}

void GlobalSettings::setLanguage(QString i_language){
    settings->setValue("language",i_language);
    settings->sync();
}

QString GlobalSettings::getHDDCounterName(){    
    string filename = QString("/HDDparameters.json").toLocal8Bit().constData();
    string fullPath = iniPath.toLocal8Bit().constData()+filename;
    QFile file(QString::fromStdString(fullPath));
    if (file.exists())
        return readAndConvert(fullPath).at(0);
    else
        return "";
}

QString GlobalSettings::getHDDCounterParameter(){
    string filename = QString("/HDDparameters.json").toLocal8Bit().constData();
    string fullPath = iniPath.toLocal8Bit().constData()+filename;
    QFile file(QString::fromStdString(fullPath));
    if (file.exists())
        return readAndConvert(fullPath).at(1);
    else
        return "";
}

void GlobalSettings::setHDDCounter(QString i_name, QString i_parameter){
    string filename = QString("/HDDparameters.json").toLocal8Bit().constData();
    string fullPath = iniPath.toLocal8Bit().constData()+filename;
    string _name = i_name.toUtf8().constData();
    string _parameter = i_parameter.toUtf8().constData();
    ofstream myfile;
    myfile.open (fullPath,ios::out);
    myfile << "{\"name\":["+_name+"],\"parameter\":["+_parameter+"]}";
    myfile.close();
}

QString GlobalSettings::getAppPath(){
    QString pom = settings->value("applicationDirectory").toString();
    return pom;
}

void GlobalSettings::setAppPath(QString i_path){
    settings->setValue("applicationDirectory",i_path);
    settings->sync();
}

bool GlobalSettings::getAutoUpdateSetting(){
    bool status = settings->value("updatesAutocheck").toBool();
    return status;
}

void GlobalSettings::setAutoUpdateSetting(bool i_status){
    settings->setValue("updatesAutocheck",i_status);
    settings->sync();
}
