#ifndef T_B_HO_H
#define T_B_HO_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include <QStringList>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

#include "mainwindow.h"
#include "dialogy/errordialog.h"
namespace Ui {
class t_b_HO;
}

extern QString videaKanalyzeAktual;
extern QString ulozeniVideiAktual;
extern QString TXTnacteniAktual;
extern QString TXTulozeniAktual;
extern QString paramFrangi;

class t_b_HO : public QWidget
{
    Q_OBJECT

public:
    explicit t_b_HO(QWidget *parent = nullptr);

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

    ~t_b_HO();
private slots:
    /**
     * @brief Function stores the path user has selected in the corresponding combobox
     * @param index
     */
    void chosenPath(int index);
    //void chosenPathString(QString cesta);

    void on_paramFrangPB_clicked();
    void on_ChooseFileFolderDirectory_clicked();
    void on_pathToVideos_clicked();
    void on_SaveVideos_clicked();
    void on_LoadingDataFolder_clicked();
    void on_SavingDataFolder_clicked();
    //void on_SaveAllPath_clicked();

    void on_FileFolderDirectory_textEdited(const QString &arg1);
signals:
    /**
     * @brief Unblocks all tabs of the program, if the paths were loaded successfully
     */
    void fileFolderDirectoryFound();
private:
    Ui::t_b_HO *ui;

    /**
     * @brief Function process path chosen by user by adding it into JSON array and string list
     * @param type - corresponding path type
     * @param path - chosen path itself
     */
    void processPath(QString type, QString path);

    /// two functions enable or disable all ui elements
    void enableElements();
    void disableElements();

    /// processing of JSON values
    bool getPathFromJson(QString type, QComboBox* box, bool onlyCheckEmpty);
    bool getPathsFromJson(bool onlyCheckEmpty);
    bool loadJsonPaths();

    QJsonObject fileWithPaths;
    QJsonArray videosForAnalysis,savingVideos,videoDataLoad,videoDataSave,frangiParameters;
    QStringList videosForAnalysisList,savingVideosList,videoDataLoadList,videoDataSaveList,frangiParametersList;
    QStringList pathTypes = {"cestaKvideim","ulozeniVidea","adresarTXT_nacteni","adresarTXT_ulozeni","parametryFrangiFiltr"};
    QHash<QString,QJsonArray> typeArrays;
    QHash<QString,QStringList> typeLists;
    QString pathToFileFolderDirectory,fileFolderDirectoryName;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // T_B_HO_H
