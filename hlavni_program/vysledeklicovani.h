#ifndef VYSLEDEKLICOVANI_H
#define VYSLEDEKLICOVANI_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>

#include <opencv2/opencv.hpp>

namespace Ui {
class VysledekLicovani;
}

class VysledekLicovani : public QDialog
{
    Q_OBJECT

public:
    explicit VysledekLicovani(cv::Mat &_referencial, cv::Mat &_translated, QDialog *parent = nullptr);
    ~VysledekLicovani();
    void nastav_velikost_okna(int sirka, int vyska);
    void displayTwo();
    void displayVideo(cv::VideoCapture video);
    void callTwo();
    void callVideo();
    void start(int startMethod);
private slots:
    void changeDisplayed(int value);
private:


    Ui::VysledekLicovani *ui;
    int wantToDisplay = 0;
    QPixmap image;
    QImage *imageReference;
    QImage *imageSlicovany;
    QImage *actualFrame;
    QGraphicsScene *scena;
    QGraphicsView *pohled;
    QGraphicsWidget* graphicalWidget;
    cv::Mat referencniSnimek, slicovanySnimek, frame0;
    cv::VideoCapture actualVideo;
};

#endif // VYSLEDEKLICOVANI_H
