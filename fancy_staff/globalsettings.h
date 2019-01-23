#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QSettings>
#include <QObject>

class GlobalSettings
{
public:
    Q_DISABLE_COPY(GlobalSettings)

    static GlobalSettings *getSettings();
    void setFileFolderDirectoriesPath(const QString& input);
    QString getFileFolderDirectoriesPath() const;
    QString getIniPath() const;
private:
    GlobalSettings();
    static GlobalSettings * m_Settings;
    QSettings * settings;
    QString iniPath;
};

#endif // GLOBALSETTINGS_H
