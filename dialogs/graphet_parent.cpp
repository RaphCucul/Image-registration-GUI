#include "graphet_parent.h"
#include "ui_graphet_parent.h"
#include "dialogs/grafet.h"
#include "util/files_folders_operations.h"

GraphET_parent::GraphET_parent(QStringList i_chosenList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphET_parent)
{
    ui->setupUi(this);
    fileList = i_chosenList;
    analyseNames(namesFlag::Fullname);
    loadAndShow();
    this->setStyleSheet("background-color: white");
}

GraphET_parent::GraphET_parent(QStringList i_chosenList,
                               QVector<QVector<double> > i_entropy,
                               QVector<QVector<double> > i_tennengrad,
                               QVector<QVector<int> > i_FirstEvalEntropy,
                               QVector<QVector<int> > i_FirstEvalTennengrad,
                               QVector<QVector<int> > i_FirstDecisionResults,
                               QVector<QVector<int> > i_SecondDecisionResults,
                               QVector<QVector<int> > i_CompleteEvaluation,
                               QWidget *parent) : QDialog(parent),ui(new Ui::GraphET_parent)
{
    ui->setupUi(this);
    fileList = i_chosenList;    
    processAndShow(i_entropy,i_tennengrad,i_FirstEvalEntropy,i_FirstEvalTennengrad,i_FirstDecisionResults,
                   i_SecondDecisionResults,i_CompleteEvaluation);
    this->setStyleSheet("background-color: white");
    analyseNames(namesFlag::FilenameOnly);
}

GraphET_parent::~GraphET_parent()
{
    delete ui;
}

void GraphET_parent::analyseNames(namesFlag parameter){
    int maxLength = 0;
    for (int var = 0; var < fileList.count(); var++) {
        if (parameter == namesFlag::Fullname){
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

void GraphET_parent::loadAndShow(){
    QMap<QString,QVector<QVector<double>>> mapDouble;
    QMap<QString,QVector<QVector<int>>> mapInt;

    for (int fileIndex = 0; fileIndex < fileList.count(); fileIndex++){
        QFile datFile(fileList.at(fileIndex));
        if (datFile.exists()){
            QJsonObject loadedJSON = readJson(datFile);
            for (int parameterIndex = 0; parameterIndex < neededParameters.count(); parameterIndex++){
                QJsonArray loadedArrayData = loadedJSON[neededParameters.at(parameterIndex)].toArray();
                if (parameterIndex < 2){
                    QVector<double> _pom = arrayDouble2vector(loadedArrayData);
                    mapDouble[neededParameters.at(parameterIndex)].append(_pom);
                }
                else{
                    QVector<int> _pom = arrayInt2vector(loadedArrayData);
                    mapInt[neededParameters.at(parameterIndex)].append(_pom);
                }
            }
        }
    }
    for (int fileIndex = 0; fileIndex < fileList.count(); fileIndex++){

        GrafET* _graph = new GrafET(mapDouble["entropy"][fileIndex],
                                    mapDouble["tennengrad"][fileIndex],
                                    mapInt["firstEvalEntropy"][fileIndex],
                                    mapInt["firstEvalTennengrad"][fileIndex],
                                    mapInt["firstEval"][fileIndex],
                                    mapInt["secondEval"][fileIndex],
                                    mapInt["evaluation"][fileIndex]);
        ui->tabWidget->addTab(_graph,filenameList.at(fileIndex));
    }
}

void GraphET_parent::processAndShow(QVector<QVector<double> > i_entropy,
                                    QVector<QVector<double> > i_tennengrad,
                                    QVector<QVector<int> > i_FirstEvalEntropy,
                                    QVector<QVector<int> > i_FirstEvalTennengrad,
                                    QVector<QVector<int> > i_FirstDecisionResults,
                                    QVector<QVector<int> > i_SecondDecisionResults,
                                    QVector<QVector<int> > i_CompleteEvaluation){
    for (int fileIndex = 0; fileIndex < fileList.count(); fileIndex++){
        QStringList nameList;
        QString folder,file,suffix;
        processFilePath(fileList.at(fileIndex),folder,file,suffix);
        GrafET* _graph = new GrafET(i_entropy[fileIndex],
                                    i_tennengrad[fileIndex],
                                    i_FirstEvalEntropy[fileIndex],
                                    i_FirstEvalTennengrad[fileIndex],
                                    i_FirstDecisionResults[fileIndex],
                                    i_SecondDecisionResults[fileIndex],
                                    i_CompleteEvaluation[fileIndex]);
        ui->tabWidget->addTab(_graph,file);
    }
}
