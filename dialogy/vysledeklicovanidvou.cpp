#include "vysledeklicovanidvou.h"
#include "ui_vysledeklicovanidvou.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include "opencv2/imgproc/types_c.h"

VysledekLicovaniDvou::VysledekLicovaniDvou(cv::Mat referencni,cv::Mat slicovany,QDialog *parent) :
    QDialog(parent),
    ui(new Ui::VysledekLicovaniDvou)
{
    ui->setupUi(this);

    referencni.copyTo(referencniSnimek);
    slicovany.copyTo(slicovanySnimek);
    nastav_velikost_okna(referencniSnimek.cols,referencniSnimek.rows);

    imageReference = new QImage(referencniSnimek.data,
                                referencniSnimek.cols,
                                referencniSnimek.rows,
                                static_cast<int>(referencniSnimek.step),
                                QImage::Format_RGB888);
    imageSlicovany = new QImage(slicovanySnimek.data,
                                slicovanySnimek.cols,
                                slicovanySnimek.rows,
                                static_cast<int>(slicovanySnimek.step),
                                QImage::Format_RGB888);

    QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageReference));
    scena = new QGraphicsScene();
    pohled = new QGraphicsView(scena);
    pohled->setFixedSize(referencniSnimek.cols,referencniSnimek.rows);
    ui->vysledekLicovani->setScene(scena);
    scena->addItem(image);
    ui->vysledekLicovani->setFixedSize(referencniSnimek.cols+30,referencniSnimek.rows+30);
    ui->vysledekLicovani->setSceneRect(0,0,referencniSnimek.cols+30,referencniSnimek.rows+30);
    this->setGeometry(50,50,referencniSnimek.cols,referencniSnimek.rows);

}

VysledekLicovaniDvou::~VysledekLicovaniDvou()
{
    delete ui;
}

void VysledekLicovaniDvou::on_vyberSnimek_valueChanged(int value)
{
    if (value == 0)
    {
        QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageReference));
        scena = new QGraphicsScene();
        pohled = new QGraphicsView(scena);
        pohled->setFixedSize(referencniSnimek.cols,referencniSnimek.rows);
        ui->vysledekLicovani->setScene(scena);
        scena->addItem(image);
        ui->vysledekLicovani->setFixedSize(referencniSnimek.cols+30,referencniSnimek.rows+30);
        ui->vysledekLicovani->setSceneRect(0,0,referencniSnimek.cols+30,referencniSnimek.rows+30);
    }
    if (value == 1)
    {
        QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageSlicovany));
        scena = new QGraphicsScene();
        pohled = new QGraphicsView(scena);
        pohled->setFixedSize(referencniSnimek.cols,referencniSnimek.rows);
        ui->vysledekLicovani->setScene(scena);
        scena->addItem(image);
        ui->vysledekLicovani->setFixedSize(referencniSnimek.cols+30,referencniSnimek.rows+30);
        ui->vysledekLicovani->setSceneRect(0,0,referencniSnimek.cols+30,referencniSnimek.rows+30);
    }
}

void VysledekLicovaniDvou::nastav_velikost_okna(int sirka,int vyska)
{
    this->setFixedSize(sirka,vyska);
}
