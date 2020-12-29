#include "dialogs/multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "image_analysis/frangi_utilization.h"
#include "image_analysis/image_processing.h"
#include "main_program/frangi_detektor.h"
#include "dialogs/graphet_parent.h"
#include "dialogs/clickimageevent.h"
#include "util/files_folders_operations.h"
#include "util/vector_operations.h"
#include "registration/multiPOC_Ai1.h"
#include "shared_staff/sharedvariables.h"

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
#include <QRegExp>

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
    ui->standardCutout->setEnabled(false);
    ui->standardCutout->setText(tr("Modify standard cutout"));
    ui->extraCutout->setEnabled(false);
    ui->extraCutout->setText(tr("Modify extra cutout"));
    ui->areaMaximum->setPlaceholderText("0-20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationCount->setPlaceholderText("1-Inf; -1~auto");
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerationlabel->setText(tr("Maximal tolerated rotation angle"));
    ui->numberIterationlabel->setText(tr("Number of iterations of algorithm"));
    ui->afewVideosPB->setText(tr("Choose few files"));
    ui->wholeFolderPB->setText(tr("Choose whole folder"));
    ui->analyzeVideosPB->setText(tr("Analyse videos"));
    ui->analyzeVideosPB->setEnabled(false);
    ui->previousThresholdsCB->setText(tr("Use previous thresholds"));
    ui->previousThresholdsCB->setEnabled(false);

    localErrorDialogHandling[ui->analyzeVideosPB] = new ErrorDialog(ui->analyzeVideosPB);

    QObject::connect(this,SIGNAL(checkValuesPass()),this,SLOT(evaluateCorrectValues()));
    QObject::connect(ui->standardCutout,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(ui->extraCutout,SIGNAL(stateChanged(int)),this,SLOT(showDialog()));
    QObject::connect(this,SIGNAL(calculationStarted()),this,SLOT(disableWidgets()));
    QObject::connect(this,SIGNAL(calculationStopped()),this,SLOT(enableWidgets()));

    ui->selectedVideos->setRowCount(1);
    ui->selectedVideos->setColumnCount(3);
    QStringList columnHeadersList = {tr("Video"),tr("Status"),tr("Data")};
    //ui->selectedVideos->setHorizontalHeaderLabels(columnHeadersList);
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
       QStringList _list;
       foreach (QUrl url,urls){
           QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
           if (mime.inherits("video/x-msvideo")) {
               QString _video = url.toLocalFile();
                _list.append(_video);
           }
       }
       if (!_list.isEmpty()) {
           foreach(QString video,_list) {
               analysedVideos.append(video);
           }
           analysedVideos.removeDuplicates();
           fillTable(true);
       }
       qDebug()<<"Actual list of videos contains: "<<analysedVideos;
}

void MultipleVideoET::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MultipleVideoET::on_afewVideosPB_clicked()
{
    QStringList _list = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),
                            SharedVariables::getSharedVariables()->getPath("videosPath"),
                            tr("Video files (*.avi);;;") );
    if (!_list.isEmpty()) {
        foreach(QString video,_list) {
            analysedVideos.append(video);
        }
        analysedVideos.removeDuplicates();
        fillTable(true);
    }
}

void MultipleVideoET::on_wholeFolderPB_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                  SharedVariables::getSharedVariables()->getPath("videosPath"),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir chosenDirectory(dir);
    QStringList _list = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
    if (!_list.isEmpty()) {
        foreach(QString video,_list) {
            analysedVideos.append(video);
        }
        analysedVideos.removeDuplicates();
        fillTable(true);
    }
}

bool MultipleVideoET::checkFileAndLoadThresholds(QString i_videoName) {
    QVector<double> videoETthresholds;
    bool _returnResult = false;
    if (checkAndLoadData("thresholds",i_videoName,videoETthresholds)){
        temporalySavedThresholds.insert(i_videoName,videoETthresholds);
        ui->previousThresholdsCB->setEnabled(true);
        ETthresholdsFound.insert(i_videoName,true);
        _returnResult = true;
    }
    else{
        ETthresholdsFound.insert(i_videoName,false);
    }
    return _returnResult;
}

bool MultipleVideoET::checkVideo(QString i_video){
    bool _returnResult = false;
    cv::VideoCapture cap = cv::VideoCapture(i_video.toLocal8Bit().constData());
    if (cap.isOpened()){
        _returnResult = true;
    }
    return _returnResult;
}

QLabel* MultipleVideoET::createIconTableItem(QString i_icon) {
    QLabel *lbl_item = new QLabel();
    lbl_item->setPixmap(QPixmap(":/images/"+i_icon));
    lbl_item ->setAlignment(Qt::AlignHCenter);
    return lbl_item;
}

void MultipleVideoET::fillTable(bool fillInternalVariables) {
    int _helperCounter = 0;
    bool _controlCheckToEnableElements = false;
    bool overallSCutout=false,overallECutout=false;
    if (!analysedVideos.isEmpty()) {
        ui->selectedVideos->setRowCount(analysedVideos.count());
        for (int indexList = 0; indexList < analysedVideos.count();indexList++)
        {
            readyToProcess[analysedVideos.at(indexList)] = false;
            if (checkVideo(analysedVideos.at(indexList))) {
                _controlCheckToEnableElements = true;
                readyToProcess[analysedVideos.at(indexList)] = true;

                QString folder,filename,suffix;
                if (fillInternalVariables) {
                    processFilePath(analysedVideos.at(indexList),folder,filename,suffix);
                    if (!videoNamesList.contains(filename))
                        videoNamesList.append(filename);
                    ui->selectedVideos->setItem(_helperCounter,0,new QTableWidgetItem(filename));
                }
                else
                    ui->selectedVideos->setItem(_helperCounter,0,new QTableWidgetItem(videoNamesList.at(indexList)));
                bool cutoutEFound=false,cutoutSFound=false;
                ui->selectedVideos->setCellWidget(_helperCounter,1,createIconTableItem("everythingOK.png"));
                if (checkFileAndLoadThresholds(fillInternalVariables ? filename : videoNamesList.at(indexList))) {
                    //if (fillInternalVariables) {
                        QVector<int> _tempStandard,_tempExtra;
                        if (checkAndLoadData("standard",fillInternalVariables ? filename : videoNamesList.at(indexList),_tempStandard)) {
                            cv::Rect _tempStandardRect = convertVector2Rect(_tempStandard);
                            SharedVariables::getSharedVariables()->setVideoInformation(fillInternalVariables ? filename : videoNamesList.at(indexList),"standard",convertRectToQRect(_tempStandardRect));
                            ui->selectedVideos->setCellWidget(_helperCounter,2,createIconTableItem("dataExistsComplete.png"));
                            cutoutSFound=true;standardLoaded=true;overallSCutout=true;
                            if (!ui->standardCutout->isChecked())
                                ui->standardCutout->setChecked(true);
                            selectedCutout=cutoutType::STANDARD;
                        }                        
                        if (checkAndLoadData("extra",fillInternalVariables ? filename : videoNamesList.at(indexList),_tempExtra)) {
                            cv::Rect _tempExtraRect = convertVector2Rect(_tempExtra);
                            SharedVariables::getSharedVariables()->setVideoInformation(fillInternalVariables ? filename : videoNamesList.at(indexList),"extra",convertRectToQRect(_tempExtraRect));
                            ui->selectedVideos->setCellWidget(_helperCounter,2,createIconTableItem("dataExistsComplete.png"));
                            cutoutEFound=true;extraLoaded=true;overallECutout=true;
                            if (!ui->extraCutout->isChecked())
                                ui->extraCutout->setChecked(true);
                            selectedCutout=cutoutType::EXTRA;
                        }

                        if (!cutoutEFound && !cutoutSFound)
                            ui->selectedVideos->setCellWidget(_helperCounter,2,createIconTableItem("dataExists.png"));
                    /*}
                    else
                        ui->selectedVideos->setCellWidget(_helperCounter,2,createIconTableItem("dataExists.png"));*/
                }
                else {
                    ui->selectedVideos->setCellWidget(_helperCounter,2,createIconTableItem("dataMissing.png"));
                }
                _helperCounter++;
            }
            else {
                ui->selectedVideos->setItem(_helperCounter,0,new QTableWidgetItem(analysedVideos.at(indexList)));
                ui->selectedVideos->setCellWidget(_helperCounter,1,createIconTableItem("everythingBad.png"));
            }
        }

        if (!overallSCutout && ui->standardCutout->isChecked())
            ui->standardCutout->setChecked(false);
        if (!overallECutout && ui->extraCutout->isChecked())
            ui->extraCutout->setChecked(false);

        ui->selectedVideos->setColumnWidth(0,(ui->selectedVideos->width()/5)*3);
        ui->selectedVideos->setColumnWidth(1,(ui->selectedVideos->width()/5));
        ui->selectedVideos->setColumnWidth(2,(ui->selectedVideos->width()/5));
        if (_controlCheckToEnableElements) {
            inputWidgetsEnabledStatus(true);
        }
        else {
            inputWidgetsEnabledStatus(false);
        }
    }
    else {
        localErrorDialogHandling[ui->analyzeVideosPB]->evaluate("center","softError",3);
        localErrorDialogHandling[ui->analyzeVideosPB]->show(false);
    }
}

void MultipleVideoET::on_analyzeVideosPB_clicked()
{
    if (runStatus){
        First[1] = new qThreadFirstPart(analysedVideos,
                                        selectedCutout,
                                        temporalySavedThresholds,
                                        ETthresholdsFound,
                                        ui->previousThresholdsCB->isChecked());
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
    else{
        cancelAllCalculations();
        ui->analyzeVideosPB->setText(tr("Analyse videos"));
        ui->computationProgress->setValue(0);
        ui->actualMethod_label->setText("");
        ui->actualVideo_label->setText("");
        canProceed = false;
        runStatus = true;
    }
}

void MultipleVideoET::on_showResultsPB_clicked()
{
    QStringList inputVector;
    for (int a=0; a<analysedVideos.count(); a++)
    {
        QString fullPath = analysedVideos.at(a);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        inputVector.append(jmeno);
    }
    GraphET_parent* graph = new GraphET_parent(analysedVideos,
                                               mapDouble["entropy"],
                                               mapDouble["tennengrad"],
                                               mapDouble["thresholds"],
                                               mapInt["firstEvalEntropy"],
                                               mapInt["firstEvalTennengrad"],
                                               mapInt["firstEval"],
                                               mapInt["secondEval"],
                                               mapInt["evaluation"]);
    connect(graph,SIGNAL(saveCalculatedData(QString,QJsonObject)),this,SLOT(onSaveFromGraphET(QString,QJsonObject)));
    graph->setModal(true);
    graph->show();
}

void MultipleVideoET::onSaveFromGraphET(QString i_videoName, QJsonObject i_object){
    saveVideoAnalysisResultsFromGraphET(i_videoName, i_object);
}

void MultipleVideoET::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
        case Qt::Key_Delete:
            deleteSelectedFiles();
            break;
         default:
            QWidget::keyPressEvent(event);
    }
}

void MultipleVideoET::deleteSelectedFiles(){
    QItemSelectionModel *selection = ui->selectedVideos->selectionModel();
    QModelIndexList _list = selection->selectedRows();
    if (selection->hasSelection()) {
        foreach (QModelIndex index, _list) {
            const QAbstractItemModel* _model = index.model();
            QString selectedVideo = _model->data(index,0).toString();
            int indexOfVideo = videoNamesList.indexOf(selectedVideo);
            videoNamesList.removeAt(indexOfVideo);
            QStringList selectedVideoFull = analysedVideos.filter(selectedVideo);
            indexOfVideo = analysedVideos.indexOf(selectedVideoFull.at(0));
            analysedVideos.removeAt(indexOfVideo);
            readyToProcess.remove(selectedVideoFull.at(0));
        }
        ui->selectedVideos->clearContents();
        if (!analysedVideos.isEmpty())
            fillTable(false);
        else {
            ui->standardCutout->setEnabled(false);
            ui->extraCutout->setEnabled(false);
            ui->savePB->setEnabled(false);
            ui->showResultsPB->setEnabled(false);
            ui->analyzeVideosPB->setEnabled(false);
            if (ui->standardCutout->isChecked())
                ui->standardCutout->setChecked(false);
            if (ui->extraCutout->isChecked())
                ui->extraCutout->setChecked(false);
            standardLoaded=false;extraLoaded=false;
        }
    }
    if (videoNamesList.isEmpty()) {
        ui->areaMaximum->clear();
        ui->rotationAngle->clear();
        ui->iterationCount->clear();
    }
    qDebug()<<videoNamesList;
    qDebug()<<analysedVideos;
}

void MultipleVideoET::on_savePB_clicked()
{
    saveVideoAnalysisResults();
}

void MultipleVideoET::onDone(int thread){
    done(thread);
    if (thread == 1){
        qDebug()<<"First done, starting second...";

        Second[2] = new qThreadSecondPart(analysedVideos,
                                          badVideos,
                                          mapAnomalies["standard"],
                                          mapAnomalies["extra"],
                                          badFramesComplete,
                                          framesReferencial,
                                          selectedCutout,
                                          areaMaximum);
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

        Third[3] = new qThreadThirdPart(analysedVideos,
                                        badVideos,
                                        badFramesComplete,
                                        mapInt["evaluation"],
                                        framesReferencial,
                                        averageCCcomplete,
                                        averageFWHMcomplete,
                                        mapAnomalies["standard"],
                                        mapAnomalies["extra"],
                                        selectedCutout,
                                        areaMaximum);
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

        Fourth[4] = new qThreadFourthPart(analysedVideos,
                                          badVideos,
                                          mapInt["firstEval"],
                                          mapInt["evaluation"],
                                          CC_problematicFrames,
                                          FWHM_problematicFrames,
                                          mapDouble["POCX"],
                                          mapDouble["POCY"],
                                          mapDouble["angle"],
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

        Fifth[5] = new qThreadFifthPart(analysedVideos,
                                        badVideos,
                                        mapAnomalies["standard"],
                                        mapAnomalies["extra"],
                                        mapDouble["POCX"],
                                        mapDouble["POCY"],
                                        mapDouble["angle"],
                                        mapDouble["FrangiX"],
                                        mapDouble["FrangiY"],
                                        mapDouble["FrangiEuklid"],
                                        selectedCutout,
                                        mapInt["evaluation"],
                                        mapInt["secondEval"],
                                        framesReferencial,
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
        ui->analyzeVideosPB->setText(tr("Analyse videos"));
        emit calculationStopped();
        runStatus = true;
        First.clear();
        Second.clear();
        Third.clear();
        Fourth.clear();
        Fifth.clear();
    }
}

void MultipleVideoET::onUnexpectedTermination(int videoIndex, QString errorType){
    Q_UNUSED(videoIndex)
    localErrorDialogHandling[ui->analyzeVideosPB]->evaluate("left",errorType,"Video could not be analysed.");
    localErrorDialogHandling[ui->analyzeVideosPB]->show(false);
    if (errorType == "hardError"){
        cancelAllCalculations();
        emit calculationStopped();
    }
}

void MultipleVideoET::newVideoProcessed(int index)
{
    ui->actualVideo_label->setText("Analysing: "+analysedVideos.at(index)+" ("+QString::number(index+1)+"/"+QString::number(analysedVideos.size())+")");
}

void MultipleVideoET::movedToMethod(int metoda)
{
    if (metoda == 0)
        ui->actualMethod_label->setText(tr("1/5 Entropy and tennengrad computation"));
    if (metoda == 1)
        ui->actualMethod_label->setText(tr("2/5 Average correlation and FWHM"));
    if (metoda == 2)
        ui->actualMethod_label->setText(tr("3/5 First decision algorithm started"));
    if (metoda == 3)
        ui->actualMethod_label->setText(tr("4/5 Second decision algorithm started"));
    if (metoda == 4)
        ui->actualMethod_label->setText(tr("5/5 Third decision algorithm started"));
}

void MultipleVideoET::evaluateCorrectValues(){
    if (areaMaximumCorrect && rotationAngleCorrect && iterationCountCorrect){
        ui->analyzeVideosPB->setEnabled(true);
        ui->standardCutout->setEnabled(true);
        ui->extraCutout->setEnabled(true);
    }
    else {
        ui->analyzeVideosPB->setEnabled(false);
        ui->standardCutout->setEnabled(false);
        ui->extraCutout->setEnabled(false);
    }
}

void MultipleVideoET::inputWidgetsEnabledStatus(bool status) {
    ui->areaMaximum->setEnabled(status);
    ui->iterationCount->setEnabled(status);
    ui->rotationAngle->setEnabled(status);
}

void MultipleVideoET::on_areaMaximum_editingFinished()
{
    bool ok;
    double input = ui->areaMaximum->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,0.0,20.0,ui->areaMaximum,areaMaximum,areaMaximumCorrect);
        emit checkValuesPass();
    }
}

void MultipleVideoET::on_rotationAngle_editingFinished()
{
    bool ok;
    double input = ui->rotationAngle->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,0.0,0.5,ui->rotationAngle,rotationAngle,rotationAngleCorrect);
        emit checkValuesPass();
    }
}

void MultipleVideoET::on_iterationCount_editingFinished()
{
    bool ok;
    double input = ui->iterationCount->text().toDouble(&ok);
    if (ok){
        checkInputNumber(input,-1.0,0.0,ui->iterationCount,iterationCount,iterationCountCorrect);
        emit checkValuesPass();
    }
}

void MultipleVideoET::showDialog(){
    if (QObject::sender() == ui->standardCutout && !standardLoaded) {
        if (ui->standardCutout->isChecked())
        {
            ClickImageEvent* markAnomaly = new ClickImageEvent(analysedVideos,cutoutType::STANDARD);
            markAnomaly->setModal(true);
            markAnomaly->show();
            connect(markAnomaly,&QDialog::finished,[=](){
                checkSelectedCutout(cutoutType::STANDARD);
            });
        }
    }
    else if (QObject::sender() == ui->standardCutout && !ui->standardCutout->isChecked())
        standardLoaded = false;
    else if (QObject::sender() == ui->extraCutout && !extraLoaded) {
        if (ui->extraCutout->isChecked())
        {
            ClickImageEvent* markAnomaly = new ClickImageEvent(analysedVideos,cutoutType::EXTRA);
            markAnomaly->setModal(true);
            markAnomaly->show();
            connect(markAnomaly,&QDialog::finished,[=](){
                checkSelectedCutout(cutoutType::EXTRA);
            });
        }
    }
    else if (QObject::sender() == ui->extraCutout && !ui->extraCutout->isChecked()) {
        extraLoaded = false;
    }
}

void MultipleVideoET::checkSelectedCutout(cutoutType i_type) {
    if (i_type == cutoutType::EXTRA) {
        int controlCount=0;
        foreach (QString video,videoNamesList) {
            if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(video)) {
                QRect __extra = SharedVariables::getSharedVariables()->getVideoInformation(video,"extra").toRect();
                if (!__extra.isNull() && __extra.width() > 0 && __extra.height() > 0) {
                    controlCount++;
                }
            }
        }
        if (controlCount <1) {
            ui->extraCutout->setChecked(false);
            if (standardLoaded)
                selectedCutout = cutoutType::STANDARD;
        }
        else {
            selectedCutout = cutoutType::EXTRA;
            if (!standardLoaded && !ui->standardCutout->isChecked()) {
                bool previous = ui->standardCutout->blockSignals(true);
                ui->standardCutout->setChecked(true);
                ui->standardCutout->blockSignals(previous);
            }
        }
    }
    else {
        int controlCount=0;
        foreach (QString video,videoNamesList) {
            if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(video)) {
                QRect __standard = SharedVariables::getSharedVariables()->getVideoInformation(video,"standard").toRect();
                if (!__standard.isNull() && __standard.width() > 0 && __standard.height() > 0) {
                    controlCount++;
                }
            }
        }
        if (controlCount <1) {
            ui->standardCutout->setChecked(false);
        }
        else {
            selectedCutout = cutoutType::STANDARD;
        }
    }
}

void MultipleVideoET::disableWidgets(){
    ui->wholeFolderPB->setEnabled(false);
    ui->afewVideosPB->setEnabled(false);
    ui->standardCutout->setEnabled(false);
    ui->extraCutout->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->showResultsPB->setEnabled(false);
    ui->areaMaximum->setEnabled(false);
    ui->iterationCount->setEnabled(false);
    ui->rotationAngle->setEnabled(false);
}

void MultipleVideoET::enableWidgets(){
    ui->wholeFolderPB->setEnabled(true);
    ui->afewVideosPB->setEnabled(true);
    ui->standardCutout->setEnabled(true);
    ui->extraCutout->setEnabled(true);
    ui->savePB->setEnabled(true);
    ui->showResultsPB->setEnabled(true);
    ui->areaMaximum->setEnabled(true);
    ui->iterationCount->setEnabled(true);
    ui->rotationAngle->setEnabled(true);
}
