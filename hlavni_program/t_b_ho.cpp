#include "hlavni_program/t_b_ho.h"
#include "ui_t_b_ho.h"
#include "util/souborove_operace.h"
#include "dialogy/errordialog.h"
#include "mainwindow.h"
#include "fancy_staff/globalsettings.h"

#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QSettings>

QString videaKanalyzeAktual;
QString ulozeniVideiAktual;
QString TXTnacteniAktual;
QString TXTulozeniAktual;
QString paramFrangi;


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
    ui->SaveAllPath->setText(tr("Save my paths"));
    ui->ChooseFileFolderDirectory->setText(tr("Choose file folder directory"));
    ui->FileFolderDirectory->setPlaceholderText(tr("Folder with paths"));

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    localErrorDialogHandling[ui->FileFolderDirectory] = new ErrorDialog(ui->FileFolderDirectory);
    localErrorDialogHandling[ui->SaveAllPath] = new ErrorDialog(ui->SaveAllPath);
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
    analyzuj_jmena_souboru_avi(checkIniPath,foundSettingsFiles,numberOfIni,"ini");
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
            analyzuj_jmena_souboru_avi(pathToFileFolderDirectory,foundFiles,numberOfJson,"json");
            if (numberOfJson == 1){
                fileFolderDirectoryName = foundFiles.at(0);
                ui->FileFolderDirectory->setText(pathToFileFolderDirectory+"/"+fileFolderDirectoryName);
                loadJsonPaths();
                fileExistence = true;
                emit fileFolderDirectoryFound();
                if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
                    localErrorDialogHandling[ui->FileFolderDirectory]->hide();
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
        iniFileError = true;
    }

    return fileExistence;
}

void t_b_HO::loadJsonPaths()
{
    QFile file;
    file.setFileName(pathToFileFolderDirectory+"/seznamCest.json");
    fileWithPaths = readJson(file);
    videosForAnalysis = fileWithPaths["cestaKvideim"].toArray();
    savingVideos = fileWithPaths["ulozeniVidea"].toArray();
    videoDataLoad = fileWithPaths["adresarTXT_nacteni"].toArray();
    videoDataSave = fileWithPaths["adresarTXT_ulozeni"].toArray();
    frangiParameters = fileWithPaths["parametryFrangiFiltr"].toArray();
    getPathsFromJson();
}

void t_b_HO::getPathFromJson(QJsonArray& poleCest, QComboBox *box, QStringList &list)
{
    int velikostPole = poleCest.size();
    if (velikostPole == 1)
    {
        box->addItem(poleCest[0].toString());
        list.append(poleCest[0].toString());
    }
    if (velikostPole > 1)
    {
        QStringList seznamCest;
        for (int a = 0; a < velikostPole; a++)
        {
            QString pom = poleCest[a].toString();
            seznamCest.append(pom);
            box->addItem(pom);
            list.append(pom);
        }
        //box->addItems(seznamCest);
        //list = seznamCest;
    }
    if (velikostPole == 0)
    {
        box->addItem("");
        list.append("");
    }
}

void t_b_HO::getPathsFromJson()
{
    getPathFromJson(videosForAnalysis,ui->Combo_videoPath,videosForAnalysisList);
    getPathFromJson(savingVideos,ui->Combo_savingVideo,savingVideosList);
    getPathFromJson(videoDataLoad,ui->Combo_VideoDataLoad,videoDataLoadList);
    getPathFromJson(videoDataSave,ui->Combo_VideoDataSave,videoDataSaveList);
    getPathFromJson(frangiParameters,ui->Combo_FrangiParams,frangiParametersList);

    videaKanalyzeAktual = videosForAnalysisList.at(0);
    ulozeniVideiAktual = savingVideosList.at(0);
    TXTnacteniAktual = videoDataLoadList.at(0);
    TXTulozeniAktual = videoDataSaveList.at(0);
    paramFrangi = frangiParametersList.at(0);

    connect(ui->Combo_videoPath,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    //connect(ui->CB_cesta_k_videim,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->Combo_savingVideo,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    //connect(ui->CB_ulozeni_videa,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->Combo_VideoDataLoad,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    //connect(ui->CB_slozka_txt,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->Combo_VideoDataSave,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
    //connect(ui->CB_ulozeni_txt,&QComboBox::currentTextChanged,this,&t_b_HO::vybranaCestaString);
    connect(ui->Combo_FrangiParams,SIGNAL(currentIndexChanged(int)),this,SLOT(vybranaCesta(int)));
}

void t_b_HO::vybranaCesta(int index)
 {
    qDebug()<<index;
     if (QObject::sender() == ui->Combo_videoPath)
         videaKanalyzeAktual = videosForAnalysisList.at(index);//.toLocal8Bit().constData();
     if (QObject::sender() == ui->Combo_savingVideo)
         ulozeniVideiAktual = savingVideosList.at(index);//.toLocal8Bit().constData();
     if (QObject::sender() == ui->Combo_VideoDataLoad)
         TXTnacteniAktual = videoDataLoadList.at(index);//.toLocal8Bit().constData();
     if (QObject::sender() == ui->Combo_FrangiParams)
         TXTulozeniAktual = videoDataSaveList.at(index);//.toLocal8Bit().constData();
     if (QObject::sender() == ui->Combo_FrangiParams)
         paramFrangi = frangiParametersList.at(index);

     qDebug()<<index<<videaKanalyzeAktual;
 }

void t_b_HO::vybranaCestaString(QString cesta)
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
}

void t_b_HO::on_ChooseFileFolderDirectory_clicked()
{
    QString fileFolderDirectoryPath_new = QFileDialog::getExistingDirectory(this,"",QDir::currentPath());
    pathToFileFolderDirectory = fileFolderDirectoryPath_new;

    if (fileFolderDirectoryPath_new != ""){
        if (localErrorDialogHandling[ui->FileFolderDirectory]->isEvaluated())
            localErrorDialogHandling[ui->FileFolderDirectory]->hide();
        GlobalSettings::getSettings()->setFileFolderDirectoriesPath(fileFolderDirectoryPath_new);
        ui->SaveAllPath->setEnabled(true);
        ui->pathToVideos->setEnabled(true);
        ui->SaveVideos->setEnabled(true);
        ui->LoadingDataFolder->setEnabled(true);
        ui->SavingDataFolder->setEnabled(true);
        ui->paramFrangPB->setEnabled(true);
        int numberOfJson = -1;
        QStringList foundFiles;
        analyzuj_jmena_souboru_avi(pathToFileFolderDirectory,foundFiles,numberOfJson,"json");
        if (numberOfJson == 1){
            fileFolderDirectoryName = foundFiles.at(0);
            ui->FileFolderDirectory->setText(pathToFileFolderDirectory+"/"+fileFolderDirectoryName);
            loadJsonPaths();
            emit fileFolderDirectoryFound();
        }
    }
}

void t_b_HO::on_pathToVideos_clicked()
{
    QString cesta_k_videim = QFileDialog::getExistingDirectory(this,"",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_videim);
    videosForAnalysis.append(novaCesta);
    videosForAnalysisList.append(cesta_k_videim);
    QString t = pathToFileFolderDirectory+"/seznamCest.json";
    writeJson(fileWithPaths,videosForAnalysis,"cestaKvideim",t);
    ui->Combo_videoPath->addItem(cesta_k_videim);

    qDebug()<<videosForAnalysis;
}

void t_b_HO::on_SaveVideos_clicked()
{
    QString cesta_k_ulozeni_videi = QFileDialog::getExistingDirectory(this,"Vyberte složku pro uložení slícovaného videa",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_ulozeni_videi);
    savingVideos.append(novaCesta);
    savingVideosList.append(cesta_k_ulozeni_videi);
    QString t = pathToFileFolderDirectory+"/seznamCest.json";
    writeJson(fileWithPaths,savingVideos,"ulozeniVidea",t);
    ui->Combo_savingVideo->addItem(cesta_k_ulozeni_videi);

   qDebug()<<savingVideos;
}

void t_b_HO::on_LoadingDataFolder_clicked()
{
    QString cesta_k_txt = QFileDialog::getExistingDirectory(this,"Vyberte složku obsahující parametrický soubory",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_txt);
    videoDataLoad.append(novaCesta);
    videoDataLoadList.append(cesta_k_txt);
    QString t = pathToFileFolderDirectory+"/seznamCest.json";
    writeJson(fileWithPaths,videoDataLoad,"adresarTXT_nacteni",t);
    ui->Combo_VideoDataLoad->addItem(cesta_k_txt);

    qDebug()<<videoDataLoad;
}

void t_b_HO::on_SavingDataFolder_clicked()
{
    QString cesta_k_ulozeni_txt = QFileDialog::getExistingDirectory(this,"Vyberte složku pro uložení parametrických souborů",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(cesta_k_ulozeni_txt);
    videoDataSave.append(novaCesta);
    videoDataSaveList.append(cesta_k_ulozeni_txt);
    QString t = pathToFileFolderDirectory+"/seznamCest.json";
    writeJson(fileWithPaths,videoDataSave,"adresarTXT_ulozeni",t);
    ui->Combo_VideoDataSave->addItem(cesta_k_ulozeni_txt);

    qDebug()<<videoDataSave;
}

void t_b_HO::on_paramFrangPB_clicked()
{
    QString parFF = QFileDialog::getExistingDirectory(this,"Vyberte složku obsahující parametry Frangiho filtru",QDir::currentPath());
    QJsonValue novaCesta = QJsonValue(parFF);
    frangiParameters.append(novaCesta);
    frangiParametersList.append(parFF);
    //int velikostPole = TXTnacteni.size();
    QString t = pathToFileFolderDirectory+"/seznamCest.json";
    writeJson(fileWithPaths,frangiParameters,"parametryFrangiFiltr",t);
    ui->Combo_FrangiParams->addItem(parFF);
}

void t_b_HO::on_SaveAllPath_clicked()
{
    if (videosForAnalysisList.isEmpty() || savingVideosList.isEmpty() || videoDataLoadList.isEmpty()
            || videoDataSaveList.isEmpty() || frangiParametersList.isEmpty()){
        localErrorDialogHandling[ui->SaveAllPath]->evaluate("left","info",2);
        localErrorDialogHandling[ui->SaveAllPath]->show();
    }
    else{
        if (localErrorDialogHandling[ui->SaveAllPath]->isEvaluated())
            localErrorDialogHandling[ui->SaveAllPath]->hide();
        fileWithPaths["cestaKvideim"] = videosForAnalysis;
        fileWithPaths[ "ulozeniVidea"] = savingVideos;
        fileWithPaths["adresarTXT_nacteni"] = videoDataLoad;
        fileWithPaths["adresarTXT_ulozeni"] = videoDataSave;
        fileWithPaths["parametryFrangiFiltr"] = frangiParameters;
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

}

void t_b_HO::on_FileFolderDirectory_textEdited(const QString &arg1)
{

}

void t_b_HO::enableElements(){
    ui->SaveAllPath->setEnabled(true);
    ui->pathToVideos->setEnabled(true);
    ui->SaveVideos->setEnabled(true);
    ui->LoadingDataFolder->setEnabled(true);
    ui->SavingDataFolder->setEnabled(true);
    ui->paramFrangPB->setEnabled(true);
}

void t_b_HO::disableElements(){
    ui->SaveAllPath->setEnabled(false);
    ui->pathToVideos->setEnabled(false);
    ui->SaveVideos->setEnabled(false);
    ui->LoadingDataFolder->setEnabled(false);
    ui->SavingDataFolder->setEnabled(false);
    ui->paramFrangPB->setEnabled(false);
}
