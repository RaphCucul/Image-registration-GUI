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

    localErrorDialogHandling[ui->analyzeVideosPB] = new ErrorDialog(ui->analyzeVideosPB);

    QObject::connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    QObject::connect(ui->verticalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(ui->horizontalAnomaly,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));
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
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
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
    if (runStatus){
        if (checkVideos()){
            qDebug()<<"videoList contains "<<analysedVideos.count()<<" videos.";
            First[1] = new qThreadFirstPart(analysedVideos,
                                            SharedVariables::getSharedVariables()->getVerticalAnomalyCoords(),
                                            SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords(),
                                            SharedVariables::getSharedVariables()->getFrangiParameters());
            QObject::connect(First[1],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
            QObject::connect(First[1],SIGNAL(done(int)),this,SLOT(onDone(int)));
            QObject::connect(First[1],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
            QObject::connect(First[1],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
            QObject::connect(First[1],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
            QObject::connect(this,SIGNAL(dataObtained_first()),First[1],SLOT(onDataObtained()));
            QObject::connect(First[1],SIGNAL(readyForFinish()),First[1],SLOT(deleteLater()));
            First[1]->start();

            initMaps();
            canProceed = true;
            emit calculationStarted();
            ui->analyzeVideosPB->setText(tr("Cancel"));
            runStatus = false;
        }
    }
    else{
        cancelAllCalculations();
        ui->analyzeVideosPB->setText(tr("Analyse videos"));
        ui->computationProgress->setValue(0);
        ui->actualMethod_label->setText("");
        ui->actualVideo_label->setText("");
    }
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

void MultipleVideoET::onDone(int thread){
    done(thread);
    if (thread == 1){
        qDebug()<<"First done, starting second...";
        /*First[1]->terminate();
        //First[1]->exit(0);
        qDebug()<<"Terminated and waiting";
        First[1]->wait();
        First[1]->deleteLater();

        qDebug()<<"Deleted completely";*/
        //delete First.take(1);

        Second[2] = new qThreadSecondPart(analysedVideos,
                                          badVideos,
                                          obtainedCutoffStandard,
                                          obtainedCutoffExtra,
                                          badFramesComplete,
                                          framesReferencial,false);
        QObject::connect(Second[2],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Second[2],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Second[2],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Second[2],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Second[2],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_second()),Second[2],SLOT(onDataObtained()));
        QObject::connect(Second[2],SIGNAL(readyForFinish()),Second[2],SLOT(deleteLater()));

        Second[2]->start();
        qDebug()<<"Started";
    }
    else if (thread == 2){
        qDebug()<<"Second done, starting third...";
        /*Second[2]->terminate();
        //Second[2]->exit(0);
        qDebug()<<"Terminated and waiting";
        Second[2]->wait();
        Second[2]->deleteLater();

        qDebug()<<"Deleted completely";*/
        //delete Second.take(2);

        Third[3] = new qThreadThirdPart(analysedVideos,
                                        badVideos,
                                        badFramesComplete,
                                        mapInt["Ohodnoceni"],
                                        framesReferencial,
                                        averageCCcomplete,
                                        averageFWHMcomplete,
                                        obtainedCutoffStandard,
                                        obtainedCutoffExtra,false);
        QObject::connect(Third[3],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Third[3],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Third[3],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Third[3],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Third[3],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_third()),Third[3],SLOT(onDataObtained()));
        QObject::connect(Third[3],SIGNAL(readyForFinish()),Third[3],SLOT(deleteLater()));

        Third[3]->start();
        qDebug()<<"Started";
    }
    else if (thread == 3){
        qDebug()<<"Third done, starting fourth...";
        /*Third[3]->terminate();
        //Third[3]->exit(0);
        qDebug()<<"Terminated and waiting";
        Third[3]->wait();
        Third[3]->deleteLater();

        qDebug()<<"Deleted completely";*/
        //delete Third.take(3);

        Fourth[4] = new qThreadFourthPart(analysedVideos,
                                          badVideos,
                                          mapInt["PrvniRozhod"],
                                          mapInt["Ohodnoceni"],
                                          CC_problematicFrames,
                                          FWHM_problematicFrames,
                                          mapDouble["POCX"],
                                          mapDouble["POCY"],
                                          mapDouble["Uhel"],
                                          mapDouble["FrangiX"],
                                          mapDouble["FrangiY"],
                                          mapDouble["FrangiEuklid"],
                                          averageCCcomplete,
                                          averageFWHMcomplete);
        QObject::connect(Fourth[4],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Fourth[4],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Fourth[4],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Fourth[4],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(this,SIGNAL(dataObtained_fourth()),Fourth[4],SLOT(onDataObtained()));
        QObject::connect(Fourth[4],SIGNAL(readyForFinish()),Fourth[4],SLOT(deleteLater()));

        Fourth[4]->start();
        qDebug()<<"Started";
    }
    else if (thread == 4){
        qDebug()<<"Fourth done, starting fifth";
        /*Fourth[4]->terminate();
        //Fourth[4]->exit(0);
        qDebug()<<"Terminated and waiting";
        Fourth[4]->wait();
        Fourth[4]->deleteLater();

        qDebug()<<"Deleted completely";*/
        //delete Fourth.take(4);

        Fifth[5] = new qThreadFifthPart(analysedVideos,
                                        badVideos,
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
                                       SharedVariables::getSharedVariables()->getFrangiParameters(),
                                        int(iterationCount),
                                        areaMaximum,
                                        rotationAngle);
        QObject::connect(Fifth[5],SIGNAL(done(int)),this,SLOT(onDone(int)));
        QObject::connect(Fifth[5],SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        QObject::connect(Fifth[5],SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        QObject::connect(Fifth[5],SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        QObject::connect(Fifth[5],SIGNAL(unexpectedTermination(int,QString)),this,SLOT(onUnexpectedTermination(int,QString)));
        QObject::connect(this,SIGNAL(dataObtained_fifth()),Fifth[5],SLOT(onDataObtained()));
        QObject::connect(Fifth[5],SIGNAL(readyForFinish()),Fifth[5],SLOT(deleteLater()));

        Fifth[5]->start();
        qDebug()<<"Started";
    }
    else if (thread == 5){
        ui->showResultsPB->setEnabled(true);
        ui->savePB->setEnabled(true);
        ui->actualMethod_label->setText(tr("Fifth part done. Analysis completed"));
        qDebug()<<"Fifth done.";
        /*Fifth[5]->terminate();
        //Fifth[5]->exit(0);
        qDebug()<<"Terminated and waiting";
        Fifth[5]->wait();
        Fifth[5]->deleteLater();

        qDebug()<<"Deleted completely";*/
        //delete Fifth.take(5);
        emit calculationStopped();
    }
}

void MultipleVideoET::onUnexpectedTermination(int videoIndex, QString errorType){
    Q_UNUSED(videoIndex);
    localErrorDialogHandling[ui->analyzeVideosPB]->evaluate("left",errorType,"Video could not be analysed.");
    localErrorDialogHandling[ui->analyzeVideosPB]->show();
    if (errorType == "hardError"){
        cancelAllCalculations();
        emit calculationStopped();
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

bool MultipleVideoET::checkVideos(){
    QVector<int> _badVideos;
    QStringList _temp;
    for (int var = 0; var < videoList.count(); var++) {
        cv::VideoCapture cap = cv::VideoCapture(videoList.at(var).toLocal8Bit().constData());
        if (!cap.isOpened()){
            _badVideos.push_back(var);
            ui->selectedVideos->item(var)->setBackgroundColor(Qt::red);
        }
        else{
            _temp.append(videoList.at(var));
            ui->selectedVideos->item(var)->setBackgroundColor(Qt::blue);
        }
    }
    if (_badVideos.length() == videoList.count()){
            localErrorDialogHandling[ui->analyzeVideosPB]->evaluate("center","softError",3);
            localErrorDialogHandling[ui->analyzeVideosPB]->show();
            return false;
    }
    else{
        analysedVideos = _temp;
        return true;
    }
}

void MultipleVideoET::showDialog(){
    if (ui->verticalAnomaly->isChecked())
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,1);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
    if (ui->horizontalAnomaly->isChecked())
    {
        QString fullPath = chosenVideoETSingle[0]+"/"+chosenVideoETSingle[1]+"."+chosenVideoETSingle[2];
        ClickImageEvent* markAnomaly = new ClickImageEvent(fullPath,referencialNumber,2);
        markAnomaly->setModal(true);
        markAnomaly->show();
    }
}

void MultipleVideoET::disableWidgets(){
    ui->wholeFolderPB->setEnabled(false);
    ui->afewVideosPB->setEnabled(false);
    ui->deleteChosenFromListPB->setEnabled(false);
    ui->verticalAnomaly->setEnabled(false);
    ui->horizontalAnomaly->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->showResultsPB->setEnabled(false);
}

void MultipleVideoET::enableWidgets(){
    ui->wholeFolderPB->setEnabled(true);
    ui->afewVideosPB->setEnabled(true);
    ui->deleteChosenFromListPB->setEnabled(true);
    ui->verticalAnomaly->setEnabled(true);
    ui->horizontalAnomaly->setEnabled(true);
    ui->savePB->setEnabled(true);
    ui->showResultsPB->setEnabled(true);
}
