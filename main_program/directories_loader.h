#ifndef DIRECTORIESLOADER_H
#define DIRECTORIESLOADER_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include <QStringList>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

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
     * @brief Function controls, if the *.ini file contains path to the JSON file (fileFolderDirectory)
     * where paths for loading, saving videos, video parameters and Frangi filter parameters are stored. If the
     * JSON file exists, the stored paths are loaded in to comboboxes and paths with zero index are marked as actualy
     * used directories. If the JSON is not found, all tabs are disabled.
     * @return
     */
    bool checkFileFolderExistence();

    /**
     * @brief Function loads the JSON path stored in the *.ini file, processed by GlobalSettings class
     * @return path to JSON
     */
    QString LoadSettings();

    ~DirectoriesLoader();
private slots:
    /**
     * @brief Function stores the path user has selected in the corresponding combobox
     * @param index
     */
    void chosenPath(int i_index);

    void on_paramFrangPB_clicked();
    void on_ChooseFileFolderDirectory_clicked();
    void on_pathToVideos_clicked();
    void on_SaveVideos_clicked();
    void on_LoadingDataFolder_clicked();
    void on_SavingDataFolder_clicked();

    void on_FileFolderDirectory_textEdited(const QString &arg1);
signals:
    /**
     * @brief Unblocks all tabs of the program, if the paths were loaded successfully
     */
    void fileFolderDirectoryFound();

    /**
     * @brief Signal blocks the GUI until the proper path to the directory with json file is found.
     */
    void fileFolderDirectoryNotFound();
private:
    Ui::DirectoriesLoader *ui;

    /**
     * @brief Function process path chosen by user by adding it into JSON array and string list
     * @param type - corresponding path type
     * @param path - chosen path itself
     */
    void processPath(QString i_type, QString i_path);

    /**
     * @brief Function enables all form elements. Paths can be added and selected.
     */
    void enableElements();

    /**
     * @brief Function disables all form elements. No operations are allowed.
     */
    void disableElements();

    /**
     * @brief Function reads the content of the object specified by "type" from the JSON file.
     * Proper "box" is then filled with the loaded content. "onlyCheckEmpty" parameter says, if the JSON object
     * should be analysed and the content loaded or if the user wants to check only if the JSON object is empty.
     * @param type
     * @param box
     * @param onlyCheckEmpty
     * @return
     */
    bool getPathFromJson(QString i_type, QComboBox* i_box, bool i_onlyCheckEmpty);

    /**
     * @brief Function calls getPathFromJson for all JSON object types and set the argument "onlyCheckEmpty"
     * to these functions. "onlyCheckEmpty" parameter says, if the JSON object
     * should be analysed and the content loaded or if the user wants to check only if the JSON object is empty.
     * @param onlyCheckEmpty
     * @return
     */
    bool getPathsFromJson(bool i_onlyCheckEmpty);

    /**
     * @brief Function loads the content from the JSON folder file.
     * @return
     */
    bool loadJsonPaths();

    /**
     * @brief Helper function for LoadSettings function. Loads data from JSON file.
     * @return
     */
    bool processLoadedSettings();

    /**
     * @brief Function creates new ini file for settings, if the file is missing, when the program
     * starts. Then, path to JSON file is added to this new ini file.
     */
    void createIni();

    /**
     * @brief Function implements the posibility to delete the highlighted item from a combobox list.
     * @param obj
     * @param event
     * @return
     */
    bool eventFilter(QObject *obj, QEvent *event);

    QJsonObject fileWithPaths;
    QStringList pathTypes = {"videosPath","saveVideosPath","loadDatFilesPath","saveDatFilesPath","parametersFrangiFiltr"};
    QHash<QString,QJsonArray> typeArrays;
    QHash<QString,QStringList> typeLists;
    QString pathToFileFolderDirectory,fileFolderDirectoryName;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
    QString search = "";
    QString _tempSettignsFile = "";
};

#endif // DIRECTORIESLOADER_H
