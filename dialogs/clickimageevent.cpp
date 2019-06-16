#include "dialogs/clickimageevent.h"
#include "ui_clickimageevent.h"
#include "image_analysis/image_processing.h"
#include "shared_staff/sharedvariables.h"

#include <QGraphicsView>
#include <QPoint>
#include <QImage>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include "opencv2/imgproc/types_c.h"

ClickImageEvent::ClickImageEvent(QString i_fullPath, int i_anomalyType, QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    filePath = i_fullPath;
    anomalyType = i_anomalyType;
    ui->clickImage->setMouseTracking(false);
    ui->clickImage->setEnabled(false);
    referencialFrame_LE = new QLineEdit();
    ui->optionalContent->addWidget(referencialFrame_LE,0,0);
    referencialFrame_LE->setPlaceholderText(tr("Ref"));
    selectedVideo = new QLabel();
    ui->optionalContent->addWidget(selectedVideo,0,1);
    selectedVideo->setText(filePath);
    cv::VideoCapture cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);

    QObject::connect(referencialFrame_LE,SIGNAL(textEdited(QString)),this,SLOT(referencialFrameChosen(QString)));

    //QPoint viewPoint = ui->clickImage->mapFromGlobal(QCursor::pos());
    //QPointF scenePoint = ui->clickImage->mapToScene(viewPoint);
    //graphicalWidget->setPos(scenePoint);
}

ClickImageEvent::ClickImageEvent(QString i_fullPath, int i_referFrameNo, int i_anomalyType, QDialog *parent) :
    QDialog (parent),ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    filePath = i_fullPath;
    anomalyType = i_anomalyType;
    ui->clickImage->setMouseTracking(true);
    ui->clickImage->setEnabled(true);
    referencialFrame_LE = new QLineEdit();
    ui->optionalContent->addWidget(referencialFrame_LE,0,0);
    referencialFrame_LE->setText(QString::number(i_referFrameNo));
    selectedVideo = new QLabel();
    ui->optionalContent->addWidget(selectedVideo,0,1);
    selectedVideo->setText(filePath);
    cv::VideoCapture cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    referencialFrameNo = i_referFrameNo;

    fillGraphicScene();

    QObject::connect(referencialFrame_LE,SIGNAL(textEdited(QString)),this,SLOT(referencialFrameChosen(QString)));
}

ClickImageEvent::ClickImageEvent(QStringList i_fullPaths, int i_anomalyType, QDialog *parent) :
    QDialog(parent),ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    filePaths = i_fullPaths;
    anomalyType = i_anomalyType;
    ui->clickImage->setMouseTracking(false);
    ui->clickImage->setEnabled(false);
    referencialFrame_LE->setEnabled(false);
    referencialFrame_LE = new QLineEdit();
    ui->optionalContent->addWidget(referencialFrame_LE,0,0);
    referencialFrame_LE->setPlaceholderText(tr("Ref"));
    videoSelection = new QComboBox();
    ui->optionalContent->addWidget(videoSelection,0,1);
    videoSelection->addItems(filePaths);

    QObject::connect(videoSelection,SIGNAL(currentIndexChanged(int)),this,SLOT(processChosenVideo(int)));
    QObject::connect(referencialFrame_LE,SIGNAL(textEdited(QString)),this,SLOT(referencialFrameChosen(QString)));
}

ClickImageEvent::~ClickImageEvent()
{
    delete ui;
}

void ClickImageEvent::setWindowSize(int width, int height)
{
    this->setFixedSize(width,height);
}

void ClickImageEvent::mousePressEvent(QMouseEvent *press)
{
    QPointF pt = ui->clickImage->mapToScene(press->pos());//ui->clickImage->mapToScene(press->pos());
    qDebug()<<pt.x()<<pt.y();
    /*QGraphicsItem* item = scene->itemAt(pt,QTransform());
    QPointF localPt = item->mapFromScene(pt);
    qDebug()<<localPt.x()<<localPt.y();*/

    if (anomalyType == 1 && !disabled) // horizontal
    {
        if (SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().x == 0.0
                || SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().y == 0.0){
            QPainterPath crossPath;
            scene->removeItem(pathItem);
            scene->removeItem(extraCutout_GRI);
            scene->removeItem(standardCutout_GRI);
            paintCross(crossPath,pt.x(),pt.y());
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
            updateCutoutExtra(pt,2);
            SharedVariables::getSharedVariables()->setHorizontalAnomalyCoords(pt);
            standardCutout_GRI = scene->addRect(standardCutout,QPen(QColor(255, 0, 0)));
            extraCutout_GRI = scene->addRect(extraCutout,QPen(QColor(0,0,255)));
            scene->update();
        }
        else{
            updateCutoutStandard(pt,2);
            scene->removeItem(pathItem);
            scene->removeItem(standardCutout_GRI);
            scene->removeItem(extraCutout_GRI);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());            
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
            updateCutoutExtra(pt,2);
            SharedVariables::getSharedVariables()->setHorizontalAnomalyCoords(pt);
            standardCutout_GRI = scene->addRect(standardCutout,QPen(QColor(255, 0, 0)));
            extraCutout_GRI = scene->addRect(extraCutout,QPen(QColor(0,0,255)));
            scene->update();
        }
    }
    if (anomalyType == 2 && !disabled) // vertical
    {
        if (SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().x == 0.0
                || SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().y == 0.0){
            scene->removeItem(pathItem);
            scene->removeItem(standardCutout_GRI);
            scene->removeItem(extraCutout_GRI);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
            updateCutoutExtra(pt,1);
            SharedVariables::getSharedVariables()->setVerticalAnomalyCoords(pt);
            standardCutout_GRI = scene->addRect(standardCutout,QPen(QColor(255, 0, 0)));
            extraCutout_GRI = scene->addRect(extraCutout,QPen(QColor(0,0,255)));
            scene->update();
        }
        else{
            scene->removeItem(pathItem);
            scene->removeItem(standardCutout_GRI);
            scene->removeItem(extraCutout_GRI);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());            
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
            updateCutoutExtra(pt,1);
            SharedVariables::getSharedVariables()->setVerticalAnomalyCoords(pt);
            standardCutout_GRI = scene->addRect(standardCutout,QPen(QColor(255, 0, 0)));
            extraCutout_GRI = scene->addRect(extraCutout,QPen(QColor(0,0,255)));
            scene->update();
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
    cv::Point3d _tempCoordinates = SharedVariables::getSharedVariables()->getFrangiMaximum();
    QPointF result(_tempCoordinates.x,_tempCoordinates.y);
    return result;
}

void ClickImageEvent::fillGraphicScene(){
    if (!cap.isOpened())
        cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrameNo);
    cv::Mat referencialImage;
    cap.read(referencialImage);
    width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    transformMatTypeTo8C3(referencialImage);

    imageObject = new QImage(referencialImage.data,
                             referencialImage.cols,
                             referencialImage.rows,
                             static_cast<int>(referencialImage.step),
                             QImage::Format_RGB888);//

    QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageObject));
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    view->setFixedSize(referencialImage.cols,referencialImage.rows);
    ui->clickImage->setScene(scene);
    scene->addItem(image);
    QPainterPath crossPath;
    paintCross(crossPath,loadFrangiMaxCoordinates().x(),loadFrangiMaxCoordinates().y());
    pathItem = scene->addPath(crossPath,QPen(QColor(255, 0, 0), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
    initCutouts(referencialImage);
    standardCutout_GRI = scene->addRect(standardCutout,QPen(QColor(255, 0, 0)));
    extraCutout_GRI = scene->addRect(extraCutout,QPen(QColor(0,0,255)));

    ui->clickImage->setFixedSize(referencialImage.cols+50,referencialImage.rows+50);
    ui->clickImage->setSceneRect(0,0,referencialImage.cols,referencialImage.rows);
    ui->clickImage->setMouseTracking(true);
}

void ClickImageEvent::initCutouts(cv::Mat i_inputFrame){
    cv::Point3d frangi_point = SharedVariables::getSharedVariables()->getFrangiMaximum();
    QMap<QString,double> frangiRatios = SharedVariables::getSharedVariables()->getFrangiRatios();
    standardCutout.setX((round(frangi_point.x-frangiRatios["left_r"]*(frangi_point.x))));
    standardCutout.setY(int(round(frangi_point.y-frangiRatios["top_r"]*frangi_point.y)));
    int rowTo = int(round(frangi_point.y+frangiRatios["bottom_r"]*(i_inputFrame.rows - frangi_point.y)));
    int columnTo = int(round(frangi_point.x+frangiRatios["right_r"]*(i_inputFrame.cols - frangi_point.x)));
    standardCutout.setWidth(columnTo-standardCutout.x());
    standardCutout.setHeight(rowTo - standardCutout.y());

    extraCutout.setX(0);
    extraCutout.setY(0);
    extraCutout.setWidth(width);
    extraCutout.setHeight(height);
}

void ClickImageEvent::updateCutoutStandard(QPointF i_C, int i_anomalyType){
    cv::Point3d F = SharedVariables::getSharedVariables()->getFrangiMaximum();
    if (i_C.x()<(width/2.0) && i_anomalyType==2){
        standardCutout.setX(F.x-(F.x - extraCutout.x())*0.9);
        standardCutout.setWidth((F.x-i_C.x())*0.9+((extraCutout.x()+extraCutout.width())-F.x)*0.9);
    }
    else if (i_C.x()>(width/2.0) && i_anomalyType==2)
        standardCutout.setWidth((F.x-standardCutout.x())+(i_C.x()-F.x)*0.9);

    if (i_C.y()<(height/2.0) && i_anomalyType==1){
        standardCutout.setY(F.y-(F.y-extraCutout.y())*0.9);
        standardCutout.setHeight((F.y-i_C.y())*0.9+((extraCutout.y()+extraCutout.height())-F.y)*0.9);
    }
    else if (i_C.y()>(height/2.0) && i_anomalyType==1)
        standardCutout.setHeight(0.9*(F.y-extraCutout.y())+(i_C.y()-F.y)*0.9);
}

void ClickImageEvent::updateCutoutExtra(QPointF i_clickCoordinates, int i_anomalyType){
    if (i_anomalyType == 2 && i_clickCoordinates.x()<(width/2.0)){
        extraCutout.setX(i_clickCoordinates.x());
        extraCutout.setWidth(i_clickCoordinates.x()+(extraCutout.width()-i_clickCoordinates.x()));
    }
    else if (i_anomalyType == 2 && i_clickCoordinates.x()>(width/2.0)){
        extraCutout.setWidth(i_clickCoordinates.x() - extraCutout.x());
    }
    else if (i_anomalyType == 1 && i_clickCoordinates.y()>(height/2.0)){
        extraCutout.setHeight(i_clickCoordinates.y() - extraCutout.y());
    }
    else if (i_anomalyType == 1 && i_clickCoordinates.y()<(height/2.0)){
        extraCutout.setY(i_clickCoordinates.y());
        extraCutout.setHeight(extraCutout.y()+(extraCutout.height()-i_clickCoordinates.y()));
    }
    updateCutoutStandard(i_clickCoordinates,i_anomalyType);
}

void ClickImageEvent::referencialFrameChosen(const QString &arg1)
{
    referencialFrameNo = arg1.toDouble()-1.0;
    if (referencialFrameNo < 0.0 || referencialFrameNo > frameCount)
    {
        referencialFrame_LE->setStyleSheet("color: #FF0000");
        referencialFrameNo = -1.0;
        ui->clickImage->setEnabled(false);
        ui->clickImage->setMouseTracking(false);
        disabled = true;
    }
    else
    {
        referencialFrame_LE->setStyleSheet("color: #33aa00");
        ui->clickImage->setEnabled(true);
        ui->clickImage->setMouseTracking(true);
        disabled = false;
        fillGraphicScene();
        //this->setGeometry(50,50,referencialImage.cols,referencialImage.rows);
    }
}

void ClickImageEvent::processChosenVideo(int videoIndex){
    QString chosenVideoPath = filePaths.at(videoIndex);
    cap = cv::VideoCapture(chosenVideoPath.toLocal8Bit().constData());
    if (!cap.isOpened()){
        referencialFrame_LE->setEnabled(false);
        cap.release();
    }
    else{
        referencialFrame_LE->setEnabled(true);
        frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    }

}
