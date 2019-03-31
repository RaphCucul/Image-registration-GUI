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
#include <exception>
#include <QCoreApplication>

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
    ui->ChooseFileFolderDirectory->setText(tr("Choose file folder directories"));
    ui->FileFolderDirectory->setPlaceholderText(tr("Folder with paths"));

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    localErrorDialogHandling[ui->FileFolderDirectory] = new ErrorDialog(ui->FileFolderDirectory);
    localErrorDialogHandling[ui->Combo_videoPath] = new ErrorDialog(ui->Combo_videoPath);
    ui->Combo_videoPath->installEventFilter(this);
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
    if (pom == ""){
        iniFileProcessingResult = "MissingPath";
        disableElements();
    }
    else{
        pathToFileFolderDirectory = pom;
        ui->FileFolderDirectory->setText(pathToFileFolderDirectory);
        if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        iniFileProcessingResult = "OK";
        enableElements();
    }
    qDebug()<<"Path to file folder directory: "<<pathToFileFolderDirectory;
    return iniFileProcessingResult;
}

bool t_b_HO::processLoadedSettings(){
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
            disableElements();
            return false;
        }
    }
}

bool t_b_HO::checkFileFolderExistence()
{
    QString checkIniPath = GlobalSettings::getSettings()->getIniPath();
    int numberOfIni=-1;
    QStringList foundSettingsFiles;
    analyseFileNames(checkIniPath,foundSettingsFiles,numberOfIni,"ini");
    bool fileExistence = false;

    if (numberOfIni == 1){
        ui->iniPath->setText(checkIniPath);
        QString loadingResult = LoadSettings();
        if (loadingResult == "MissingPath"){
            localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","softError",0);
            localErrorDialogHandling[ui->FileFolderDirectory]->show();
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
        localErrorDialogHandling[ui->FileFolderDirectory]->show();
        ui->ChooseFileFolderDirectory->setText(tr("Find ini file"));
        search = "ini";
    }
    return fileExistence;
}

bool t_b_HO::loadJsonPaths()
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
        localErrorDialogHandling[ui->FileFolderDirectory]->show();
        return false;
    }

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
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    bool VFA = getPathFromJson(pathTypes.at(0),ui->Combo_videoPath,onlyCheckEmpty);
    if (!VFA){
        localErrorDialogHandling[ui->Combo_videoPath]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_videoPath]->show();
    }
    else{
        if (localErrorDialogHandling[ui->Combo_videoPath]->isEvaluated())
            localErrorDialogHandling[ui->Combo_videoPath]->hide();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    bool SV = getPathFromJson(pathTypes.at(1),ui->Combo_savingVideo,onlyCheckEmpty);
    if (!SV){
        localErrorDialogHandling[ui->Combo_savingVideo]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_savingVideo]->show();
    }
    else{
        if (localErrorDialogHandling[ui->Combo_savingVideo]->isEvaluated())
            localErrorDialogHandling[ui->Combo_savingVideo]->hide();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    bool VDL = getPathFromJson(pathTypes.at(2),ui->Combo_VideoDataLoad,onlyCheckEmpty);
    if (!VDL)
    {
        localErrorDialogHandling[ui->Combo_VideoDataLoad]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_VideoDataLoad]->show();
    }
    else{
        if (localErrorDialogHandling[ui->Combo_VideoDataLoad]->isEvaluated())
            localErrorDialogHandling[ui->Combo_VideoDataLoad]->hide();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    bool VDS = getPathFromJson(pathTypes.at(3),ui->Combo_VideoDataSave,onlyCheckEmpty);
    if (!VDS){
        localErrorDialogHandling[ui->Combo_VideoDataSave]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_VideoDataSave]->show();
    }
    else{
        if (localErrorDialogHandling[ui->Combo_VideoDataSave]->isEvaluated())
            localErrorDialogHandling[ui->Combo_VideoDataSave]->hide();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    bool FP = getPathFromJson(pathTypes.at(4),ui->Combo_FrangiParams,onlyCheckEmpty);
    if (FP && !SharedVariables::getSharedVariables()->processFrangiParameters(typeLists["parametryFrangiFiltr"].at(0))){
        localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","softError",2);
        localErrorDialogHandling[ui->Combo_FrangiParams]->show();
        FP = false;
    }
    else if (!FP){
        localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","hardError",2);
        localErrorDialogHandling[ui->Combo_FrangiParams]->show();
    }
    else{
        if (localErrorDialogHandling[ui->Combo_FrangiParams]->isEvaluated())
            localErrorDialogHandling[ui->Combo_FrangiParams]->hide();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    if (VFA && SV && VDL && VDS && FP){
        qDebug()<<"Loading successfull. Filling comboboxes.";
        for (int typeIndex = 0; typeIndex < pathTypes.count(); typeIndex++)
            SharedVariables::getSharedVariables()->setPath(pathTypes.at(typeIndex),typeLists[pathTypes.at(typeIndex)].at(0));

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
    return allCategoriesSet;
}

void t_b_HO::chosenPath(int index)
 {
    //qDebug()<<index;
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
         if (!SharedVariables::getSharedVariables()->processFrangiParameters(typeLists["parametryFrangiFiltr"].at(index))){
             localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","softError",2);
             localErrorDialogHandling[ui->Combo_FrangiParams]->show();
         }
         else
             localErrorDialogHandling[ui->Combo_FrangiParams]->hide();
     }
     //qDebug()<<index<<typeLists["cestaKvideim"].at(index);
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
    if (search == "json"){
        localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        QString jsonPath = QFileDialog::getOpenFileName(this,
             tr("Find json file"), QDir::currentPath(),"*.json");
        if (jsonPath != ""){
            GlobalSettings::getSettings()->setFileFolderDirectoriesPath(jsonPath);
            pathToFileFolderDirectory = jsonPath;
            if (loadJsonPaths()){
                ui->FileFolderDirectory->setText(pathToFileFolderDirectory);
                emit fileFolderDirectoryFound();
            }
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
            localErrorDialogHandling[ui->FileFolderDirectory]->show();
            ui->ChooseFileFolderDirectory->setText(tr("Reload paths"));
            ui->iniPath->setText(settingsPath);
        }
    }
    else if (search == "reload"){
        localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        pathToFileFolderDirectory = GlobalSettings::getSettings()->getFileFolderDirectoriesPath();
        if (!processLoadedSettings()){
            localErrorDialogHandling[ui->FileFolderDirectory]->evaluate("center","softError",0);
            localErrorDialogHandling[ui->FileFolderDirectory]->show();
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

void t_b_HO::createIni(){
    qDebug()<<"Creating ini file";
    QFile _iniFile(QCoreApplication::applicationDirPath()+"/settings.ini");
    _iniFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&_iniFile);
    stream << "[General]\n";
    _iniFile.close();
}

void t_b_HO::processPath(QString type, QString path){
    QJsonValue pathJson = QJsonValue(path);
    typeArrays[type].append(pathJson);
    typeLists[type].append(path);
    QString t = pathToFileFolderDirectory;
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
    QString pathToVideoParameters = QFileDialog::getExistingDirectory(this,tr("Vyberte složku obsahující parametrický soubory"),QDir::currentPath());
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
    QString pathToSaveVideoParameters = QFileDialog::getExistingDirectory(this,tr("Vyberte složku pro uložení parametrických souborů"),QDir::currentPath());
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
        if (!SharedVariables::getSharedVariables()->processFrangiParameters(pathFF)){
            localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","softError",2);
            localErrorDialogHandling[ui->Combo_FrangiParams]->show();
        }
        else{
            if (getPathsFromJson(true))
                emit fileFolderDirectoryFound();
        }
    }
    else{
        localErrorDialogHandling[ui->Combo_FrangiParams]->evaluate("left","hardError",7);
        localErrorDialogHandling[ui->Combo_FrangiParams]->show();
    }
}

bool t_b_HO::eventFilter(QObject *obj, QEvent *event){
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key::Key_Delete)
        {
            auto combobox = dynamic_cast<QComboBox *>(obj);
            if (combobox){
                combobox->removeItem(combobox->currentIndex());
                return true;
            }
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
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
    ui->pathToVideos->setEnabled(true);
    ui->SaveVideos->setEnabled(true);
    ui->LoadingDataFolder->setEnabled(true);
    ui->SavingDataFolder->setEnabled(true);
    ui->paramFrangPB->setEnabled(true);
}

void t_b_HO::disableElements(){
    ui->pathToVideos->setEnabled(false);
    ui->SaveVideos->setEnabled(false);
    ui->LoadingDataFolder->setEnabled(false);
    ui->SavingDataFolder->setEnabled(false);
    ui->paramFrangPB->setEnabled(false);
}
