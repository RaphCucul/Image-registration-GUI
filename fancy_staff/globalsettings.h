#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QSettings>
#include <QObject>

class GlobalSettings
{
public:
    Q_DISABLE_COPY(GlobalSettings)

    static GlobalSettings *getSettings();

    /**
     * @brief setFileFolderDirectoriesPath and getFileFolderDirectoriesPath are used for *.json file
     * with stored paths loaded in t_b_HO class
     */
    void setFileFolderDirectoriesPath(const QString& input);
    QString getFileFolderDirectoriesPath() const;

    /**
     * @brief Function return the location of *.ini file
     * @return
     */
    QString getIniPath() const;
private:
    GlobalSettings();
    static GlobalSettings * m_Settings;
    QSettings * settings;
    QString iniPath;
};

#endif // GLOBALSETTINGS_H
