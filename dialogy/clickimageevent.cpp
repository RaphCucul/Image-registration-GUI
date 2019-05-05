#include "dialogy/clickimageevent.h"
#include "ui_clickimageevent.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "fancy_staff/sharedvariables.h"

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

ClickImageEvent::ClickImageEvent(QString i_fullPath, int i_referencialNumber, int i_anomalyType, QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    filePath = i_fullPath;
    anomalyType = i_anomalyType;
    ui->clickImage->setMouseTracking(true);
    cv::VideoCapture cap = cv::VideoCapture(filePath.toLocal8Bit().constData());
    cap.set(CV_CAP_PROP_POS_FRAMES,double(i_referencialNumber));
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
    ui->clickImage->setFixedSize(referencialImage.cols+30,referencialImage.rows+30);
    ui->clickImage->setSceneRect(0,0,referencialImage.cols+30,referencialImage.rows+30);
    this->setGeometry(50,50,referencialImage.cols,referencialImage.rows);

    //QPoint viewPoint = ui->clickImage->mapFromGlobal(QCursor::pos());
    //QPointF scenePoint = ui->clickImage->mapToScene(viewPoint);
    //graphicalWidget->setPos(scenePoint);
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
    QPointF pt = ui->clickImage->mapToScene(press->pos());
    qDebug()<<pt.x()<<pt.y();
    if (anomalyType == 2)
    {
        if (SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().x == 0.0 ||
                SharedVariables::getSharedVariables()->getVerticalAnomalyCoords().y == 0.0){
            SharedVariables::getSharedVariables()->setVerticalAnomalyCoords(pt);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());
            //pathItem = new QGraphicsPathItem(crossPath);
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
        }
        else{
            SharedVariables::getSharedVariables()->setVerticalAnomalyCoords(pt);
            scene->removeItem(pathItem);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
        }
    }
    if (anomalyType == 1)
    {
        if (SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().x == 0.0 ||
                SharedVariables::getSharedVariables()->getHorizontalAnomalyCoords().y == 0.0){
            SharedVariables::getSharedVariables()->setHorizontalAnomalyCoords(pt);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());
            //pathItem = new QGraphicsPathItem(crossPath);
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
        }
        else{
            SharedVariables::getSharedVariables()->setHorizontalAnomalyCoords(pt);
            scene->removeItem(pathItem);
            QPainterPath crossPath;
            paintCross(crossPath,pt.x(),pt.y());
            pathItem = scene->addPath(crossPath,QPen(QColor(0, 0, 255), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin));
        }
    }
}

void ClickImageEvent::paintCross(QPainterPath &path, double x, double y){
    path.moveTo(x, y-10);
    path.lineTo(x, y+10);
    path.moveTo(x-10, y);
    path.lineTo(x+10, y);
}
