#include "dialogs/clickimageevent.h"
#include "ui_clickimageevent.h"
#include "image_analysis/image_processing.h"
#include "util/files_folders_operations.h"

#include <QGraphicsView>
#include <QPoint>
#include <QImage>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QDebug>
#include <QThread>
#include <QJsonObject>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include "opencv2/imgproc/types_c.h"

ClickImageEvent::ClickImageEvent(QString i_fullPath,
                                 cutoutType i_cutoutType,
                                 bool i_referentialFrameFound,
                                 QDialog *parent):
    QDialog (parent),ui(new Ui::ClickImageEvent)
{

    ui->setupUi(this);
    QString suffix,folder;
    filePath = i_fullPath;
    processFilePath(i_fullPath,folder,videoName,suffix);

    cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);

    cutout = i_cutoutType;
    referencialFrameNo[videoName] = -1;
    frangiCoordinates[videoName] = QPoint(-999,-999);

    ui->clickImage->setMouseTracking(true);
    ui->clickImage->setEnabled(false);
    ui->clickImage->viewport()->installEventFilter(this);

    initStandardVideoWidgets(videoCount::ONE_VIDEO,i_cutoutType,i_referentialFrameFound);
}

ClickImageEvent::ClickImageEvent(QString i_fullPath,
                                 int i_referFrameNo,
                                 cutoutType i_cutoutType,
                                 bool i_referentialFrameFound,
                                 chosenSource i_chosenType,
                                 QDialog *parent) :
    QDialog (parent),ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    QString suffix,folder;
    filePath = i_fullPath;
    processFilePath(i_fullPath,folder,videoName,suffix);

    cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);

    cutout = i_cutoutType;
    referencialFrameNo[videoName] = i_referFrameNo;
    frangiCoordinates[videoName] = QPoint(-999,-999);

    ui->clickImage->setMouseTracking(true);
    ui->clickImage->setEnabled(false);
    ui->clickImage->viewport()->installEventFilter(this);

    if (i_chosenType == chosenSource::VIDEO)
        initStandardVideoWidgets(videoCount::ONE_VIDEO,i_cutoutType,i_referentialFrameFound);
    else if (i_chosenType == chosenSource::IMAGE)
        initStandardImageWidgets(i_cutoutType);
    whatIsAnalysed = i_chosenType;
}

ClickImageEvent::ClickImageEvent(QStringList i_fullPaths, cutoutType i_type, QDialog *parent) :
    QDialog(parent),ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    filePaths = i_fullPaths;
    for (int index=0;index<i_fullPaths.count();index++){
        QString folder,suffix,_videoName;
        processFilePath(i_fullPaths.at(index),folder,_videoName,suffix);
        videoNames.append(_videoName);
    }
    videoName = videoNames.at(0);
    cap = cv::VideoCapture(filePaths.at(0).toLocal8Bit().constData());
    frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    cutout = i_type;
    referencialFrameNo[videoName] = -1;
    frangiCoordinates[videoName] = QPoint(-999,-999);

    ui->clickImage->setMouseTracking(false);
    ui->clickImage->setEnabled(false);
    ui->clickImage->viewport()->installEventFilter(this);

    initStandardVideoWidgets(videoCount::MULTIPLE_VIDEOS,i_type,false);
}

ClickImageEvent::~ClickImageEvent()
{
    delete ui;
    delete imageObject;
    delete scene;
    delete pathItem;
    delete frangiCoords;
    delete standardCutout_GRI;
    delete extraCutout_GRI;
    delete videoSelection;
    delete referentialFrameNumber;
    delete chosenVideo;
    delete chosenCutout;
    delete revealFrangiOptions;
    delete _integratedFrangiOptionsObject;
}

void ClickImageEvent::initStandardVideoWidgets(videoCount i_count, cutoutType i_cutout,
                                          bool i_loadFrangiCoordinates){
    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    actualVideoCount = i_count;
    // exists everytime
    referentialFrameNumber = new QLineEdit();
    ui->optionalContent->addWidget(referentialFrameNumber,1);
    QObject::connect(referentialFrameNumber,SIGNAL(returnPressed()),this,SLOT(referencialFrameChosen()));
    localErrorDialogHandling[referentialFrameNumber] = new ErrorDialog(referentialFrameNumber);

    _integratedFrangiOptionsObject = new IntegratedFrangiOptions;
    _integratedFrangiOptionsObject->setVisible(false);
    QMap<QString,double> _r = SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(frangiType::VIDEO_SPECIFIC,videoName);
    QMap<QString,int> _m = SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(frangiType::VIDEO_SPECIFIC,videoName);
    QMap<QString,double> _p = SharedVariables::getSharedVariables()->getFrangiParameterWrapper(frangiType::VIDEO_SPECIFIC,videoName);
    _integratedFrangiOptionsObject->GetSetValues("SET",_m,_r,_p);

    // standard or extra cutout
    chosenCutout = new QLabel();
    if (i_cutout == cutoutType::STANDARD){
        cutoutFunctionPointer = &ClickImageEvent::updateCutoutStandard;
        chosenCutout->setText(tr("Standard cutout update"));
        cutoutUpdate = updateType::UPDATE_DIRECTLY;
    }
    else{
        cutoutFunctionPointer = &ClickImageEvent::updateCutoutExtra;
        chosenCutout->setText(tr("Extra cutout update"));
        cutoutUpdate = updateType::RECALCULATE;
    }
    ui->optionalContent->addWidget(chosenCutout,1);

    // one video or multiple videos
    if (i_count == videoCount::ONE_VIDEO){
        chosenVideo = new QLabel();
        chosenVideo->setText(videoName);
        ui->optionalContent->addWidget(chosenVideo,10);

        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(videoName) &&
                (SharedVariables::getSharedVariables()->getVideoInformation(videoName,"frame").toInt() == referencialFrameNo[videoName])){
            // first possible situation - video was analysed previously and the program was not closed -> necessary
            // data is stored temporaly in the SharedViariables class
            frangiCoordinates[videoName] = SharedVariables::getSharedVariables()->getVideoInformation(videoName,"frangi").toPoint();
            standardCutout[videoName] = SharedVariables::getSharedVariables()->getVideoInformation(videoName,"standard").toRect();
            extraCutout[videoName] = SharedVariables::getSharedVariables()->getVideoInformation(videoName,"extra").toRect();
            fillGraphicScene(false);
        }
        else if (referencialFrameNo[videoName] != -1){
            // one video with known reference -> start processing
            referentialFrameNumber->setText(QString::number(referencialFrameNo[videoName]+1));
            referentialFrameNumber->setEnabled(false);
            // check if the *.dat file for selected video and referential frame info exist
            // we can check, if the frangi coordinates are present in the file
            if (i_loadFrangiCoordinates){
                if (findReferentialFrameData(videoName,referencialFrameNo[videoName],frangiCoordinates[videoName])){
                    // dat file for video exists, referential frame record was found -> setup scene
                    fillGraphicScene(true);
                }
                else{
                    // file not found or frangi info is missign -> start frangi analysis
                    startFrangiAnalysis();
                }
            }
            else
                startFrangiAnalysis();
        }
        else{// referential frame is unknown -> try to find it
            if (findReferentialFrameData(videoName,referencialFrameNo[videoName],frangiCoordinates[videoName])){
                // success -> setup scene
                referentialFrameNumber->setText(QString::number(referencialFrameNo[videoName]+1));
                referentialFrameNumber->setEnabled(false);
                fillGraphicScene(true);
            }            
            else{
                // file does not exist -> ask to add referential frame No to start frangi analysis
                referentialFrameNumber->setEnabled(true);
                referentialFrameNumber->setPlaceholderText(tr("Ref"));
                localErrorDialogHandling[referentialFrameNumber] = new ErrorDialog(referentialFrameNumber);
                localErrorDialogHandling[referentialFrameNumber]->evaluate("left","whatToDo",1);
                localErrorDialogHandling[referentialFrameNumber]->show(false);
            }
        }        
    }
    else if (i_count == videoCount::MULTIPLE_VIDEOS){
        videoSelection = new QComboBox();
        videoSelection->addItems(videoNames);
        QObject::connect(videoSelection,SIGNAL(currentIndexChanged(int)),this,SLOT(processChosenVideo(int)));
        ui->optionalContent->addWidget(videoSelection,10);
        filePath = filePaths.at(0);videoName = videoNames.at(0);

        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(videoName) &&
                (SharedVariables::getSharedVariables()->getVideoInformation(videoName,"frame").toInt() == referencialFrameNo[videoName])){
            // first possible situation - video was analysed previously in this app run -> all necessary
            // data is stored temporaly in the SharedViariables class
            frangiCoordinates[videoName] = SharedVariables::getSharedVariables()->getVideoInformation(videoName,"frangi").toPoint();
            standardCutout[videoName] = SharedVariables::getSharedVariables()->getVideoInformation(videoName,"standard").toRect();
            extraCutout[videoName] = SharedVariables::getSharedVariables()->getVideoInformation(videoName,"extra").toRect();
            fillGraphicScene(false);
        }
        // blind shot - test if the first video in list exists to start working immediately
        else if (findReferentialFrameData(videoName,referencialFrameNo[videoName],frangiCoordinates[videoName])){
            referentialFrameNumber->setText(QString::number(referencialFrameNo[videoName]+1));
            referentialFrameNumber->setEnabled(false);
            fillGraphicScene(true);
        }
        else{
            // if the first video does not have *.dat file, standard procedure follows
            referentialFrameNumber->setEnabled(true);
            referentialFrameNumber->setPlaceholderText(tr("Ref"));
            localErrorDialogHandling[referentialFrameNumber] = new ErrorDialog(referentialFrameNumber);
            localErrorDialogHandling[referentialFrameNumber]->evaluate("left","whatToDo",2);
            localErrorDialogHandling[referentialFrameNumber]->show(true);            
        }
    }

    // preparing button for custom frangi options
    revealFrangiOptions = new QPushButton;
    QPixmap px(":/images/UP.png");
    QTransform tr;tr.rotate(180);
    revealFrangiOptions->setIcon(px.transformed(tr));
    revealFrangiOptions->setFocusPolicy(Qt::NoFocus);
    ui->optionalContent->addWidget(revealFrangiOptions,1);
    connect(revealFrangiOptions,SIGNAL(clicked()),this,SLOT(onShowFrangiOptions()));

    ui->layout->addWidget(_integratedFrangiOptionsObject);

    connect(_integratedFrangiOptionsObject,
            SIGNAL(frangiParametersSelected(QMap<QString,int>,QMap<QString,double>,QMap<QString,double>)),
            this,
            SLOT(saveSelectedParameters(QMap<QString,int>,QMap<QString,double>,QMap<QString,double>)));
    connect(_integratedFrangiOptionsObject,
            SIGNAL(applyFrangiParameters()),this,SLOT(onApplyFrangiParameters()));
}

void ClickImageEvent::initStandardImageWidgets(cutoutType i_cutout) {
    // standard or extra cutout
    chosenCutout = new QLabel();
    if (i_cutout == cutoutType::STANDARD){
        cutoutFunctionPointer = &ClickImageEvent::updateCutoutStandard;
        chosenCutout->setText(tr("Standard cutout update"));
        cutoutUpdate = updateType::UPDATE_DIRECTLY;
    }
    else{
        cutoutFunctionPointer = &ClickImageEvent::updateCutoutExtra;
        chosenCutout->setText(tr("Extra cutout update"));
        cutoutUpdate = updateType::RECALCULATE;
    }
    ui->optionalContent->addWidget(chosenCutout,10);

    // preparing button for custom frangi options
    revealFrangiOptions = new QPushButton;
    QPixmap px(":/images/UP.png");
    QTransform tr;tr.rotate(180);
    revealFrangiOptions->setIcon(px.transformed(tr));
    revealFrangiOptions->setDefault(false);
    revealFrangiOptions->setAutoDefault(false);
    ui->optionalContent->addWidget(revealFrangiOptions,1);
    connect(revealFrangiOptions,SIGNAL(clicked()),this,SLOT(onShowFrangiOptions()));

    _integratedFrangiOptionsObject = new IntegratedFrangiOptions;
    //_integratedFrangiOptionsObject->setParent(this);
    ui->layout->addWidget(_integratedFrangiOptionsObject);
    _integratedFrangiOptionsObject->setVisible(false);
    QMap<QString,double> _r = SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(frangiType::GLOBAL,"");
    QMap<QString,int> _m = SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(frangiType::GLOBAL,"");
    QMap<QString,double> _p = SharedVariables::getSharedVariables()->getFrangiParameterWrapper(frangiType::GLOBAL,"");
    _integratedFrangiOptionsObject->GetSetValues("SET",_m,_r,_p);

    connect(_integratedFrangiOptionsObject,
            SIGNAL(frangiParametersSelected(QMap<QString,int>,QMap<QString,double>,QMap<QString,double>)),
            this,
            SLOT(saveSelectedParameters(QMap<QString,int>,QMap<QString,double>,QMap<QString,double>)));
    connect(_integratedFrangiOptionsObject,
            SIGNAL(applyFrangiParameters()),this,SLOT(onApplyFrangiParameters()));

    startFrangiAnalysis();
}

void ClickImageEvent::onApplyFrangiParameters() {
    startFrangiAnalysis();
}

void ClickImageEvent::startFrangiAnalysis(){
    cv::Mat referencialImage;
    if (whatIsAnalysed == chosenSource::VIDEO) {
        if (!cap.isOpened())
            cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
        cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrameNo[videoName]);
        cap.read(referencialImage);        
    }
    else if (whatIsAnalysed == chosenSource::IMAGE) {
        referencialImage = cv::imread(filePath.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
    }
    transformMatTypeTo8C3(referencialImage);
    QMap<QString,double> _r,_p;
    QMap<QString,int> _m;
    _integratedFrangiOptionsObject->GetSetValues("GET",_m,_r,_p);
    qDebug()<<"Providing parameters: "<<_m<<" "<<_p<<" "<<_r;
    FrangiThread* frangiAnalyzer = new FrangiThread(referencialImage,
                                      1,1,0,"",1,cv::Point3d(0.0,0.0,0.0),
                                      _p,_m);
    QThread* thread = new QThread;
    threadPool.insert(0,thread);
    frangiAnalyzer->moveToThread(threadPool[0]);

    connect(threadPool[0],SIGNAL(started()),frangiAnalyzer,SLOT(startFrangiAnalysis()));
    connect(frangiAnalyzer,SIGNAL(finishedCalculation(QPoint)),
            this,SLOT(obtainFrangiCoordinates(QPoint)));
    connect(this,SIGNAL(dataExtracted()),frangiAnalyzer,SLOT(deleteLater()));
    connect(this,SIGNAL(dataExtracted()),this,SLOT(onDataExtracted()));

    ProcessingIndicator* _processingIndicator = new ProcessingIndicator(tr("Calculating Frangi coordinates"),
                                                                        ui->clickImage);
    _processingIndicator->placeIndicator();
    _processingIndicator->showIndicator();

    connect(frangiAnalyzer,SIGNAL(finishedCalculation(QPoint)),
                    _processingIndicator,SLOT(hideIndicatorSlot()));

    thread->start();

    ui->clickImage->setEnabled(false);
    ui->clickImage->setMouseTracking(false);
}

void ClickImageEvent::onDataExtracted(){
    if (threadPool.contains(0)) {
        if (threadPool[0]->isRunning()){
            threadPool[0]->terminate();
            threadPool[0]->wait(10);
            threadPool[0]->deleteLater();
        }
        else
            threadPool[0]->deleteLater();
        threadPool.clear();
    }
}

void ClickImageEvent::obtainFrangiCoordinates(QPoint i_calculatedData){
    qDebug()<<"Obtaining calculated Frangi: "<<i_calculatedData.x()<<" "<<i_calculatedData.y();
    emit dataExtracted();
    drawGraphicsScene(i_calculatedData);
}

void ClickImageEvent::drawGraphicsScene(QPoint i_result){
    if (i_result.x() == -999 && i_result.y() == -999){
        localErrorDialogHandling[referentialFrameNumber]->evaluate("left","hardError",14);
        localErrorDialogHandling[referentialFrameNumber]->show(false);
    }
     else{
        frangiCoordinates[videoName] = i_result;
        fillGraphicScene(true);
    }
}

/*void ClickImageEvent::setWindowSize(int width, int height)
{
    this->setFixedSize(width,height);
}*/

void ClickImageEvent::closeEvent(QCloseEvent *e) {
    if (modified && whatIsAnalysed == chosenSource::VIDEO) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText(tr("At least one cutout was modified. Do you want to apply these changes for the analysis?"));
        int response = msgBox.exec();
        switch (response) {
          case QMessageBox::Yes:
            saveCutouts(true);
          break;
          case QMessageBox::No:
            saveCutouts(false);
          break;
          default:
            saveCutouts(false);
          break;
        }
    }
    else
        saveCutouts(false);
    e->accept();
}

void ClickImageEvent::saveCutouts(bool saveNew) {
    if (whatIsAnalysed == chosenSource::VIDEO && actualVideoCount == videoCount::ONE_VIDEO) {
        SharedVariables::getSharedVariables()->setVideoInformation(videoName,"frame",referencialFrameNo[videoName]);
        SharedVariables::getSharedVariables()->setVideoInformation(videoName,"frangi",frangiCoordinates[videoName]);
        if (cutout == cutoutType::STANDARD)
            SharedVariables::getSharedVariables()->setVideoInformation(videoName,"standard",saveNew ? standardCutout[videoName] : originalStandardCutout[videoName]);
        else if (cutout == cutoutType::EXTRA) {
            SharedVariables::getSharedVariables()->setVideoInformation(videoName,"extra",saveNew ? extraCutout[videoName] : originalExtraCutout[videoName]);
            SharedVariables::getSharedVariables()->setVideoInformation(videoName,"standard",standardCutout[videoName]);
        }
        qDebug()<<SharedVariables::getSharedVariables()->getCompleteVideoInformation(videoName);
    }
    else if (whatIsAnalysed == chosenSource::VIDEO && actualVideoCount == videoCount::MULTIPLE_VIDEOS) {
        QList<QString> videoNames = referencialFrameNo.keys();
        foreach (QString name, videoNames) {
            SharedVariables::getSharedVariables()->setVideoInformation(name,"frame",referencialFrameNo[name]);
            SharedVariables::getSharedVariables()->setVideoInformation(name,"frangi",frangiCoordinates[name]);
            if (cutout == cutoutType::STANDARD)
                SharedVariables::getSharedVariables()->setVideoInformation(name,"standard",saveNew ? standardCutout[name] : originalStandardCutout[name]);
            else if (cutout == cutoutType::EXTRA) {
                SharedVariables::getSharedVariables()->setVideoInformation(name,"extra",saveNew ? extraCutout[name] : originalExtraCutout[name]);
                SharedVariables::getSharedVariables()->setVideoInformation(name,"standard",standardCutout[name]);
            }
        }
    }
    else if (whatIsAnalysed == chosenSource::IMAGE && actualVideoCount == videoCount::NO_VIDEO) {
        //emit saveImageCutouts();
    }
}

bool ClickImageEvent::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent* mousePress = dynamic_cast<QMouseEvent*>(event);
        mousePressEvent(mousePress);
    }
    else if (event->type() == QEvent::MouseMove){
        QMouseEvent* mouseMove = dynamic_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseMove);
    }
    else if (event->type() == QEvent::MouseButtonRelease){
        QMouseEvent *mouseRelease = dynamic_cast<QMouseEvent*>(event);
        mouseReleaseEvent(mouseRelease);
    }
    else
        event->ignore();
    return false;
}

void ClickImageEvent::mouseMoveEvent(QMouseEvent *move){
    if (mousePressed && !disabled){
        lastDragPosition = ui->clickImage->mapToScene(move->pos());
        //qDebug()<<"Lower right: "<<ui->clickImage->mapToScene(move->pos());
        cutoutFunctionPointer(this);
        move->accept();
        return;
    }
    else move->ignore();
}

void ClickImageEvent::mousePressEvent(QMouseEvent *press)
{
    if (!disabled) {
        if (press->button()==Qt::LeftButton){
            selectionOrigin = ui->clickImage->mapToScene(press->pos());
            if ((selectionOrigin.x() < 0 || selectionOrigin.y() < 0) ||
                (selectionOrigin.x() > cap.get(CV_CAP_PROP_FRAME_HEIGHT) || selectionOrigin.y() > cap.get(CV_CAP_PROP_FRAME_WIDTH))) {
                press->ignore();
                return;
            }
            else {
                lastSelectionPosition = press->pos();
                mousePressed = true;
            }
        }
        if (cutout == cutoutType::STANDARD){
            tempStandardCutout.setX(int(selectionOrigin.x()));
            tempStandardCutout.setY(int(selectionOrigin.y()));
            tempStandardCutout.setWidth(0.0);
            tempStandardCutout.setHeight(0.0);
            qDebug()<<"Left mouse click for standard cutout: "<<selectionOrigin;
        }
        else if (cutout == cutoutType::EXTRA){
            tempExtraCutout.setX(int(selectionOrigin.x()));
            tempExtraCutout.setY(int(selectionOrigin.y()));
            tempExtraCutout.setWidth(0.0);
            tempExtraCutout.setHeight(0.0);
            qDebug()<<"Left mouse click for extra cutout: "<<selectionOrigin;
        }
        press->accept();
    }
    else press->ignore();
}

void ClickImageEvent::mouseReleaseEvent(QMouseEvent *release){
    if (!disabled) {
        if (mousePressed){
            selectionEnd = ui->clickImage->mapToScene(release->pos());
            qDebug()<<"Released at: "<<selectionEnd;
            mousePressed = false;
        }

        // temp cutouts are modified during mouseMoveEvent
        // on release event, it must be clear, if the new cutout can be saved or not
        // standard or extra cutout can be modified
        if (!checkFrangiMaximumPresenceInCutout())
            revertCutoutChange();
        else{
            qDebug()<<"Results: standard: "<<standardCutout<<" extra: "<<extraCutout;
            modified = true;
        }
        release->accept();
    }
    else release->ignore();
}

bool ClickImageEvent::checkFrangiMaximumPresenceInCutout(){
    if (cutout == cutoutType::STANDARD){
        // if standard cutout is changed, just check the frangi point presence
        if ((frangiCoordinates[videoName].x() > tempStandardCutout.x() && frangiCoordinates[videoName].x() < (tempStandardCutout.x()+tempStandardCutout.width()))
                && (frangiCoordinates[videoName].y() > tempStandardCutout.y() && frangiCoordinates[videoName].y() < (tempStandardCutout.y()+tempStandardCutout.height()))){
            standardCutout[videoName] = tempStandardCutout;
            return true;
        }
        else
            return false;
    }
    else if (cutout == cutoutType::EXTRA){
        // if extra cutout is changed, firstly recalculate standard cutout
        updateCutoutStandard();
        // then check if the standard cutout contains frangi point
        if ((frangiCoordinates[videoName].x() > tempStandardCutout.x() && frangiCoordinates[videoName].x() < (tempStandardCutout.x()+tempStandardCutout.width()))
                && (frangiCoordinates[videoName].y() > tempStandardCutout.y() && frangiCoordinates[videoName].y() < (tempStandardCutout.y()+tempStandardCutout.height()))){
            standardCutout[videoName] = tempStandardCutout;
            extraCutout[videoName] = tempExtraCutout;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

void ClickImageEvent::updateCutoutStandard(){
    //qDebug()<<"Updating standard cutout";

    // there are two possible situations:
    // 1. extra cutout is changing -> standard must be recalculated
    // 2. standard cutout itself is changed -> no recalculation needed

    if (cutoutUpdate == updateType::RECALCULATE){
        // when updating the extraCutout, the frangi point must be inside recalculated standard cutout
        QMap<QString,double> frangiRatios = _integratedFrangiOptionsObject->getInternalRatios();
        int __x = int(round(frangiCoordinates[videoName].x()-frangiRatios["left_r"]*(frangiCoordinates[videoName].x()-selectionOrigin.x())));
        qDebug()<<__x;
        tempStandardCutout.setX(__x);
        int __y = int(round(frangiCoordinates[videoName].y()-frangiRatios["top_r"]*(frangiCoordinates[videoName].y()-selectionOrigin.y())));
        qDebug()<<__y;
        tempStandardCutout.setY(__y);
        int rowTo = int(round(frangiCoordinates[videoName].y()+frangiRatios["bottom_r"]*(selectionEnd.y() - frangiCoordinates[videoName].y())));
        int columnTo = int(round(frangiCoordinates[videoName].x()+frangiRatios["right_r"]*(selectionEnd.x() - frangiCoordinates[videoName].x())));
        qDebug()<<"Recalculating standard cutout: "<<rowTo<<" "<<columnTo;
        tempStandardCutout.setWidth(columnTo-__x);
        tempStandardCutout.setHeight(rowTo - __y);
        scene->removeItem(standardCutout_GRI);
        standardCutout_GRI = scene->addRect(tempStandardCutout,QPen(QColor(255, 0, 0)));
        scene->update();
    }
    else if (cutoutUpdate == updateType::UPDATE_DIRECTLY){
        // standard cutout should be area with the sharpest edges
        // the visibility of the veins should be the best
        // there is the highest probability of frangi maximum existence in this image part
        // it is necessary to check if the frangi maximum is in the new standard cutout area
        double height = lastDragPosition.y() - selectionOrigin.y();
        double width = lastDragPosition.x() - selectionOrigin.x();
        scene->removeItem(standardCutout_GRI);
        tempStandardCutout.setWidth(int(width));
        tempStandardCutout.setHeight(int(height));
        standardCutout_GRI = scene->addRect(tempStandardCutout,QPen(QColor(255, 0, 0)));
        scene->update();
    }
}

void ClickImageEvent::updateCutoutExtra(){
    qDebug()<<"Updating extra cutout";
    double height = lastDragPosition.y() - selectionOrigin.y();
    double width = lastDragPosition.x() - selectionOrigin.x();
    scene->removeItem(extraCutout_GRI);
    tempExtraCutout.setWidth(int(width));
    tempExtraCutout.setHeight(int(height));
    extraCutout_GRI = scene->addRect(tempExtraCutout,QPen(QColor(0, 0, 255)));
    scene->update();
}

void ClickImageEvent::revertCutoutChange(){
    if (cutout == cutoutType::STANDARD){
        scene->removeItem(standardCutout_GRI);
        standardCutout_GRI = scene->addRect(standardCutout[videoName],QPen(QColor(255, 0, 0)));
        scene->update();
    }
    else if (cutout == cutoutType::EXTRA){
        scene->removeItem(extraCutout_GRI);
        extraCutout_GRI = scene->addRect(extraCutout[videoName],QPen(QColor(0, 0, 255)));
        scene->removeItem(standardCutout_GRI);
        standardCutout_GRI = scene->addRect(standardCutout[videoName],QPen(QColor(255, 0, 0)));
        scene->update();
    }
}

void ClickImageEvent::fillGraphicScene(bool i_initCutouts){
    cv::Mat referencialImage;
    if(whatIsAnalysed == chosenSource::VIDEO) {
        if (!cap.isOpened())
            cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
        frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
        cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrameNo[videoName]);
        cap.read(referencialImage);
        width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    }
    else if (whatIsAnalysed == chosenSource::IMAGE) {
        referencialImage = cv::imread(filePath.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
        width = referencialImage.cols;
        height = referencialImage.rows;
    }
    transformMatTypeTo8C3(referencialImage);

    imageObject = new QImage(referencialImage.data,
                             referencialImage.cols,
                             referencialImage.rows,
                             static_cast<int>(referencialImage.step),
                             QImage::Format_RGB888);//

    QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageObject));
    scene = new QGraphicsScene();
    ui->clickImage->setFixedSize(referencialImage.cols,referencialImage.rows);
    ui->clickImage->setScene(scene);
    scene->addItem(image);
    QPainterPath crossPath;
    paintCross(crossPath,frangiCoordinates[videoName].x(),frangiCoordinates[videoName].y());
    pathItem = scene->addPath(crossPath,QPen(QColor(255, 0, 0), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));

    if (i_initCutouts)
        initCutouts(referencialImage);
    originalStandardCutout = standardCutout;
    originalExtraCutout = extraCutout;

    standardCutout_GRI = scene->addRect(standardCutout[videoName],QPen(QColor(255, 0, 0)));
    extraCutout_GRI = scene->addRect(extraCutout[videoName],QPen(QColor(0,0,255)));

    ui->clickImage->setFixedSize(referencialImage.cols+10,referencialImage.rows+10);
    ui->clickImage->setSceneRect(0,0,referencialImage.cols,referencialImage.rows);
    ui->clickImage->setMouseTracking(true);
    ui->clickImage->setEnabled(true);
    disabled = false;
    if (actualVideoCount == videoCount::ONE_VIDEO)
        chosenVideo->setEnabled(true);
    else
        videoSelection->setEnabled(true);

}

void ClickImageEvent::initCutouts(cv::Mat i_inputFrame){
    QMap<QString,double> frangiRatios = _integratedFrangiOptionsObject->getInternalRatios();
    qDebug()<<int(round(frangiCoordinates[videoName].x()-frangiRatios["left_r"]*(frangiCoordinates[videoName].x())));
    standardCutout[videoName].setX(int(round(frangiCoordinates[videoName].x()-frangiRatios["left_r"]*frangiCoordinates[videoName].x())));
    qDebug()<<int(round(frangiCoordinates[videoName].y()-frangiRatios["top_r"]*frangiCoordinates[videoName].y()));
    standardCutout[videoName].setY(int(round(frangiCoordinates[videoName].y()-frangiRatios["top_r"]*frangiCoordinates[videoName].y())));
    int rowTo = int(round(frangiCoordinates[videoName].y()+frangiRatios["bottom_r"]*(i_inputFrame.rows - frangiCoordinates[videoName].y())));
    int columnTo = int(round(frangiCoordinates[videoName].x()+frangiRatios["right_r"]*(i_inputFrame.cols - frangiCoordinates[videoName].x())));
    qDebug()<<rowTo<<" "<<columnTo;
    standardCutout[videoName].setWidth(columnTo-standardCutout[videoName].x());
    standardCutout[videoName].setHeight(rowTo - standardCutout[videoName].y());

    extraCutout[videoName].setX(0);
    extraCutout[videoName].setY(0);
    extraCutout[videoName].setWidth(int(width));
    extraCutout[videoName].setHeight(int(height));
}

void ClickImageEvent::referencialFrameChosen()
{
    bool ok;
    referencialFrameNo[videoName] = referentialFrameNumber->text().toInt(&ok)-1;
    if (ok && (referencialFrameNo[videoName] < 0 || double(referencialFrameNo[videoName]) > frameCount))
    {
        referentialFrameNumber->setStyleSheet("color: #FF0000");
        referencialFrameNo[videoName] = -1;
        ui->clickImage->setEnabled(false);
        ui->clickImage->setMouseTracking(false);
        disabled = true;
    }
    else
    {
        // if MULTIPLE_VIDEOS option is active, this function is used and must therefore
        // use frangi analyser to work properly
        referentialFrameNumber->setStyleSheet("color: #33aa00");
        if (actualVideoCount == videoCount::ONE_VIDEO)
            chosenVideo->setEnabled(false);
        else
            videoSelection->setEnabled(false);
        startFrangiAnalysis();
        if (localErrorDialogHandling[referentialFrameNumber]->isEvaluated())
            localErrorDialogHandling[referentialFrameNumber]->hide();
    }
}

void ClickImageEvent::processChosenVideo(int videoIndex){
    QString chosenVideoPath = filePaths.at(videoIndex);
    videoName = videoNames.at(videoIndex);
    QFile file(chosenVideoPath);
    if (!file.exists()){
        referentialFrameNumber->setEnabled(false);
        cap.release();
        localErrorDialogHandling[referentialFrameNumber]->evaluate("left","hardError",6);
        localErrorDialogHandling[referentialFrameNumber]->show(true);
    }
    else{
        if (findReferentialFrameData(videoNames.at(videoIndex),referencialFrameNo[videoName],frangiCoordinates[videoName])){
            referentialFrameNumber->setText(QString::number(referencialFrameNo[videoName]));
            referentialFrameNumber->setEnabled(false);
            startFrangiAnalysis();
        }
        else{
            cap = cv::VideoCapture(chosenVideoPath.toLocal8Bit().constData());
            referentialFrameNumber->setEnabled(true);
            frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
            filePath = filePaths.at(videoIndex);videoName = videoNames.at(videoIndex);
            bool ok;
            int existiningFrame = referentialFrameNumber->text().toInt(&ok);
            if (ok && existiningFrame > 0 && existiningFrame < frameCount) {
                startFrangiAnalysis();
            }
        }        
    }
}

void ClickImageEvent::paintCross(QPainterPath &path, double x, double y){
    path.moveTo(x, y-10);
    path.lineTo(x, y+10);
    path.moveTo(x-10, y);
    path.lineTo(x+10, y);
}

QPointF ClickImageEvent::loadFrangiMaxCoordinates(){
    cv::Point3d _tempCoordinates = SharedVariables::getSharedVariables()->getFrangiMaximumWrapper(frangiType::VIDEO_SPECIFIC,videoName);
    QPointF result(_tempCoordinates.x,_tempCoordinates.y);
    return result;
}

void ClickImageEvent::onShowFrangiOptions() {
    showFrangiOptions();
}

void ClickImageEvent::showFrangiOptions(){

    if (frangiOptionsShown) { // hide
        QPropertyAnimation *animation = new QPropertyAnimation(_integratedFrangiOptionsObject, "maximumHeight");
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InCurve);
        animation->setStartValue(200);
        animation->setEndValue(0);
        animation->start();

        QTimer *timer = new QTimer();
        connect(timer, SIGNAL(timeout()), this, SLOT(onHidden()));
        connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
        connect(timer, SIGNAL(timeout()), animation, SLOT(deleteLater()));
        timer->setSingleShot(true);
        timer->start(300);
    }
    else { // show
        QPropertyAnimation *animation = new QPropertyAnimation(_integratedFrangiOptionsObject, "maximumHeight");
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InCurve);
        qDebug()<<"Height: "<<this->height<<" Width: "<<this->width;
        qDebug()<<"Height: "<<_integratedFrangiOptionsObject->height()<<" Width: "<<_integratedFrangiOptionsObject->width();
        animation->setStartValue(0);
        animation->setEndValue(200);
        animation->start();

        QTimer *timer = new QTimer();
        connect(timer, SIGNAL(timeout()), this, SLOT(onShown()));
        connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
        connect(timer, SIGNAL(timeout()), animation, SLOT(deleteLater()));
        timer->setSingleShot(true);
        timer->start(300);
    }
}

void ClickImageEvent::onShown() {
    _integratedFrangiOptionsObject->setVisible(true);
    QPixmap px(":/images/UP.png");
    revealFrangiOptions->setIcon(px);
    frangiOptionsShown = true;
}

void ClickImageEvent::onHidden() {
    _integratedFrangiOptionsObject->setVisible(false);
    QTransform tr;tr.rotate(180);
    QPixmap px(":/images/UP.png");
    revealFrangiOptions->setIcon(px.transformed(tr));
    frangiOptionsShown = false;
    resize(sizeHint());
}

void ClickImageEvent::saveSelectedParameters(QMap<QString,int> _margins, QMap<QString, double> _ratios, QMap<QString,double> _parameters){
    qDebug()<<"Saving frangi parameters: "<<_margins<<" "<<_ratios<<" "<<_parameters;
    if (whatIsAnalysed == chosenSource::VIDEO) {
        SharedVariables::getSharedVariables()->setFrangiMarginsWrapper(frangiType::VIDEO_SPECIFIC,
                                                                       _margins,videoName);
        SharedVariables::getSharedVariables()->setFrangiRatiosWrapper(frangiType::VIDEO_SPECIFIC,
                                                                       _ratios,videoName);
        SharedVariables::getSharedVariables()->setFrangiParameterWrapper(frangiType::VIDEO_SPECIFIC,
                                                                       videoName,_parameters);
    }
    else if (whatIsAnalysed == chosenSource::IMAGE) {
        SharedVariables::getSharedVariables()->setFrangiMarginsWrapper(frangiType::GLOBAL,
                                                                       _margins,"");
        SharedVariables::getSharedVariables()->setFrangiRatiosWrapper(frangiType::GLOBAL,
                                                                       _ratios,"");
        SharedVariables::getSharedVariables()->setFrangiParameterWrapper(frangiType::GLOBAL,
                                                                       "",_parameters);
    }
}

IntegratedFrangiOptions::IntegratedFrangiOptions(){
    createWidget();
}

IntegratedFrangiOptions::~IntegratedFrangiOptions(){
    delete layoutForIntegratedFrangi;
}

void IntegratedFrangiOptions::createWidget(){
    // connect labels and corresponding names
    for (int textIndex=0; textIndex<labelsTexts.length()-1; textIndex++) {
        if (textIndex < 4) {
            labelsConnections.insert(FrangiMarginsList.at(FrangiMarginsList.length()-1-textIndex),labelsTexts.at(textIndex));
        }
        else if (textIndex > 3 && textIndex < 8) {
            labelsConnections.insert(FrangiRatiosList.at(2*FrangiRatiosList.length()-1-textIndex),labelsTexts.at(textIndex));
        }
        else if (textIndex > 7)
            labelsConnections.insert(FrangiParametersList.at(3*FrangiParametersList.length()-6-textIndex),labelsTexts.at(textIndex));
    }

    layoutForIntegratedFrangi = new QGridLayout(this);
    int _innerCounter = 0;
    // create margins spinboxes and corresponding labels
    foreach (QString margin, FrangiMarginsList) {
        marginsConnections.insert(margin,new QSpinBox);
        labels.insert(margin,new QLabel(labelsConnections[margin]));
        addWidgetToGrid(labels[margin],0,_innerCounter);
        addWidgetToGrid(marginsConnections[margin],1,_innerCounter);
        _innerCounter++;
    }
    // create ratio doublespinboxes and corresponding labels
    _innerCounter = 0;
    foreach (QString ratio, FrangiRatiosList) {
        ratiosConnections.insert(ratio,new QDoubleSpinBox);
        labels.insert(ratio,new QLabel(labelsConnections[ratio]));
        addWidgetToGrid(labels[ratio],2,_innerCounter);
        addWidgetToGrid(ratiosConnections[ratio],3,_innerCounter);
        _innerCounter++;
    }
    // create parameters doublespinboxes and corresponding labels
    _innerCounter = 0;
    foreach (QString parameter, FrangiParametersList) {
        if (parameter != "zpracovani") {
            parametersConnections.insert(parameter,new QDoubleSpinBox);
            labels.insert(parameter,new QLabel(labelsConnections[parameter]));
            addWidgetToGrid(labels[parameter],4,_innerCounter);
            addWidgetToGrid(parametersConnections[parameter],5,_innerCounter);
            _innerCounter++;
        }
    }

    // adding buttons and radios
    setParameters = new QPushButton(tr("Set"));
    saveParameters = new QPushButton(tr("Save"));
    addWidgetToGrid(setParameters,1,4);
    addWidgetToGrid(saveParameters,1,5);
    labels.insert("reverse",new QLabel(tr("Reverse")));
    labels.insert("standard",new QLabel(tr("Standard")));
    reverse = new QRadioButton;
    standard = new QRadioButton;
    addWidgetToGrid(labels["reverse"],2,4);
    addWidgetToGrid(labels["standard"],2,5);
    addWidgetToGrid(reverse,3,4);
    addWidgetToGrid(standard,3,5);
    BG = new QButtonGroup;
    BG->addButton(standard);
    BG->addButton(reverse);
    BG->setExclusive(true);
    layoutForIntegratedFrangi->setMargin(2);
}

void IntegratedFrangiOptions::addWidgetToGrid(QWidget *widget, int row, int col) {
    layoutForIntegratedFrangi->addWidget(widget,row,col);
}

void IntegratedFrangiOptions::saveButtonClicked(){
    QMap<QString,int> _margins;
    QMap<QString,double> _ratios;
    QMap<QString,double> _parameters;

    GetSetValues("GET",_margins,_ratios,_parameters);
    emit frangiParametersSelected(_margins,_ratios,_parameters);
}

void IntegratedFrangiOptions::setButtonClicked() {
    QMap<QString,int> _margins;
    QMap<QString,double> _ratios;
    QMap<QString,double> _parameters;

    GetSetValues("GET",_margins,_ratios,_parameters);
    marginsInternal = _margins;
    parametersInternal = _parameters;
    ratiosInternal = _ratios;
    emit applyFrangiParameters();
}

void IntegratedFrangiOptions::GetSetValues(QString method,QMap<QString, int> &m,
                                           QMap<QString, double> &r,
                                           QMap<QString, double> &p) {
    if (method == "GET") {
        //int _innerCounter = 0;
        foreach (QString margin, FrangiMarginsList) {
            m[margin] = marginsConnections[margin]->value();
            //_innerCounter++;
        }
        //_innerCounter = 0;
        foreach (QString ratio, FrangiRatiosList) {
            r[ratio] = ratiosConnections[ratio]->value();
            //_innerCounter++;
        }
        //_innerCounter = 0;
        foreach (QString parameter, FrangiParametersList) {
            if (parameter != "zpracovani") {
                p[parameter] = parametersConnections[parameter]->value();
                //_innerCounter++;
            }
            else {
                if (standard->isChecked())
                    p[parameter] = 1.0;
                else
                    p[parameter] = 0.0;
            }
        }
    }
    else {
        //int _innerCounter = 0;
        foreach (QString margin, FrangiMarginsList) {
            marginsConnections[margin]->setValue(m[margin]);
            marginsInternal[margin] = m[margin];
            //_innerCounter++;
        }
        //_innerCounter = 0;
        foreach (QString ratio, FrangiRatiosList) {
            ratiosConnections[ratio]->setValue(r[ratio]);
            ratiosInternal[ratio] = r[ratio];
            //_innerCounter++;
        }
        //_innerCounter = 0;
        foreach (QString parameter, FrangiParametersList) {
            if (parameter != "zpracovani") {
                parametersConnections[parameter]->setValue(p[parameter]);
                parametersInternal[parameter] = p[parameter];
                //_innerCounter++;
            }
            else {
                if (p[parameter] == 1.0)
                    standard->setChecked(true);
                else
                    reverse->setChecked(true);
            }
        }
    }
}
