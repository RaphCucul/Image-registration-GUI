#include "globalsettings.h"
#include <QCoreApplication>
#include <QDebug>

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
    qDebug()<<settings->value("fileFolderDirectoryPath").toString();
    QString pom = settings->value("fileFolderDirectoryPath").toString();
    return pom;
}

void GlobalSettings::setFileFolderDirectoriesPath(const QString &input)
{
    settings->setValue("fileFolderDirectoryPath", input);
    qDebug()<<settings->value("fileFolderDirectoryPath").toString();
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
