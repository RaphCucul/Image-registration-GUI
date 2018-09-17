#ifndef VYSLEDEKLICOVANIDVOU_H
#define VYSLEDEKLICOVANIDVOU_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>

#include <opencv2/opencv.hpp>

namespace Ui {
class VysledekLicovaniDvou;
}

class VysledekLicovaniDvou : public QDialog
{
    Q_OBJECT

public:
    explicit VysledekLicovaniDvou(cv::Mat referencni,cv::Mat slicovany,QDialog *parent = nullptr);
    ~VysledekLicovaniDvou();
    void nastav_velikost_okna(int sirka, int vyska);
private slots:
    void on_vyberSnimek_valueChanged(int value);

private:
    Ui::VysledekLicovaniDvou *ui;
    QPixmap image;
    QImage *imageReference;
    QImage *imageSlicovany;
    QGraphicsScene *scena;
    QGraphicsView *pohled;
    QGraphicsWidget* graphicalWidget;
    cv::Mat referencniSnimek, slicovanySnimek;
};

#endif // VYSLEDEKLICOVANIDVOU_H
