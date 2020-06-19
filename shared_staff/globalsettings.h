#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QSettings>
#include <QObject>

class GlobalSettings
{
public:
    Q_DISABLE_COPY(GlobalSettings)

    /**
     * @brief Enables to get the GlobalSettings class object.
     * @return GlobalSettings class singleton
     */
    static GlobalSettings *getSettings();

    /**
     * @brief Saves a given path to the JSON file with the list of directories.
     * @sa getFileFolderDirectoriesPath()
     * @param[in] input
     */
    void setFileFolderDirectoriesPath(const QString& input);

    /**
     * @brief Returns the path to the JSON file with the list of directories.
     * @sa setFileFolderDirectoriesPath(const QString& input)
     */
    QString getFileFolderDirectoriesPath() const;

    /**
     * @brief Returns the location of the *.ini file.
     * @sa setIniPath(const QString &inputPath, const QString &inputFile)
     */
    QString getIniPath() const;

    /**
     * @brief Saves a directory path. If the program could not find the *.ini file,
     * user can add find it manually and save it to the program.
     * @sa getIniPath()
     * @param[in] inputPath
     * @param[in] inputFile
     */
    void setIniPath(const QString &inputPath, const QString &inputFile);

    /**
     * @brief Returns saved app language from the *.ini file.
     * @sa setLanguage(QString i_language)
     */
    QString getLanguage();

    /**
     * @brief Saves the app language chosen by a user.
     * @param[in] i_language
     * @sa getLanguage()
     */
    void setLanguage(QString i_language);

    /**
     * @brief Returns the name of the HDD counter localized to the system language.
     * @sa getHDDCounterParameter()
     */
    QString getHDDCounterName();

    /**
     * @brief Returns the name of the HDD counter parameter localized to the system language.
     * @sa getHDDCounterName()
     */
    QString getHDDCounterParameter();

    /**
     * @brief Saves the counter name and counter parameter name into the HDDparameters.json file.
     * @param[in] i_name - HDD counter name
     * @param[in] i_parameter - HDD counter parameter
     * @sa getHDDCounterParameter()
     * @sa getHDDCounterName()
     */
    void setHDDCounter(QString i_name,QString i_parameter);

    /**
     * @brief Returns application path where all files
     * necessary for the program should be located.
     * @sa setAppPath(QString i_path)
     */
    QString getAppPath();

    /**
     * @brief Saves application path where all files
     * necessary for the program should be located.
     * @param[in] i_path
     * @sa getAppPath()
     */
    void setAppPath(QString i_path);

    /**
     * @brief Returns user's choice of the auto update check.
     * @sa setAutoUpdateSetting(bool i_status)
     */
    bool getAutoUpdateSetting();

    /**
     * @brief Saves user's choice of auto update check.
     * @sa getAutoUpdateSetting()
     */
    void setAutoUpdateSetting(bool i_status);
private:
    GlobalSettings();
    static GlobalSettings * m_Settings;
    QSettings * settings;
    QString iniPath;
    QString iniFileName = "settings.ini"; /**<The name of the application file with settings is strict and should not be changed */
};

#endif // GLOBALSETTINGS_H
