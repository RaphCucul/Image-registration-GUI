// Part of Frames registration project
// created by Jan Prosser
// Licensed under BSD-2-Clause License

#ifndef DIRECTORIESLOADER_H
#define DIRECTORIESLOADER_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QStringList>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QEvent>

#include "mainwindow.h"
#include "dialogs/errordialog.h"
namespace Ui {
class DirectoriesLoader;
}

/**
 * @class DirectoriesLoader
 * @brief Provides functions allowing a user to load *.ini settings file, frangi parameters JSON file
 * with global frangi parameters and the JSON file with directories paths.
 *
 * It also automatically saves an added directory to the JSON.
 */
class DirectoriesLoader : public QWidget
{
    Q_OBJECT

public:
    explicit DirectoriesLoader(QWidget *parent = nullptr);

    /**
     * @brief Controls if the *.ini file contains a path to the JSON file where paths for loading from, saving videos, video
     * parameters and Frangi filter parameters are stored.
     *
     * If the JSON file exists, stored directories are loaded to combo boxes,
     * those with zero indexes are marked as actually used directories. If the JSON file isn't found, all tabs are disabled.
     * @return Returns true or false based on the check existence result
     */
    bool checkFileFolderExistence();

    /**
     * @brief Loads a path linked to a JSON file with directories stored in the *.ini file, processed by GlobalSettings class.
     * @sa processLoadedSettings
     * @return The result of the loading process.
     */
    bool LoadSettings();

    ~DirectoriesLoader();
private slots:
    /**
     * @brief Stores a directory path a user has selected.
     * @param[in] index - index of the selected path in the combo box list
     */
    void chosenPath(int i_index);
    /**
     * @brief Enables a user to add a new directory with Frangi parameters description file.
     */
    void on_paramFrangPB_clicked();
    /**
     * @brief Enables a user to update a directory with a JSON file with the list of directories.
     */
    void on_ChooseFileFolderDirectory_clicked();
    /**
     * @brief Enables a user to add a new directory with original videos.
     */
    void on_pathToVideos_clicked();
    /**
     * @brief Enables a user to to add a new directory where registrated videos should be saved.
     */
    void on_SaveVideos_clicked();
    /**
     * @brief Enables a user to to add a new directory with created video *.dat files.
     */
    void on_DataFolder_clicked();

signals:
    /**
     * @brief Emitted to unblock all tabs of the program. If the paths were loaded successfully - the program is
     * usable.
     */
    void fileFolderDirectoryFound();

    /**
     * @brief Emitted to block all tabs until a path to a directory with JSON file is found.
     */
    void fileFolderDirectoryNotFound();
private:
    Ui::DirectoriesLoader *ui;

    /**
     * @brief Processes a path chosen by a user. It is added to the JSON array and string list.
     * @param[in] type - corresponding path type (category)
     * @param[in] path - chosen path itself
     */
    void processPath(QString i_type, QString i_path);

    /**
     * @brief Makes all widgets enabled. Directories can be added and selected.
     */
    void enableElements();

    /**
     * @brief Disables all widgets. No operation can be done.
     */
    void disableElements();

    /**
     * @brief Checks existing loaded directories. It may check if a directory exists or it can added it
     * to the internal variable.
     * @param[in] type - corresponding path category (type)
     * @param[in] box - activated comboBox
     * @param[in] onlyCheckEmpty - if true, the path is not added to the internal variable
     * @return Returns true or false based on the processing result
     */
    bool getPathFromJson(QString i_type, QComboBox* i_box, bool i_onlyCheckEmpty);

    /**
     * @brief Calls getPathFromJson for all directory types and sets the argument onlyCheckEmpty
     * to these functions.
     * @sa getPathFromJson(QString i_type, QComboBox* i_box, bool i_onlyCheckEmpty)
     * @param[in] onlyCheckEmpty - if true, the path is not added to the internal variable
     * @return Returns true or false based on the processing result
     */
    bool getPathsFromJson(bool i_onlyCheckEmpty);

    /**
     * @brief Loads the content from a JSON file with directories.
     * @sa getPathsFromJson(bool i_onlyCheckEmpty)
     * @return Returns true or false based on the processing result
     */
    bool loadJsonPaths();

    /**
     * @brief Wrapper function for a JSON file data loading.
     * @sa loadJsonPaths();
     * @return Returns true or false based on the processing result
     */
    bool processLoadedSettings();

    /**
     * @brief Creates a new *.ini settings file, if the file is found missing when the program
     * starts. Then, the path to JSON file is added to this new *.ini file.
     */
    void createIni();

    /**
     * @brief Function implements the posibility to delete the highlighted item from a combobox list.
     * @param[in] obj - affected object (widget)
     * @param[in] event - type of event (mouse pressed, released etc.)
     * @return Returns true or false based on the processing result
     */
    bool eventFilter(QObject *obj, QEvent *event);

    QJsonObject fileWithPaths; /**<JsonObject storing loaded information from the JSON file with directories. */
    QStringList pathTypes = {"videosPath","saveVideosPath","datFilesPath","parametersFrangiFiltr"};
    QHash<QString,QJsonArray> typeArrays;
    QHash<QString,QStringList> typeLists;
    QString pathToFileFolderDirectory,fileFolderDirectoryName;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QString search = "";
    QString _tempSettignsFile = "";
};

#endif // DIRECTORIESLOADER_H
