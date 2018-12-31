#include "hlavni_program/vysledeklicovani.h"
#include "ui_vysledeklicovani.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include "opencv2/imgproc/types_c.h"

VysledekLicovani::VysledekLicovani(QDialog *parent) : QDialog(parent),
    ui(new Ui::VysledekLicovani)
{
    ui->setupUi(this);

}

VysledekLicovani::~VysledekLicovani()
{
    delete ui;
}

void VysledekLicovani::displayTwo(cv::Mat reference, cv::Mat moved)
{
    reference.copyTo(referencniSnimek);
    moved.copyTo(slicovanySnimek);
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
}

void VysledekLicovani::displayVideo(cv::VideoCapture video)
{
    actualVideo = video;
    actualVideo.set(CV_CAP_PROP_POS_FRAMES,0);
    if (actualVideo.read(frame0)!=1)
    {
        qWarning()<< "Image "<<0<<" cannot be registrated.";
    }
    actualFrame = new QImage(frame0.data,
                             frame0.cols,
                             frame0.rows,
                             static_cast<int>(frame0.step),
                             QImage::Format_RGB888);
}

void VysledekLicovani::on_vyberSnimek_valueChanged(int value)
{
    if (wantToDisplay == 1){
        if (value == 0)
        {
            QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageReference));
            scena->addItem(image);
        }
        if (value == 1)
        {
            QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageSlicovany));
            scena->addItem(image);
        }
    }
    if (wantToDisplay == 2){
        cv::Mat displayedFrame;
        actualVideo.set(CV_CAP_PROP_POS_FRAMES,value);
        if (actualVideo.read(displayedFrame)!=1)
        {
            qWarning()<< "Image "<<0<<" cannot be registrated.";
        }
        else{
            actualFrame = new QImage(displayedFrame.data,
                                     displayedFrame.cols,
                                     displayedFrame.rows,
                                     static_cast<int>(displayedFrame.step),
                                     QImage::Format_RGB888);
            QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*actualFrame));
            scena->addItem(image);
        }
    }

}

void VysledekLicovani::nastav_velikost_okna(int sirka,int vyska)
{
    this->setFixedSize(sirka,vyska);
}

void VysledekLicovani::callTwo()
{
    displayTwo(referencniSnimek,slicovanySnimek);
}

void VysledekLicovani::callVideo()
{
    displayVideo(actualVideo);
}

void VysledekLicovani::start(int startMethod)
{
    if (startMethod == 1){
        callTwo();
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
    if (startMethod == 2){
        callVideo();
        QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*actualFrame));
        scena = new QGraphicsScene();
        pohled = new QGraphicsView(scena);
        pohled->setFixedSize(frame0.cols,frame0.rows);
        ui->vysledekLicovani->setScene(scena);
        scena->addItem(image);
        ui->vysledekLicovani->setFixedSize(frame0.cols+30,frame0.rows+30);
        ui->vysledekLicovani->setSceneRect(0,0,frame0.cols+30,frame0.rows+30);
        this->setGeometry(50,50,frame0.cols,frame0.rows);
    }
}
