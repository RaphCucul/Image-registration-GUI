#include "clickimageevent.h"
#include "ui_clickimageevent.h"
#include "util/upravy_obrazu.h"

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

cv::Point2f oznacena_hranice_svetelne_anomalie;
cv::Point2f oznacena_hranice_casove_znacky;

ClickImageEvent::ClickImageEvent(QString kompletni_cesta,int cisloReference,int TypAnomalie,QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ClickImageEvent)
{
    ui->setupUi(this);
    cesta_k_souboru = kompletni_cesta;
    typ_anomalie = TypAnomalie;
    ui->klikniNaObrazek->setMouseTracking(true);
    cv::VideoCapture cap = cv::VideoCapture(cesta_k_souboru.toLocal8Bit().constData());
    cap.set(CV_CAP_PROP_POS_FRAMES,double(cisloReference));
    cv::Mat referencni_snimek;
    cap.read(referencni_snimek);
    sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    //qDebug()<<"Referece má "<<referencni_snimek.channels()<<" kanálů a je typu"<<referencni_snimek.type();
    kontrola_typu_snimku_8C3(referencni_snimek);
    //qDebug()<<"Referece má "<<referencni_snimek.channels()<<" kanálů a je typu"<<referencni_snimek.type();

    imageObject = new QImage(referencni_snimek.data,
                             referencni_snimek.cols,
                             referencni_snimek.rows,
                             static_cast<int>(referencni_snimek.step),
                             QImage::Format_RGB888);//

    //imageObject->load(kompletni_cesta);
    //image = QPixmap::fromImage(*imageObject);
    QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageObject));
    scena = new QGraphicsScene();
    pohled = new QGraphicsView(scena);
    pohled->setFixedSize(referencni_snimek.cols,referencni_snimek.rows);
    ui->klikniNaObrazek->setScene(scena);
    scena->addItem(image);
    //pohled->show();
    ui->klikniNaObrazek->setFixedSize(referencni_snimek.cols+30,referencni_snimek.rows+30);
    ui->klikniNaObrazek->setSceneRect(0,0,referencni_snimek.cols+30,referencni_snimek.rows+30);
    this->setGeometry(50,50,referencni_snimek.cols,referencni_snimek.rows);
    //ui->klikniNaObrazek->fitInView(scena->sceneRect(),Qt::KeepAspectRatio); // nefunguje


    /*QPoint viewPoint = ui->klikniNaObrazek->mapFromGlobal(QCursor::pos());
    QPointF scenePoint = ui->klikniNaObrazek->mapToScene(viewPoint);
    graphicalWidget->setPos(scenePoint);*/
}

ClickImageEvent::~ClickImageEvent()
{
    delete ui;
}

void ClickImageEvent::nastav_velikost_okna(int sirka,int vyska)
{
    this->setFixedSize(sirka,vyska);
}

void ClickImageEvent::mousePressEvent(QMouseEvent *press)
{
    QPointF pt = ui->klikniNaObrazek->mapToScene(press->pos());
    //qDebug()<<pt.x()<<pt.y();
    if (typ_anomalie == 1)
    {
        oznacena_hranice_svetelne_anomalie.x = float(pt.x());
        oznacena_hranice_svetelne_anomalie.y = float(pt.y());
        oznacena_hranice_casove_znacky.x = 0;
        oznacena_hranice_casove_znacky.y = 0;
    }
    if (typ_anomalie == 2)
    {
        oznacena_hranice_casove_znacky.x = float(pt.x());
        oznacena_hranice_casove_znacky.y = float(pt.y());
        oznacena_hranice_svetelne_anomalie.x = 0;
        oznacena_hranice_svetelne_anomalie.y = 0;
    }
    qDebug()<<"MousePressEvent: "<<oznacena_hranice_svetelne_anomalie.x<<" "<<oznacena_hranice_svetelne_anomalie.y;
    qDebug()<<"MousePressEvent: "<<oznacena_hranice_casove_znacky.x<<" "<<oznacena_hranice_casove_znacky.y;
    //emit SendClickCoordinates(pt);
}
