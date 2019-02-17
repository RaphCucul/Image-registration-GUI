#include "hlavni_program/t_b_ho.h"
#include "ui_t_b_ho.h"
#include "util/souborove_operace.h"
#include "dialogy/errordialog.h"
#include "mainwindow.h"
#include "fancy_staff/globalsettings.h"
#include "fancy_staff/sharedvariables.h"

#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QComboBox>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

t_b_HO::t_b_HO(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::t_b_HO)
{
    ui->setupUi(this);
    ui->pathToVideos->setText(tr("Load video from"));
    ui->SaveVideos->setText(tr("Save video to"));
    ui->LoadingDataFolder->setText(tr("Load video parameters"));
    ui->SavingDataFolder->setText(tr("Save video parameters"));
    ui->paramFrangPB->setText(tr("Load frangi parameters"));
    //ui->SaveAllPath->setText(tr("Save my paths"));
    //ui->SaveAllPath->setEnabled(false);
    ui->ChooseFileFolderDirectory->setText(tr("Choose file folder directory"));
    ui->FileFolderDirectory->setPlaceholderText(tr("Folder with paths"));

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    localErrorDialogHandling[ui->FileFolderDirectory] = new ErrorDialog(ui->FileFolderDirectory);
    localErrorDialogHandling[ui->Combo_videoPath] = new ErrorDialog(ui->Combo_videoPath);
    localErrorDialogHandling[ui->Combo_savingVideo] = new ErrorDialog(ui->Combo_savingVideo);
    localErrorDialogHandling[ui->Combo_VideoDataLoad] = new ErrorDialog(ui->Combo_VideoDataLoad);
    localErrorDialogHandling[ui->Combo_VideoDataSave] = new ErrorDialog(ui->Combo_VideoDataSave);
    localErrorDialogHandling[ui->Combo_FrangiParams] = new ErrorDialog(ui->Combo_FrangiParams);
}

t_b_HO::~t_b_HO()
{
    delete ui;
}
QString t_b_HO::LoadSettings(){
    QString iniFileProcessingResult = "";
    QString pom = GlobalSettings::getSettings()->getFileFolderDirectoriesPath();
    /// if pom is empty, *.ini file does not have required content
    if (pom == "")
        iniFileProcessingResult = "MissingPath";
    else{
        pathToFileFolderDirectory = pom;
        if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        iniFileProcessingResult = "OK";
    }
    qDebug()<<pathToFileFolderDirectory;
    return iniFileProcessingResult;
}

bool t_b_HO::checkFileFolderExistence()
{
    QString checkIniPath = GlobalSettings::getSettings()->getIniPath();
    int numberOfIni=-1;
    QStringList foundSettingsFiles;
    analyseFileNames(checkIniPath,foundSettingsFiles,numberOfIni,"ini");
    bool fileExistence = false;

    if (numberOfIni == 1){
        QString loadingResult = LoadSettings();
        if (loadingResult == "MissingPath"){
            disableElements();
            localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","softError",1);
            localErrorDialogHandling[ui->FileFolderDirectory]->show();
        }
        else{
            int numberOfJson = -1;
            QStringList foundFiles;
            analyseFileNames(pathToFileFolderDirectory,foundFiles,numberOfJson,"json");
            if (numberOfJson == 1){
                fileFolderDirectoryName = foundFiles.at(0);
                ui->FileFolderDirectory->setText(pathToFileFolderDirectory+"/"+fileFolderDirectoryName);
                if (loadJsonPaths()){
                    fileExistence = true;
                    //ui->SaveAllPath->setEnabled(true);
                    emit fileFolderDirectoryFound();
                    if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
                        localErrorDialogHandling[ui->FileFolderDirectory]->hide();
                }
                /*else{
                    localErrorDialogHandling[ui->SaveAllPath]->evaluate("left","hardError",2);
                    localErrorDialogHandling[ui->SaveAllPath]->show();
                }*/
            }
            else{
                enableElements();
                localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","info",2);
                localErrorDialogHandling[ui->FileFolderDirectory]->show();
            }
        }
    }
    else{
        disableElements();
        localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","hardError",0);
        localErrorDialogHandling[ui->FileFolderDirectory]->show();
    }
    return fileExistence;
}

bool t_b_HO::loadJsonPaths()
{
    QFile file;
    file.setFileName(pathToFileFolderDirectory+"/seznamCest.json");
    fileWithPaths = readJson(file);
    for (int typeIndex = 0; typeIndex < pathTypes.count(); typeIndex++)
        typeArrays[pathTypes.at(typeIndex)] = fileWithPaths[pathTypes.at(typeIndex)].toArray();
    qDebug()<<"Loading paths.";
    return getPathsFromJson(false);
}

bool t_b_HO::getPathFromJson(QString type, QComboBox *box, bool onlyCheckEmpty)
{
    bool pathSet = false; /// false returned if empty string would be added
    int arraySize = typeArrays[type].size();
    if (arraySize == 1)
    {
        if (!onlyCheckEmpty){
            box->addItem(typeArrays[type][0].toString());
            typeLists[type].append(typeArrays[type][0].toString());
            pathSet = true;
        }
        else
            pathSet = true;
    }
    if (arraySize > 1)
    {
        if (!onlyCheckEmpty){
            for (int a = 0; a < arraySize; a++)
            {
                QString pom = typeArrays[type][a].toString();
                box->addItem(pom);
                typeLists[type].append(pom);
            }
            pathSet = true;
        }
        else
            pathSet = true;
    }
    return pathSet;
}

bool t_b_HO::getPathsFromJson(bool onlyCheckEmpty)
{
    bool allCategoriesSet = false;
    bool VFA = getPathFromJson(pathTypes.at(0),ui->Combo_videoPath,onlyCheckEmpty);
    bool SV = getPathFromJson(pathTypes.at(1),ui->Combo_savingVideo,onlyCheckEmpty);
    bool VDL = getPathFromJson(pathTypes.at(2),ui->Combo_VideoDataLoad,onlyCheckEmpty);
    bool VDS = getPathFromJson(pathTypes.at(3),ui->Combo_VideoDataSave,onlyCheckEmpty);
    bool FP = getPathFromJson(pathTypes.at(4),ui->Combo_FrangiParams,onlyCheckEmpty);

    if (VFA && SV && VDL && VDS && FP){
        qDebug()<<"Loading successfull. Filling comboboxes.";
        for (int typeIndex = 0; typeIndex < pathTypes.count(); typeIndex++)
            SharedVariables::getSharedVariables()->setPath(pathTypes.at(typeIndex),typeLists[pathTypes.at(typeIndex)].at(0));

        SharedVariables::getSharedVariables()->processFrangiParameters(typeLists["parametryFrangiFiltr"].at(0));
        connect(ui->Combo_videoPath,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        //connect(ui->CB_cesta_k_videim,&QComboBox::currentTextChanged,this,&t_b_HO::chosenPathString);
        connect(ui->Combo_savingVideo,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        //connect(ui->CB_ulozeni_videa,&QComboBox::currentTextChanged,this,&t_b_HO::chosenPathString);
        connect(ui->Combo_VideoDataLoad,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        //connect(ui->CB_slozka_txt,&QComboBox::currentTextChanged,this,&t_b_HO::chosenPathString);
        connect(ui->Combo_VideoDataSave,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        //connect(ui->CB_ulozeni_txt,&QComboBox::currentTextChanged,this,&t_b_HO::chosenPathString);
        connect(ui->Combo_FrangiParams,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        allCategoriesSet = true;
    }
    if (!VFA){
        localErrorDialogHandling[ui->Combo_videoPath]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_videoPath]->show();
    }
    if (!SV){
        localErrorDialogHandling[ui->Combo_savingVideo]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_savingVideo]->show();
    }
    if (!VDL)
    {
        localErrorDialogHandling[ui->Combo_VideoDataLoad]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_VideoDataLoad]->show();
    }
    if (!VDS){
        localErrorDialogHandling[ui->Combo_VideoDataSave]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_VideoDataSave]->show();
    }
    if (!FP){
        localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_FrangiParams]->show();
    }
    return allCategoriesSet;
}

void t_b_HO::chosenPath(int index)
 {
    qDebug()<<index;
     if (QObject::sender() == ui->Combo_videoPath)
         SharedVariables::getSharedVariables()->setPath("cestaKvideim",typeLists["cestaKvideim"].at(index));
     if (QObject::sender() == ui->Combo_savingVideo)
         SharedVariables::getSharedVariables()->setPath("ulozeniVidea",typeLists["ulozeniVidea"].at(index));
     if (QObject::sender() == ui->Combo_VideoDataLoad)
         SharedVariables::getSharedVariables()->setPath("adresarTXT_nacteni",typeLists["adresarTXT_nacteni"].at(index));
     if (QObject::sender() == ui->Combo_FrangiParams)
         SharedVariables::getSharedVariables()->setPath("adresarTXT_ulozeni",typeLists["adresarTXT_ulozeni"].at(index));
     if (QObject::sender() == ui->Combo_FrangiParams){
         SharedVariables::getSharedVariables()->setPath("parametryFrangiFiltr",typeLists["parametryFrangiFiltr"].at(index));
         SharedVariables::getSharedVariables()->processFrangiParameters(typeLists["parametryFrangiFiltr"].at(index));
     }
     qDebug()<<index<<typeLists["cestaKvideim"].at(index);
 }

/*void t_b_HO::chosenPathString(QString cesta)
{
    if (QObject::sender() == ui->Combo_videoPath)
        videaKanalyzeAktual = cesta;
    if (QObject::sender() == ui->Combo_savingVideo)
        ulozeniVideiAktual = cesta;
    if (QObject::sender() == ui->Combo_VideoDataLoad)
        TXTnacteniAktual = cesta;
    if (QObject::sender() == ui->Combo_VideoDataSave)
        TXTulozeniAktual = cesta;
    if (QObject::sender() == ui->Combo_FrangiParams)
        paramFrangi = cesta;

    qDebug()<<cesta;
}*/

void t_b_HO::on_ChooseFileFolderDirectory_clicked()
{
    QString fileFolderDirectoryPath_new = QFileDialog::getExistingDirectory(this,"",QDir::currentPath());
    pathToFileFolderDirectory = fileFolderDirectoryPath_new;

    if (fileFolderDirectoryPath_new != ""){
        if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        GlobalSettings::getSettings()->setFileFolderDirectoriesPath(fileFolderDirectoryPath_new);
        //ui->SaveAllPath->setEnabled(true);
        ui->pathToVideos->setEnabled(true);
        ui->SaveVideos->setEnabled(true);
        ui->LoadingDataFolder->setEnabled(true);
        ui->SavingDataFolder->setEnabled(true);
        ui->paramFrangPB->setEnabled(true);
        int numberOfJson = -1;
        QStringList foundFiles;
        analyseFileNames(pathToFileFolderDirectory,foundFiles,numberOfJson,"json");
        if (numberOfJson == 1){
            fileFolderDirectoryName = foundFiles.at(0);
            ui->FileFolderDirectory->setText(pathToFileFolderDirectory+"/"+fileFolderDirectoryName);
            loadJsonPaths();
            emit fileFolderDirectoryFound();
        }
    }
}

void t_b_HO::processPath(QString type, QString path){
    QJsonValue pathJson = QJsonValue(path);
    typeArrays[type].append(pathJson);
    typeLists[type].append(path);
    QString t = pathToFileFolderDirectory+"/seznamCest.json";
    writeJson(fileWithPaths,typeArrays[type],type,t);
    SharedVariables::getSharedVariables()->setPath(type,path);
}

void t_b_HO::on_pathToVideos_clicked()
{
    QString pathToVideos = QFileDialog::getExistingDirectory(this,"",QDir::currentPath());
    if (pathToVideos != ""){
        processPath("cestaKvideim",pathToVideos);
        ui->Combo_videoPath->addItem(pathToVideos);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["cestaKvideim"];
    }
}

void t_b_HO::on_SaveVideos_clicked()
{
    QString pathToVideoSave = QFileDialog::getExistingDirectory(this,"Vyberte složku pro uložení slícovaného videa",QDir::currentPath());
    if (pathToVideoSave != ""){
        processPath("ulozeniVidea",pathToVideoSave);
        ui->Combo_savingVideo->addItem(pathToVideoSave);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["ulozeniVidea"];
    }
}

void t_b_HO::on_LoadingDataFolder_clicked()
{
    QString pathToVideoParameters = QFileDialog::getExistingDirectory(this,"Vyberte složku obsahující parametrický soubory",QDir::currentPath());
    if (pathToVideoParameters != ""){
        processPath("adresarTXT_nacteni",pathToVideoParameters);
        ui->Combo_VideoDataLoad->addItem(pathToVideoParameters);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["adresarTXT_nacteni"];
    }
}

void t_b_HO::on_SavingDataFolder_clicked()
{
    QString pathToSaveVideoParameters = QFileDialog::getExistingDirectory(this,"Vyberte složku pro uložení parametrických souborů",QDir::currentPath());
    if (pathToSaveVideoParameters != ""){
        processPath("adresarTXT_ulozeni",pathToSaveVideoParameters);
        ui->Combo_VideoDataSave->addItem(pathToSaveVideoParameters);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["adresarTXT_ulozeni"];
    }
}

void t_b_HO::on_paramFrangPB_clicked()
{
    QString pathFF = QFileDialog::getExistingDirectory(this,"Vyberte složku obsahující parametry Frangiho filtru",QDir::currentPath());
    QString completePath = pathFF+"/frangiParameters.json";
    QFileInfo check_file(completePath);
    if (check_file.exists() && check_file.isFile()){
        processPath("parametryFrangiFiltr",pathFF);
        ui->Combo_FrangiParams->addItem(pathFF);
        SharedVariables::getSharedVariables()->processFrangiParameters(pathFF);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
    }
    else{
        localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","hardError",7);
        localErrorDialogHandling[ui->Combo_FrangiParams]->show();
    }
}

/*void t_b_HO::on_SaveAllPath_clicked()
{
    if (videosForAnalysisList.isEmpty() || savingVideosList.isEmpty() || videoDataLoadList.isEmpty()
            || videoDataSaveList.isEmpty() || frangiParametersList.isEmpty()){
        localErrorDialogHandling[ui->SaveAllPath]->evaluate("left","info",2);
        localErrorDialogHandling[ui->SaveAllPath]->show();
    }
    else{
        if (localErrorDialogHandling[ui->SaveAllPath]->isEvaluated())
            localErrorDialogHandling[ui->SaveAllPath]->hide();
        for (int typeIndex = 0; typeIndex < typeArrays.count(); typeIndex++)
            fileWithPaths[pathTypes.at(typeIndex)] = typeArrays[pathTypes.at(typeIndex)];

        QJsonDocument document;
        document.setObject(fileWithPaths);
        QString documentString = document.toJson();
        QFile zapis;
        zapis.setFileName(pathToFileFolderDirectory+"/seznamCest.json");
        zapis.open(QIODevice::WriteOnly);
        zapis.write(documentString.toLocal8Bit());
        zapis.close();
        emit fileFolderDirectoryFound();
    }

}*/

void t_b_HO::on_FileFolderDirectory_textEdited(const QString &arg1)
{

}

void t_b_HO::enableElements(){
    //ui->SaveAllPath->setEnabled(true);
    ui->pathToVideos->setEnabled(true);
    ui->SaveVideos->setEnabled(true);
    ui->LoadingDataFolder->setEnabled(true);
    ui->SavingDataFolder->setEnabled(true);
    ui->paramFrangPB->setEnabled(true);
}

void t_b_HO::disableElements(){
    //ui->SaveAllPath->setEnabled(false);
    ui->pathToVideos->setEnabled(false);
    ui->SaveVideos->setEnabled(false);
    ui->LoadingDataFolder->setEnabled(false);
    ui->SavingDataFolder->setEnabled(false);
    ui->paramFrangPB->setEnabled(false);
}
