#ifndef CLICKIMAGEEVENT_H
#define CLICKIMAGEEVENT_H

#include <QDialog>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QPoint>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>

#include <opencv2/opencv.hpp>

namespace Ui {
class ClickImageEvent;
}

extern cv::Point2f oznacena_hranice_svetelne_anomalie;
extern cv::Point2f oznacena_hranice_casove_znacky;

class ClickImageEvent : public QDialog
{
    Q_OBJECT

public:
    explicit ClickImageEvent(QString kompletni_cesta,int cisloReference, int TypAnomalie, QDialog *parent = nullptr);
    ~ClickImageEvent();
    void nastav_velikost_okna(int a_sirka, int b_vyska);
public slots:
    void mousePressEvent(QMouseEvent *press);
signals:
    //void SendClickCoordinates(QPointF hranice_anomalie);
private:
    Ui::ClickImageEvent *ui;
    QPixmap image;
    QImage  *imageObject;
    QGraphicsScene *scena;
    QGraphicsView *pohled;
    double vyska, sirka;
    QGraphicsWidget* graphicalWidget;
    QString cesta_k_souboru;
    int typ_anomalie;
};

#endif // CLICKIMAGEEVENT_H
