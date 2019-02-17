#ifndef CLICKIMAGEEVENT_H
#define CLICKIMAGEEVENT_H

#include <QDialog>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QPoint>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>

#include <opencv2/opencv.hpp>

namespace Ui {
class ClickImageEvent;
}

class ClickImageEvent : public QDialog
{
    Q_OBJECT

public:
    explicit ClickImageEvent(QString i_fullPath,int i_referencialNumber, int i_anomalyType, QDialog *parent = nullptr);
    ~ClickImageEvent();
    void setWindowSize(int width, int height);
public slots:
    void mousePressEvent(QMouseEvent *press);
private:

    void paintCross(QPainterPath &path,double x, double y);
    Ui::ClickImageEvent *ui;
    QPixmap image;
    QImage  *imageObject;
    QGraphicsScene *scene;
    QGraphicsView *view;
    double height, width;
    QGraphicsPathItem* pathItem;
    QString filePath;
    int anomalyType;
};

#endif // CLICKIMAGEEVENT_H
