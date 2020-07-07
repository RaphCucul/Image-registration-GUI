#include "main_program/directories_loader.h"
#include "ui_directories_loader.h"
#include "util/files_folders_operations.h"
#include "dialogs/errordialog.h"
#include "mainwindow.h"
#include "shared_staff/globalsettings.h"
#include "shared_staff/sharedvariables.h"

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
#include <exception>
#include <QCoreApplication>

DirectoriesLoader::DirectoriesLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectoriesLoader)
{
    ui->setupUi(this);
    ui->pathToVideos->setText(tr("Load video from"));
    ui->SaveVideos->setText(tr("Save video to"));
    ui->LoadingDataFolder->setText(tr("Load video parameters"));
    ui->SavingDataFolder->setText(tr("Save video parameters"));
    ui->paramFrangPB->setText(tr("Load frangi parameters"));
    ui->ChooseFileFolderDirectory->setText(tr("Choose file folder directories"));
    ui->FileFolderDirectory->setPlaceholderText(tr("Folder with paths"));

    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    localErrorDialogHandling[ui->FileFolderDirectory] = new ErrorDialog(ui->FileFolderDirectory);
    localErrorDialogHandling[ui->Combo_videoPath] = new ErrorDialog(ui->Combo_videoPath);
    ui->Combo_videoPath->installEventFilter(this);
    ui->Combo_savingVideo->installEventFilter(this);
    ui->Combo_VideoDataSave->installEventFilter(this);
    ui->Combo_VideoDataLoad->installEventFilter(this);
    ui->Combo_FrangiParams->installEventFilter(this);
    localErrorDialogHandling[ui->Combo_savingVideo] = new ErrorDialog(ui->Combo_savingVideo);
    localErrorDialogHandling[ui->Combo_VideoDataLoad] = new ErrorDialog(ui->Combo_VideoDataLoad);
    localErrorDialogHandling[ui->Combo_VideoDataSave] = new ErrorDialog(ui->Combo_VideoDataSave);
    localErrorDialogHandling[ui->Combo_FrangiParams] = new ErrorDialog(ui->Combo_FrangiParams);
}

DirectoriesLoader::~DirectoriesLoader()
{
    delete ui;
}
bool DirectoriesLoader::LoadSettings(){
    bool iniFileProcessingResult = false;
    QString pom = GlobalSettings::getSettings()->getFileFolderDirectoriesPath();
    if (pom == ""){
        disableElements();
    }
    else{
        pathToFileFolderDirectory = pom;
        ui->FileFolderDirectory->setText(pathToFileFolderDirectory);
        if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        iniFileProcessingResult = true;
        enableElements();
    }
    qDebug()<<"Path to file folder directory: "<<pathToFileFolderDirectory;
    return iniFileProcessingResult;
}

bool DirectoriesLoader::processLoadedSettings(){
    if (pathToFileFolderDirectory == "")
        return false;
    else{
        if (loadJsonPaths()){
            emit fileFolderDirectoryFound();
            if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
                localErrorDialogHandling[ui->FileFolderDirectory]->hide();
            return true;
        }
        else{
            //disableElements();
            return false;
        }
    }
}

bool DirectoriesLoader::checkFileFolderExistence()
{
    QString checkIniPath = GlobalSettings::getSettings()->getIniPath();
    int numberOfIni=-1;
    QStringList foundSettingsFiles;
    analyseFileNames(checkIniPath,foundSettingsFiles,numberOfIni,"ini");
    bool fileExistence = false;

    if (numberOfIni == 1){
        ui->iniPath->setText(checkIniPath);
        if (! LoadSettings()){
            localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","softError",0);
            localErrorDialogHandling[ui->FileFolderDirectory]->show(false);
            search = "json";
            ui->ChooseFileFolderDirectory->setText(tr("Find json file"));
        }
        else{            
            return processLoadedSettings();
        }
    }
    else{
        disableElements();
        localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","hardError",0);
        localErrorDialogHandling[ui->FileFolderDirectory]->show(false);
        ui->ChooseFileFolderDirectory->setText(tr("Find ini file"));
        search = "ini";
    }
    return fileExistence;
}

bool DirectoriesLoader::loadJsonPaths()
{
    try {
        QFile file;
        file.setFileName(pathToFileFolderDirectory);
        if (file.exists()){
            fileWithPaths = readJson(file);
            for (int typeIndex = 0; typeIndex < pathTypes.count(); typeIndex++)
                typeArrays[pathTypes.at(typeIndex)] = fileWithPaths[pathTypes.at(typeIndex)].toArray();
            qDebug()<<"Loading paths.";
            return getPathsFromJson(false);
        }
    } catch (std::exception& e) {
        localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","hardError",e.what());
        localErrorDialogHandling[ui->FileFolderDirectory]->show(false);
        return false;
    }
}

bool DirectoriesLoader::getPathFromJson(QString i_type, QComboBox *i_box, bool i_onlyCheckEmpty)
{
    bool pathSet = false; /// false returned if empty string would be added
    int arraySize = typeArrays[i_type].size();
    if (arraySize == 1)
    {
        if (!i_onlyCheckEmpty){
            i_box->addItem(typeArrays[i_type][0].toString());
            typeLists[i_type].append(typeArrays[i_type][0].toString());
            pathSet = true;
        }
        else
            pathSet = true;
    }
    if (arraySize > 1)
    {
        if (!i_onlyCheckEmpty){
            for (int a = 0; a < arraySize; a++)
            {
                QString pom = typeArrays[i_type][a].toString();
                i_box->addItem(pom);
                typeLists[i_type].append(pom);
            }
            pathSet = true;
        }
        else
            pathSet = true;
    }
    return pathSet;
}

bool DirectoriesLoader::getPathsFromJson(bool i_onlyCheckEmpty)
{
    bool allCategoriesSet = false;

    bool VFA = getPathFromJson(pathTypes.at(0),ui->Combo_videoPath,i_onlyCheckEmpty);
    if (!VFA){
        if (!localErrorDialogHandling[ui->Combo_videoPath]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_videoPath]->evaluate("center","hardError",2);
            localErrorDialogHandling[ui->Combo_videoPath]->show(false);
        }
    }
    else{
        if (localErrorDialogHandling[ui->Combo_videoPath]->isEvaluated())
            localErrorDialogHandling[ui->Combo_videoPath]->hide();
    }

    bool SV = getPathFromJson(pathTypes.at(1),ui->Combo_savingVideo,i_onlyCheckEmpty);
    if (!SV){
        if (!localErrorDialogHandling[ui->Combo_savingVideo]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_savingVideo]->evaluate("center","hardError",2);
            localErrorDialogHandling[ui->Combo_savingVideo]->show(false);
        }
    }
    else{
        if (localErrorDialogHandling[ui->Combo_savingVideo]->isEvaluated())
            localErrorDialogHandling[ui->Combo_savingVideo]->hide();
    }

    bool VDL = getPathFromJson(pathTypes.at(2),ui->Combo_VideoDataLoad,i_onlyCheckEmpty);
    if (!VDL)
    {
        if (!localErrorDialogHandling[ui->Combo_VideoDataLoad]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_VideoDataLoad]->evaluate("center","hardError",2);
            localErrorDialogHandling[ui->Combo_VideoDataLoad]->show(false);
        }
    }
    else{
        if (localErrorDialogHandling[ui->Combo_VideoDataLoad]->isEvaluated())
            localErrorDialogHandling[ui->Combo_VideoDataLoad]->hide();
    }

    bool VDS = getPathFromJson(pathTypes.at(3),ui->Combo_VideoDataSave,i_onlyCheckEmpty);
    if (!VDS){
        if (!localErrorDialogHandling[ui->Combo_VideoDataSave]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_VideoDataSave]->evaluate("center","hardError",2);
            localErrorDialogHandling[ui->Combo_VideoDataSave]->show(false);
        }
    }
    else{
        if (localErrorDialogHandling[ui->Combo_VideoDataSave]->isEvaluated())
            localErrorDialogHandling[ui->Combo_VideoDataSave]->hide();
    }

    bool FP = getPathFromJson(pathTypes.at(4),ui->Combo_FrangiParams,i_onlyCheckEmpty);
    if (FP && !SharedVariables::getSharedVariables()->processFrangiParameters(typeLists["parametersFrangiFiltr"].at(0))){
        if (!localErrorDialogHandling[ui->Combo_FrangiParams]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("center","softError",2);
            localErrorDialogHandling[ui->Combo_FrangiParams]->show(false);
        }
        FP = false;
    }
    else if (!FP){
        if (!localErrorDialogHandling[ui->Combo_FrangiParams]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("center","hardError",2);
            localErrorDialogHandling[ui->Combo_FrangiParams]->show(false);
        }
    }
    else{
        if (localErrorDialogHandling[ui->Combo_FrangiParams]->isEvaluated())
            localErrorDialogHandling[ui->Combo_FrangiParams]->hide();
    }

    if (VFA && SV && VDL && VDS && FP){
        qDebug()<<"Loading successfull. Filling comboboxes.";
        for (int typeIndex = 0; typeIndex < pathTypes.count(); typeIndex++)
            SharedVariables::getSharedVariables()->setPath(pathTypes.at(typeIndex),typeLists[pathTypes.at(typeIndex)].at(0));

        connect(ui->Combo_videoPath,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        connect(ui->Combo_savingVideo,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        connect(ui->Combo_VideoDataLoad,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        connect(ui->Combo_VideoDataSave,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        connect(ui->Combo_FrangiParams,SIGNAL(currentIndexChanged(int)),this,SLOT(chosenPath(int)));
        allCategoriesSet = true;
    }
    return allCategoriesSet;
}

void DirectoriesLoader::chosenPath(int i_index)
 {
     if (QObject::sender() == ui->Combo_videoPath)
         SharedVariables::getSharedVariables()->setPath("videosPath",typeLists["videosPath"].at(i_index));
     if (QObject::sender() == ui->Combo_savingVideo)
         SharedVariables::getSharedVariables()->setPath("saveVideosPath",typeLists["saveVideosPath"].at(i_index));
     if (QObject::sender() == ui->Combo_VideoDataLoad)
         SharedVariables::getSharedVariables()->setPath("loadDatFilesPath",typeLists["loadDatFilesPath"].at(i_index));
     if (QObject::sender() == ui->Combo_FrangiParams)
         SharedVariables::getSharedVariables()->setPath("saveDatFilesPath",typeLists["saveDatFilesPath"].at(i_index));
     if (QObject::sender() == ui->Combo_FrangiParams){
         SharedVariables::getSharedVariables()->setPath("parametersFrangiFiltr",typeLists["parametersFrangiFiltr"].at(i_index));
         if (!SharedVariables::getSharedVariables()->processFrangiParameters(typeLists["parametersFrangiFiltr"].at(i_index))){
             localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","softError",2);
             localErrorDialogHandling[ui->Combo_FrangiParams]->show(false);
         }
         else
             localErrorDialogHandling[ui->Combo_FrangiParams]->hide();
     }
 }

void DirectoriesLoader::on_ChooseFileFolderDirectory_clicked()
{
    if (search == "json" || search == ""){
        if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        QString jsonPath = QFileDialog::getOpenFileName(this,
             tr("Find json file"), QDir::currentPath(),"*.json");
        if (jsonPath != ""){
            GlobalSettings::getSettings()->setFileFolderDirectoriesPath(jsonPath);
            pathToFileFolderDirectory = jsonPath;
            ui->FileFolderDirectory->setText(pathToFileFolderDirectory);
            if (loadJsonPaths()){                
                emit fileFolderDirectoryFound();
            }
            enableElements();
        }       
    }
    else if (search == "ini"){
        QString settingsPath = QFileDialog::getOpenFileName(this,
             tr("Find ini file"), QDir::currentPath(),"*.ini");
        if (settingsPath != ""){
            QString folder,filename,suffix;
            processFilePath(settingsPath,folder,filename,suffix);
            _tempSettignsFile = filename+".ini";
            GlobalSettings::getSettings()->setIniPath(folder,_tempSettignsFile);
            createIni();

            search = "reload";
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
            localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","wtd","Reload folder paths!");
            localErrorDialogHandling[ui->FileFolderDirectory]->show(false);
            ui->ChooseFileFolderDirectory->setText(tr("Reload paths"));
            ui->iniPath->setText(settingsPath);
        }
    }
    else if (search == "reload"){
        localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        pathToFileFolderDirectory = GlobalSettings::getSettings()->getFileFolderDirectoriesPath();
        if (!processLoadedSettings()){
            localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","softError",0);
            localErrorDialogHandling[ui->FileFolderDirectory]->show(false);
            search = "json";
            ui->ChooseFileFolderDirectory->setText(tr("Find json file"));
        }
        else{
            QFile _iniFile(QCoreApplication::applicationDirPath()+"/settings.ini");
            _iniFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
            QTextStream stream(&_iniFile);
            stream<<"fileFolderDirectoryPath="+pathToFileFolderDirectory+"\n";
            _iniFile.close();
            ui->FileFolderDirectory->setText(pathToFileFolderDirectory);
            ui->ChooseFileFolderDirectory->setText(tr("Choose file folder directories"));
            search = "json";
            GlobalSettings::getSettings()->setIniPath(QCoreApplication::applicationDirPath(),"settings.ini");
        }
    }    
}

void DirectoriesLoader::createIni(){
    qDebug()<<"Creating ini file";
    QFile _iniFile(QCoreApplication::applicationDirPath()+"/settings.ini");
    _iniFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&_iniFile);
    stream << "[General]\n";
    _iniFile.close();
}

void DirectoriesLoader::processPath(QString i_type, QString i_path){
    QJsonValue pathJson = QJsonValue(i_path);
    typeArrays[i_type].append(pathJson);
    typeLists[i_type].append(i_path);
    QString t = pathToFileFolderDirectory;
    writeJson(fileWithPaths,typeArrays[i_type],i_type,t);
    SharedVariables::getSharedVariables()->setPath(i_type,i_path);
}

void DirectoriesLoader::on_pathToVideos_clicked()
{
    QString pathToVideos = QFileDialog::getExistingDirectory(this,
                                                             tr("Choose folder where the videos will be loaded from"),
                                                             QDir::currentPath());
    if (pathToVideos != ""){
        processPath("videosPath",pathToVideos);
        ui->Combo_videoPath->addItem(pathToVideos);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["videosPath"];
    }
}

void DirectoriesLoader::on_SaveVideos_clicked()
{
    QString pathToVideoSave = QFileDialog::getExistingDirectory(this,
                                                                tr("Choose folder where registrated videos will be saved"),
                                                                QDir::currentPath());
    if (pathToVideoSave != ""){
        processPath("saveVideosPath",pathToVideoSave);
        ui->Combo_savingVideo->addItem(pathToVideoSave);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["saveVideosPath"];
    }
}

void DirectoriesLoader::on_LoadingDataFolder_clicked()
{
    QString pathToVideoParameters = QFileDialog::getExistingDirectory(this,
                                                                      tr("Choose folder with video parametric files"),
                                                                      QDir::currentPath());
    if (pathToVideoParameters != ""){
        processPath("loadDatFilesPath",pathToVideoParameters);
        ui->Combo_VideoDataLoad->addItem(pathToVideoParameters);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["loadDatFilesPath"];
    }
}

void DirectoriesLoader::on_SavingDataFolder_clicked()
{
    QString pathToSaveVideoParameters = QFileDialog::getExistingDirectory(this,
                                                                          tr("Choose folder where video parametric files will be saved"),
                                                                          QDir::currentPath());
    if (pathToSaveVideoParameters != ""){
        processPath("saveDatFilesPath",pathToSaveVideoParameters);
        ui->Combo_VideoDataSave->addItem(pathToSaveVideoParameters);
        if (getPathsFromJson(true))
            emit fileFolderDirectoryFound();
        qDebug()<<typeArrays["saveDatFilesPath"];
    }
}

void DirectoriesLoader::on_paramFrangPB_clicked()
{
    QString pathFF = QFileDialog::getExistingDirectory(this,tr("Choose folder with Frangi parameters file"),
                                                       QDir::currentPath());
    QString completePath = pathFF+"/frangiParameters.json";
    QFileInfo check_file(completePath);
    if (check_file.exists() && check_file.isFile()){
        processPath("parametersFrangiFiltr",pathFF);
        ui->Combo_FrangiParams->addItem(pathFF);
        if (getPathsFromJson(true)) {
            emit fileFolderDirectoryFound();
        }
    }
    else{
        if (!localErrorDialogHandling[ui->Combo_FrangiParams]->isEvaluated()) {
            localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","hardError",7);
            localErrorDialogHandling[ui->Combo_FrangiParams]->show(false);
        }
    }
}

bool DirectoriesLoader::eventFilter(QObject *obj, QEvent *event){
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key::Key_Delete)
        {
            auto combobox = dynamic_cast<QComboBox *>(obj);
            if (combobox && combobox->count() > 1){
                combobox->removeItem(combobox->currentIndex());
                return true;
            }
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

/*void DirectoriesLoader::on_FileFolderDirectory_textEdited(const QString &arg1)
{
    QStringList foundJSONs;
    int JSONsCount;
    analyseFileNames(arg1,foundJSONs,JSONsCount,"json");
    if (JSONsCount == 0){
        disableElements();
        emit fileFolderDirectoryNotFound();
    }
    else{
        enableElements();
        emit fileFolderDirectoryFound();
    }
}*/

void DirectoriesLoader::enableElements(){
    ui->pathToVideos->setEnabled(true);
    ui->SaveVideos->setEnabled(true);
    ui->LoadingDataFolder->setEnabled(true);
    ui->SavingDataFolder->setEnabled(true);
    ui->paramFrangPB->setEnabled(true);
}

void DirectoriesLoader::disableElements(){
    ui->pathToVideos->setEnabled(false);
    ui->SaveVideos->setEnabled(false);
    ui->LoadingDataFolder->setEnabled(false);
    ui->SavingDataFolder->setEnabled(false);
    ui->paramFrangPB->setEnabled(false);
}
