#include "graphet_parent.h"
#include "ui_graphet_parent.h"
#include "dialogs/grafet.h"
#include "util/files_folders_operations.h"
#include "shared_staff/sharedvariables.h"

GraphET_parent::GraphET_parent(QStringList i_chosenList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphET_parent)
{
    ui->setupUi(this);
    fileList = i_chosenList;
    analyseNames(namesFlag::FilenameOnly);
    loadAndShow();
    this->setStyleSheet("background-color: white");
}

GraphET_parent::GraphET_parent(QStringList i_chosenList,
                               QMap<QString, QVector<double>> i_entropy,
                               QMap<QString, QVector<double>> i_tennengrad,
                               QMap<QString, QVector<double> > i_thresholds,
                               QMap<QString, QVector<int>> i_FirstEvalEntropy,
                               QMap<QString, QVector<int>> i_FirstEvalTennengrad,
                               QMap<QString, QVector<int>> i_FirstDecisionResults,
                               QMap<QString, QVector<int>> i_SecondDecisionResults,
                               QMap<QString, QVector<int>> i_CompleteEvaluation,
                               QWidget *parent) : QDialog(parent),ui(new Ui::GraphET_parent)
{
    ui->setupUi(this);
    fileList = i_chosenList;
    analyseNames(namesFlag::FilenameOnly);
    processAndShow(i_entropy,i_tennengrad,i_thresholds,i_FirstEvalEntropy,i_FirstEvalTennengrad,i_FirstDecisionResults,
                   i_SecondDecisionResults,i_CompleteEvaluation);
    this->setStyleSheet("background-color: white");
}

GraphET_parent::~GraphET_parent()
{
    delete ui;
}

void GraphET_parent::analyseNames(namesFlag parameter){
    int maxLength = 0;
    for (int var = 0; var < fileList.count(); var++) {
        if (parameter == namesFlag::FilenameOnly){
            QStringList nameList;
            QString folder,file,suffix;
            processFilePath(fileList.at(var),folder,file,suffix);
            if (file.length() > maxLength){
                maxLength = file.length();
                filenameList.append(file);
            }
            else{
                filenameList.append(file);
                continue;
            }
            videoReferences.insert(fileList.at(var),file);
        }
        else{
            if (fileList.at(var).length()>maxLength)
                maxLength = fileList.at(var).length();
            else
                continue;
        }
    }
    maxLength*=7;
    ui->tabWidget->setStyleSheet("QTabBar::tab { height:20px; width:"+QString::number(maxLength)+"px; }");
}

bool GraphET_parent::prepareData(QString i_videoName, QJsonObject& loadedData) {
    QString filePath = videoReferences.key(i_videoName);
    QFile datFile(filePath);
    if (datFile.exists()){
        loadedData = readJson(datFile);
        return true;
    }
    else
        return false;
}

void GraphET_parent::loadAndShow(){
    QMap<QString,QMap<QString,QVector<double>>> mapDouble;
    QMap<QString,QMap<QString,QVector<int>>> mapInt;

    for (int parameterIndex = 0; parameterIndex < neededParameters.count(); parameterIndex++) {
        foreach (QString filePath, fileList ) {
            QFile datFile(filePath);
            if (datFile.exists()) {
                QString video = videoReferences[filePath];
                QJsonObject loadedJSON = readJson(datFile);

                QMap<QString,QVector<double>> _pomD;
                QMap<QString,QVector<int>> _pomI;
                QJsonArray loadedArrayData = loadedJSON[neededParameters.at(parameterIndex)].toArray();
                if (parameterIndex < 3){
                    _pomD.insert(video,array2vector<double>(loadedArrayData));
                }
                else{
                    _pomI.insert(video,array2vector<int>(loadedArrayData));
                }
                if (parameterIndex < 3){
                    mapDouble.insert(neededParameters.at(parameterIndex),_pomD);
                }
                else{
                    mapInt.insert(neededParameters.at(parameterIndex),_pomI);
                }
            }
        }
    }

    int index = 0;
    foreach (QString video, filenameList){
        GrafET* _graph = new GrafET(mapDouble["entropy"][video],
                                    mapDouble["tennengrad"][video],
                                    mapDouble["thresholds"][video],
                                    mapInt["firstEvalEntropy"][video],
                                    mapInt["firstEvalTennengrad"][video],
                                    mapInt["firstEval"][video],
                                    mapInt["secondEval"][video],
                                    mapInt["evaluation"][video],
                                    video,"avi");
        ui->tabWidget->addTab(_graph,filenameList.at(index));
        index++;
        /*connect(_graph,&GrafET::saveCalculatedData,[=](){
            qDebug()<<"Saving data from GrafET class object for video "<<_graph->getVideoName()<<"\n"<<"passing the signal";
            emit saveCalculatedData(_graph->getVideoName(), _graph->getReturnJSONObject());
        });*/
        connect(_graph,SIGNAL(saveCalculatedData(QString,QJsonObject)),this,SLOT(saveDataForGivenVideo(QString,QJsonObject)));
        connect(_graph,&GrafET::resizeWindow,[=](){
            resize(sizeHint());
        });
    }
}

void GraphET_parent::processAndShow(QMap<QString,QVector<double>> i_entropy,
                                    QMap<QString,QVector<double>> i_tennengrad,
                                    QMap<QString, QVector<double> > i_thresholds,
                                    QMap<QString,QVector<int>> i_FirstEvalEntropy,
                                    QMap<QString,QVector<int>> i_FirstEvalTennengrad,
                                    QMap<QString,QVector<int>> i_FirstDecisionResults,
                                    QMap<QString,QVector<int>> i_SecondDecisionResults,
                                    QMap<QString,QVector<int>> i_CompleteEvaluation)
{
    foreach (QString file, filenameList){
        GrafET* _graph = new GrafET(i_entropy[file],
                                    i_tennengrad[file],
                                    i_thresholds[file],
                                    i_FirstEvalEntropy[file],
                                    i_FirstEvalTennengrad[file],
                                    i_FirstDecisionResults[file],
                                    i_SecondDecisionResults[file],
                                    i_CompleteEvaluation[file],
                                    file,"avi");
        ui->tabWidget->addTab(_graph,file);
        /*connect(_graph,&GrafET::saveCalculatedData,[=](){
            qDebug()<<"Saving data from GrafET class object for video "<<_graph->getVideoName()<<"\n"<<"passing the signal";
            emit saveCalculatedData(_graph->getVideoName(), _graph->getReturnJSONObject());
        });*/
        connect(_graph,SIGNAL(saveCalculatedData(QString,QJsonObject)),this,SIGNAL(saveCalculatedData(QString,QJsonObject)));
        connect(_graph,&GrafET::resizeWindow,[=](){
            resize(sizeHint());
        });
    }
}

void GraphET_parent::saveDataForGivenVideo(QString i_videoName,QJsonObject i_newData) {
    QJsonObject loadedData;
    QJsonDocument document;
    if (prepareData(i_videoName,loadedData)){
        foreach (QString key, i_newData.keys()) {
            loadedData[key] = i_newData[key];
        }
        document.setObject(loadedData);
        QString documentString = document.toJson();
        QFile writer;
        writer.setFileName(videoReferences.key(i_videoName));
        writer.open(QIODevice::WriteOnly);
        writer.write(documentString.toLocal8Bit());
        writer.close();
    }
    else
        qDebug()<<"Saving for "<<i_videoName<<" failed";
}
