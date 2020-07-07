#include "main_program/registrationresult.h"
#include "ui_registrationresult.h"

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

RegistrationResult::RegistrationResult(cv::Mat& _referencial, cv::Mat& _translated, QDialog *parent) : QDialog(parent),
    ui(new Ui::RegistrationResult)
{
    ui->setupUi(this);
    _referencial.copyTo(referencialFrame);
    _translated.copyTo(registratedFrame);
    //_referencial.release();
    //_translated.release();
    connect(ui->vyberSnimek,SIGNAL(valueChanged(int)),this,SLOT(changeDisplayed(int)));
}

RegistrationResult::~RegistrationResult()
{
    delete ui;
}

void RegistrationResult::displayTwo()
{
    setWindowsSize(referencialFrame.cols,referencialFrame.rows);

    imageReference = new QImage(referencialFrame.data,
                                referencialFrame.cols,
                                referencialFrame.rows,
                                static_cast<int>(referencialFrame.step),
                                QImage::Format_RGB888);
    imageSlicovany = new QImage(registratedFrame.data,
                                registratedFrame.cols,
                                registratedFrame.rows,
                                static_cast<int>(registratedFrame.step),
                                QImage::Format_RGB888);
}

void RegistrationResult::displayVideo(cv::VideoCapture video)
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

void RegistrationResult::changeDisplayed(int value)
{
    if (wantToDisplay == 1){
        //qDebug()<<value;
        if (value == 0)
        {
            //qDebug()<<value;
            QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageReference));
            scene->addItem(image);
        }
        if (value == 1)
        {
            //qDebug()<<value;
            QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageSlicovany));
            scene->addItem(image);
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
            scene->addItem(image);
        }
    }

}

void RegistrationResult::setWindowsSize(int width, int height)
{
    this->setFixedSize(width,height);
}

void RegistrationResult::callTwo()
{
    displayTwo();
}

void RegistrationResult::callVideo()
{
    displayVideo(actualVideo);
}

void RegistrationResult::start(int startMethod)
{
    if (startMethod == 1){
        callTwo();
        QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*imageReference));
        scene = new QGraphicsScene();
        view = new QGraphicsView(scene);
        view->setFixedSize(referencialFrame.cols,referencialFrame.rows);
        ui->vysledekLicovani->setScene(scene);
        scene->addItem(image);
        ui->vysledekLicovani->setFixedSize(referencialFrame.cols+30,referencialFrame.rows+30);
        ui->vysledekLicovani->setSceneRect(0,0,referencialFrame.cols+30,referencialFrame.rows+30);
        this->setGeometry(50,50,referencialFrame.cols,referencialFrame.rows);
        wantToDisplay = startMethod;
    }
    if (startMethod == 2){
        callVideo();
        QGraphicsPixmapItem* image = new QGraphicsPixmapItem(QPixmap::fromImage(*actualFrame));
        scene = new QGraphicsScene();
        view = new QGraphicsView(scene);
        view->setFixedSize(frame0.cols,frame0.rows);
        ui->vysledekLicovani->setScene(scene);
        scene->addItem(image);
        ui->vysledekLicovani->setFixedSize(frame0.cols+30,frame0.rows+30);
        ui->vysledekLicovani->setSceneRect(0,0,frame0.cols+30,frame0.rows+30);
        this->setGeometry(50,50,frame0.cols,frame0.rows);
        wantToDisplay = startMethod;
    }
}
