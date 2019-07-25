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

    /**
     * @brief Function returns the settings data.
     * @return
     */
    QString getFileFolderDirectoriesPath() const;

    /**
     * @brief Function return the location of *.ini file
     * @return
     */
    QString getIniPath() const;

    /**
     * @brief If the program could not find the *.ini file, user can add find it manually and save it to the program.
     * @param input
     */
    void setIniPath(const QString &inputPath, const QString &inputFile);

    /**
     * @brief Function returns saved language from ini file.
     * @return
     */
    QString getLanguage();

    /**
     * @brief Function saves language, if user changes it.
     * @param i_language
     */
    void setLanguage(QString i_language);

    /**
     * @brief Function returns the name of HDD counter localized to system language
     * @return
     */
    QString getHDDCounterName();

    /**
     * @brief Function returns the name of HDD counter parameter localized to system language
     * @return
     */
    QString getHDDCounterParameter();

    /**
     * @brief Function saves the counter name and counter parameter name into tje HDDparameters.json file.
     * @param i_name
     * @param i_parameter
     */
    void setHDDCounter(QString i_name,QString i_parameter);

    /**
     * @brief Function returns application path, where almost all files
     * necessary for the program should be located.
     * @return
     */
    QString getAppPath();

    /**
     * @brief Function saves application path, where almost all files
     * necessary for the program should be located.
     * @param i_path
     */
    void setAppPath(QString i_path);

    /**
     * @brief Function returns user's choice of auto update check.
     * @return
     */
    bool getAutoUpdateSetting();

    /**
     * @brief Function saves user's choice of auto update check.
     */
    void setAutoUpdateSetting(bool i_status);
private:
    GlobalSettings();
    static GlobalSettings * m_Settings;
    QSettings * settings;
    QString iniPath;
    QString iniFileName = "settings.ini";
};

#endif // GLOBALSETTINGS_H
