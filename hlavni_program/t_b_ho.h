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
    QStringList seznam_cest_prazdny=(QStringList()<<"-");
    int pocet_mist_combobox = 3;
    explicit t_b_HO(QWidget *parent = nullptr);
    bool checkFileFolderExistence();
    QString LoadSettings();

    ~t_b_HO();
private slots:
    void vybranaCesta(int index);
    void vybranaCestaString(QString cesta);

    void on_paramFrangPB_clicked();
    void on_ChooseFileFolderDirectory_clicked();
    void on_pathToVideos_clicked();
    void on_SaveVideos_clicked();
    void on_LoadingDataFolder_clicked();
    void on_SavingDataFolder_clicked();
    void on_SaveAllPath_clicked();

    void on_FileFolderDirectory_textEdited(const QString &arg1);
signals:
    void fileFolderDirectoryFound();
private:
    Ui::t_b_HO *ui;
    void enableElements();
    void disableElements();
    void getPathFromJson(QJsonArray& poleCest,QComboBox* box, QStringList& list);
    void getPathsFromJson();
    void loadJsonPaths();

    QVector<int> fillingComboboxsVideoPaths;
    bool iniFileError = false;
    QJsonObject fileWithPaths;
    QJsonArray videosForAnalysis,savingVideos,videoDataLoad,videoDataSave,frangiParameters;
    QStringList videosForAnalysisList,savingVideosList,videoDataLoadList,videoDataSaveList,frangiParametersList;
    QString pathToFileFolderDirectory,fileFolderDirectoryName;
    QHash<QWidget*,ErrorDialog*> localErrorDialogHandling;
};

#endif // T_B_HO_H
