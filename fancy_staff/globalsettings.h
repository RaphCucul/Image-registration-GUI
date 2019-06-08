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

    QString getHDDCounterName();
    QString getHDDCounterParameter();
    void setHDDCounter(QString i_name,QString i_parameter);

    QString getAppPath();
    void setAppPath(QString i_path);
private:
    GlobalSettings();
    static GlobalSettings * m_Settings;
    QSettings * settings;
    QString iniPath;
    QString iniFileName = "settings.ini";
};

#endif // GLOBALSETTINGS_H
