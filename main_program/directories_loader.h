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

class DirectoriesLoader : public QWidget
{
    Q_OBJECT

public:
    explicit DirectoriesLoader(QWidget *parent = nullptr);

    /**
     * @brief The function controls if the *.ini file contains path to the JSON file (fileFolderDirectory)
     * where paths for loading, saving videos, video parameters and Frangi filter parameters are stored. If the
     * JSON file exists, stored paths are added to comboboxes and paths with zero index are marked as actualy
     * used directories. If the JSON is not found, all tabs are disabled.
     * @return Returns true or false based on the check existence result
     */
    bool checkFileFolderExistence();

    /**
     * @brief The function loads the JSON path stored in the *.ini file, processed by GlobalSettings class
     * @sa processLoadedSettings();
     * @return path to JSON
     */
    QString LoadSettings();

    ~DirectoriesLoader();
private slots:
    /**
     * @brief Slot function which stores a directory path user has selected
     * @param[in] index
     */
    void chosenPath(int i_index);
    /**
     * @brief Slot function which enables a user to add a new directory path with Frangi parameters description file
     */
    void on_paramFrangPB_clicked();
    /**
     * @brief Slot function which enables a user to update a directory path of a JSON file with the list of directories
     */
    void on_ChooseFileFolderDirectory_clicked();
    /**
     * @brief Slot function which enables a user to add a new directory path with original videos
     */
    void on_pathToVideos_clicked();
    /**
     * @brief Slot function which enables a user to to add a new directory path where registrated videos should be saved.
     */
    void on_SaveVideos_clicked();
    /**
     * @brief Slot function which enables a user to to add a new directory path with created *.dat files.
     */
    void on_LoadingDataFolder_clicked();
    /**
     * @brief Slot function which enables a user to to add a new directory path where *.dat files should be saved
     */
    void on_SavingDataFolder_clicked();

    //void on_FileFolderDirectory_textEdited(const QString &arg1);
signals:
    /**
     * @brief Signal emitted to unblock all tabs of the program, if the paths were loaded successfully - program is usable
     */
    void fileFolderDirectoryFound();

    /**
     * @brief Signal emitted to block the GUI until the proper path to the directory with json file is found.
     */
    void fileFolderDirectoryNotFound();
private:
    Ui::DirectoriesLoader *ui;

    /**
     * @brief The function processes a path chosen by a user. It is added to the JSON array and string list.
     * @param[in] type - corresponding path type
     * @param[in] path - chosen path itself
     */
    void processPath(QString i_type, QString i_path);

    /**
     * @brief The function makes all form elements enabled. Paths can be added and selected.
     */
    void enableElements();

    /**
     * @brief The function makes all form elements. No operations are allowed.
     */
    void disableElements();

    /**
     * @brief The function reads the content of an object specified by a type from the JSON file.
     * Combobox is then filled with the loaded content. onlyCheckEmpty parameter says if the JSON object
     * should be analysed and the content loaded or if the user wants to check the JSON object is empty or not.
     * @param[in] type - corresponding path type
     * @param[in] box - activated comboBox
     * @param[in] onlyCheckEmpty - if true, the path is not added to the comboBox list
     * @return Returns true or false based on the processing result
     */
    bool getPathFromJson(QString i_type, QComboBox* i_box, bool i_onlyCheckEmpty);

    /**
     * @brief The function calls getPathFromJson for all JSON object types and sets the argument onlyCheckEmpty
     * to these functions. onlyCheckEmpty parameter says if the JSON object
     * should be analysed and the content loaded or if the user wants to check the JSON object is empty or not.
     * @sa getPathFromJson(QString i_type, QComboBox* i_box, bool i_onlyCheckEmpty)
     * @param[in] onlyCheckEmpty
     * @return
     */
    bool getPathsFromJson(bool i_onlyCheckEmpty);

    /**
     * @brief The function loads the content from the JSON folder file.
     * @sa getPathsFromJson(bool i_onlyCheckEmpty)
     * @return
     */
    bool loadJsonPaths();

    /**
     * @brief Helper function for LoadSettings function. Loads data from JSON file.
     * @sa LoadSettings()
     * @return
     */
    bool processLoadedSettings();

    /**
     * @brief The function creates a new *.ini settings file, if the file is missing, when the program
     * starts. Then, the path to JSON file is added to this new *.ini file.
     */
    void createIni();

    /**
     * @brief Function implements the posibility to delete the highlighted item from a combobox list.
     * @param obj
     * @param event
     * @return
     */
    bool eventFilter(QObject *obj, QEvent *event);

    QJsonObject fileWithPaths; /**<JsonObject storing loaded information from the JSON file with directories. */
    QStringList pathTypes = {"videosPath","saveVideosPath","loadDatFilesPath","saveDatFilesPath","parametersFrangiFiltr"};
    QHash<QString,QJsonArray> typeArrays;
    QHash<QString,QStringList> typeLists;
    QString pathToFileFolderDirectory,fileFolderDirectoryName;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QString search = "";
    QString _tempSettignsFile = "";
};

#endif // DIRECTORIESLOADER_H
