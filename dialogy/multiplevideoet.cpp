#include "dialogy/multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "hlavni_program/frangi_detektor.h"
#include "dialogy/grafet.h"
#include "dialogy/clickimageevent.h"
#include "util/souborove_operace.h"
#include "multithreadET/qThreadFirstPart.h"
#include "util/prace_s_vektory.h"
#include "licovani/rozhodovaci_algoritmy.h"
#include "licovani/multiPOC_Ai1.h"
#include "fancy_staff/sharedvariables.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

MultipleVideoET::MultipleVideoET(QWidget *parent) :
    ETanalysisParent (parent),
    ui(new Ui::MultipleVideoET)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->showResultsPB->setEnabled(false);
    ui->showResultsPB->setText(tr("Show results"));
    ui->savePB->setEnabled(false);
    ui->savePB->setText(tr("Save computed parameters"));
    ui->horizontalAnomaly->setEnabled(false);
    ui->horizontalAnomaly->setText(tr("Top/bottom anomaly"));
    ui->verticalAnomaly->setEnabled(false);
    ui->verticalAnomaly->setText(tr("Left/right anomaly"));
    ui->areaMaximum->setPlaceholderText("0-20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationCount->setPlaceholderText("1-Inf; -1~auto");
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerationlabel->setText(tr("Maximal tolerated rotation angle"));
    ui->numberIterationlabel->setText(tr("Number of iterations of algorithm"));
    ui->afewVideosPB->setText(tr("Choose few files"));
    ui->wholeFolderPB->setText(tr("Choose whole folder"));
    ui->deleteChosenFromListPB->setText(tr("Delete selected"));
    ui->analyzeVideosPB->setText(tr("Analyse videos"));
    ui->analyzeVideosPB->setEnabled(false);

    connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    connect(ui->verticalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    connect(ui->horizontalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
}

MultipleVideoET::~MultipleVideoET()
{
    delete ui;
}

void MultipleVideoET::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
       if (!mimeData->hasUrls()) {
           return;
       }
       QList<QUrl> urls = mimeData->urls();
       foreach (QUrl url,urls){
           QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
           if (mime.inherits("video/x-msvideo")) {
                videoList.append(url.toLocalFile());
              }
       }
       //videoList = seznamVidei;
       qDebug()<<"Aktualizace seznamu videi: "<<videoList;
       ui->selectedVideos->addItems(videoList);
}

void MultipleVideoET::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MultipleVideoET::on_afewVideosPB_clicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),
                            SharedVariables::getSharedVariables()->getPath("cestaKvideim"),
                            tr("Video files (*.avi);;;") );
    if( !filenames.isEmpty() )
    {
        for (int i =0;i<filenames.count();i++)
        {
            ui->selectedVideos->addItem(filenames.at(i));
            videoList.append(filenames.at(i));
        }
    }
}

void MultipleVideoET::on_wholeFolderPB_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                  SharedVariables::getSharedVariables()->getPath("cestaKvideim"),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir chosenDirectory(dir);
    //qDebug()<<"Chosen directory:"<<dir;
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
    //qDebug()<<"contains "<<videosInDirectory.length();
    if (!videosInDirectory.isEmpty())
    {
        for (int a = 0; a < videosInDirectory.count();a++)
        {
            videoList.append(videosInDirectory.at(a));
        }
        ui->selectedVideos->addItems(videosInDirectory);
    }
    qDebug()<<"videoList contains "<<videoList.count()<<" videos.";
}

void MultipleVideoET::on_analyzeVideosPB_clicked()
{
    initMaps();

    QString pom = videoList.at(0);
    cv::VideoCapture cap = cv::VideoCapture(pom.toLocal8Bit().constData());
    if (!cap.isOpened())
        qWarning()<<"Error opening video";

    qDebug()<<"videoList contains "<<videoList.count()<<" videos.";
    TFirstP = new qThreadFirstPart(videoList,
                                   SharedVariables::getSharedVariables()->getVerticalAnomalyCoords(),
                                   SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords(),
                                   SharedVariables::getSharedVariables()->getFrangiParameters());
    connect(TFirstP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
    connect(TFirstP,SIGNAL(done(int)),this,SLOT(done(int)));
    connect(TFirstP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
    connect(TFirstP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
    TFirstP->start();
}

void MultipleVideoET::on_showResultsPB_clicked()
{
    QStringList inputVector;
    for (int a=0; a<videoList.count(); a++)
    {
        QString fullPath = videoList.at(a);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        inputVector.append(jmeno);
    }
    GrafET* graf_ET = new GrafET(
                mapDouble["entropie"],
                mapDouble["tennengrad"],
                mapInt["PrvotOhodEntropie"],
                mapInt["PrvotOhodTennengrad"],
                mapInt["PrvniRozhod"],
                mapInt["DruheRozhod"],
                mapInt["Ohodnoceni"],
                inputVector,
                this);
    graf_ET->setModal(true);
    graf_ET->show();
}

void MultipleVideoET::on_deleteChosenFromListPB_clicked()
{
    QList<QListWidgetItem*> selectedVideos = ui->selectedVideos->selectedItems();
    //qDebug()<<"Selected videos will be deleted: "<<selectedVideos;
    foreach (QListWidgetItem* item,selectedVideos)
    {
        int index = ui->selectedVideos->row(item);
        videoList.removeAt(index);
        delete ui->selectedVideos->takeItem(ui->selectedVideos->row(item));
    }
    //qDebug()<<"Number of videos after deletion: "<<ui->vybranaVidea->count();
    //qDebug()<<"Number of videos in the video list: "<<videoList.count();
}

void MultipleVideoET::on_savePB_clicked()
{

    for (int a = 0; a < videoList.count(); a++)
    {
        QJsonDocument document;
        QJsonObject object;
        QString folder,filename,suffix;
        QString fullPath = videoList.at(a);
        processFilePath(fullPath,folder,filename,suffix);
        QString aktualJmeno = filename;
        QString cesta = SharedVariables::getSharedVariables()->getPath("adresarTXT_ulozeni")+"/"+aktualJmeno+".dat";
        for (int indexVideo=0; indexVideo<mapDouble["entropie"].length(); indexVideo++){
            for (int parameter = 0; parameter < videoParameters.count(); parameter++){
                qDebug()<<videoParameters.at(parameter);
                if (parameter < 8){
                    QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][indexVideo];
                    QJsonArray pomArray = vector2array(pomDouble);
                    object[videoParameters.at(parameter)] = pomArray;
                }
                else if (parameter >= 8 && parameter <= 12){
                    QVector<int> pomInt = mapInt[videoParameters.at(parameter)][indexVideo];
                    if (videoParameters.at(parameter) == "Ohodnoceni")
                        pomInt[framesReferencial[indexVideo]]=2;

                    QJsonArray pomArray = vector2array(pomInt);
                    object[videoParameters.at(parameter)] = pomArray;
                }
                else{
                    if (videoParameters.at(parameter) == "VerticalAnomaly")
                        object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().y);
                    else
                        object[videoParameters.at(parameter)] = double(SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().x);
                }
            }
        }
        document.setObject(object);
        QString documentString = document.toJson();
        QFile writer;
        writer.setFileName(cesta);
        writer.open(QIODevice::WriteOnly);
        writer.write(documentString.toLocal8Bit());
        writer.close();
    }
}

void MultipleVideoET::done(int finished)
{
    if (finished == 1)
    {
        mapDouble["entropie"] = TFirstP->computedEntropy();
        mapDouble["tennengrad"] = TFirstP->computedTennengrad();
        mapInt["PrvotOhodEntropie"] = TFirstP->computedFirstEntropyEvaluation();
        mapInt["PrvotOhodTennengrad"] = TFirstP->computedFirstTennengradEvalueation();
        obtainedCutoffStandard = TFirstP->computedCOstandard();
        obtainedCutoffExtra = TFirstP->computedCOextra();
        mapInt["Ohodnoceni"] = TFirstP->computedCompleteEvaluation();
        framesReferencial = TFirstP->estimatedReferencialFrames();
        badFramesComplete = TFirstP->computedBadFrames();
        TSecondP = new qThreadSecondPart(videoList,
                                         obtainedCutoffStandard,
                                         obtainedCutoffExtra,
                                         badFramesComplete,
                                         framesReferencial,false);
        connect(TSecondP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TSecondP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TSecondP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TSecondP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TFirstP->isFinished()){
            qDebug()<<"First done, starting second...";
            TFirstP->terminate();
            TFirstP->wait();
            TFirstP->deleteLater();
            TSecondP->start();
        //}
    }
    if (finished == 2)
    {
        averageCCcomplete = TSecondP->computedCC();
        averageFWHMcomplete = TSecondP->computedFWHM();
        TThirdP = new qThreadThirdPart(videoList,
                                       badFramesComplete,
                                       mapInt["Ohodnoceni"],
                                       framesReferencial,
                                       averageCCcomplete,
                                       averageFWHMcomplete,
                                       obtainedCutoffStandard,
                                       obtainedCutoffExtra,false);
        connect(TThirdP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TThirdP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TThirdP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TThirdP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TSecondP->isFinished()){
            qDebug()<<"Second done, starting third...";
            TSecondP->terminate();
            TSecondP->wait();
            TSecondP->deleteLater();
            TThirdP->start();
        //}
    }
    if (finished == 3)
    {
        mapInt["Ohodnoceni"]= TThirdP->framesUpdateEvaluation();
        mapInt["PrvniRozhod"] = TThirdP->framesFirstEvaluationComplete();
        QVector<QVector<double>> KKproblematickychSnimku = TThirdP->framesProblematic_CC();
        QVector<QVector<double>> FWHMproblematickychSnimku = TThirdP->framesProblematic_FWHM();
        mapDouble["FrangiX"] = TThirdP->framesFrangiXestimated();
        mapDouble["FrangiY"] = TThirdP->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = TThirdP->framesFrangiEuklidestimated();
        mapDouble["POCX"] = TThirdP->framesPOCXestimated();
        mapDouble["POCY"] = TThirdP->framesPOCYestimated();
        mapDouble["Uhel"] = TThirdP->framesUhelestimated();
        TFourthP = new qThreadFourthPart(videoList,
                                         mapInt["PrvniRozhod"],
                                         mapInt["Ohodnoceni"],
                                         KKproblematickychSnimku,
                                         FWHMproblematickychSnimku,
                                         mapDouble["POCX"],
                                         mapDouble["POCY"],
                                         mapDouble["Uhel"],
                                         mapDouble["FrangiX"],
                                         mapDouble["FrangiY"],
                                         mapDouble["FrangiEuklid"],
                                         averageCCcomplete,
                                         averageFWHMcomplete);
        connect(TFourthP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TFourthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFourthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFourthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TThirdP->isFinished()){
            qDebug()<<"Third done, starting fourth...";
            TThirdP->terminate();
            TThirdP->wait();
            TThirdP->deleteLater();
            TFourthP->start();
        //}
    }
    if (finished == 4)
    {
        mapInt["Ohodnoceni"] = TFourthP->framesUpdateEvaluationComplete();
        mapInt["DruheRozhod"] = TFourthP->framesSecondEvaluation();
        mapDouble["FrangiX"] = TFourthP->framesFrangiXestimated();
        mapDouble["FrangiY"] = TFourthP->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = TFourthP->framesFrangiEuklidestimated();
        mapDouble["POCX"] = TFourthP->framesPOCXestimated();
        mapDouble["POCY"] = TFourthP->framesPOCYestimated();
        mapDouble["Uhel"] = TFourthP->framesAngleestimated();
        TFifthP = new qThreadFifthPart(videoList,
                                       obtainedCutoffStandard,
                                       obtainedCutoffExtra,
                                       mapDouble["POCX"],
                                       mapDouble["POCY"],
                                       mapDouble["Uhel"],
                                       mapDouble["FrangiX"],
                                       mapDouble["FrangiY"],
                                       mapDouble["FrangiEuklid"],
                                       false,
                                       mapInt["Ohodnoceni"],
                                       mapInt["DruheRozhod"],
                                       framesReferencial,
                                       SharedVariables::getSharedVariables()->getFrangiParameters());
        connect(TFifthP,SIGNAL(done(int)),this,SLOT(done(int)));
        connect(TFifthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFifthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFifthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        //if (TFourthP->isFinished()){
            qDebug()<<"Fourth done, starting fifth";
            TFourthP->terminate();
            TFourthP->wait();
            TFourthP->deleteLater();
            TFifthP->start();
        //}
    }
    if (finished == 5)
    {
        mapDouble["FrangiX"] = TFifthP->framesFrangiXestimated();
        mapDouble["FrangiY"] = TFifthP->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = TFifthP->framesFrangiEuklidestimated();
        mapDouble["POCX"] = TFifthP->framesPOCXestimated();
        mapDouble["POCY"] = TFifthP->framesPOCYestimated();
        mapDouble["Uhel"] = TFifthP->framesAngleestimated();
        mapInt["Ohodnoceni"] = TFifthP->framesUpdateEvaluationComplete();
        ui->showResultsPB->setEnabled(true);
        ui->savePB->setEnabled(true);
        ui->actualMethod_label->setText(tr("Fifth part done. Analysis completed"));
        //if (TFifthP->isFinished()){
            qDebug()<<"Fifth done.";
            TFifthP->terminate();
            TFifthP->wait();
            TFifthP->deleteLater();
        //}
    }
}

void MultipleVideoET::newVideoProcessed(int index)
{
    ui->actualVideo_label->setText("Analysing: "+videoList.at(index)+" ("+QString::number(index+1)+"/"+QString::number(videoList.size())+")");
}

void MultipleVideoET::movedToMethod(int metoda)
{
    if (metoda == 0)
        ui->actualMethod_label->setText("1/5 Entropy and tennengrad computation");
    if (metoda == 1)
        ui->actualMethod_label->setText("2/5 Average correlation and FWHM");
    if (metoda == 2)
        ui->actualMethod_label->setText("3/5 First decision algorithm started");
    if (metoda == 3)
        ui->actualMethod_label->setText("4/5 Second decision algorithm started");
    if (metoda == 4)
        ui->actualMethod_label->setText("5/5 Third decision algorithm started");
}

void MultipleVideoET::evaluateCorrectValues(){
    if (areaMaximumCorrect && rotationAngleCorrect && iterationCountCorrect){
        ui->analyzeVideosPB->setEnabled(true);
    }
    else
        ui->analyzeVideosPB->setEnabled(false);
}

void MultipleVideoET::on_areaMaximum_editingFinished()
{
    bool ok;
    double input = ui->areaMaximum->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,0.0,20.0,ui->areaMaximum,areaMaximum,areaMaximumCorrect);
        checkValuesPass();
    }
}

void MultipleVideoET::on_rotationAngle_editingFinished()
{
    bool ok;
    double input = ui->rotationAngle->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,0.0,0.5,ui->rotationAngle,rotationAngle,rotationAngleCorrect);
        checkValuesPass();
    }
}

void MultipleVideoET::on_iterationCount_editingFinished()
{
    bool ok;
    double input = ui->iterationCount->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,-1.0,0.0,ui->iterationCount,iterationCount,iterationCountCorrect);
        checkValuesPass();
    }
}
