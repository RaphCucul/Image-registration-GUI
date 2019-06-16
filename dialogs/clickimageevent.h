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
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include <opencv2/opencv.hpp>

namespace Ui {
class ClickImageEvent;
}

class ClickImageEvent : public QDialog
{
    Q_OBJECT

public:
    explicit ClickImageEvent(QString i_fullPath,int i_anomalyType, QDialog *parent = nullptr);
    explicit ClickImageEvent(QString i_fullPath,int i_referFrameNo,int i_anomalyType, QDialog *parent = nullptr);
    explicit ClickImageEvent(QStringList i_fullPaths, int i_anomalyType, QDialog *parent = nullptr);
    ~ClickImageEvent();
    void setWindowSize(int width, int height);
public slots:
    void mousePressEvent(QMouseEvent *press);
private slots:
    /**
     * @brief Function process the input of referencial frame line edit and enables the QGraphicsView if
     * the frame number is correct and frame can be loaded.
     * @param arg1
     */
    void referencialFrameChosen(const QString &arg1);

    /**
     * @brief Function process the chosen video, checks if it is loadable and enables other widgets.
     * @param videoIndex
     */
    void processChosenVideo(int videoIndex);
private:
    /**
     * @brief Function paint the cross using QPainterPath
     * @param path
     * @param x
     * @param y
     */
    void paintCross(QPainterPath &path,double x, double y);

    /**
     * @brief Function loads detected coordinates of frangi filter maximum.
     * @return
     */
    QPointF loadFrangiMaxCoordinates();

    /**
     * @brief Function fills the graphic scene of graphics view with the proper frame at the dialog start up.
     */
    void fillGraphicScene();

    void initCutouts(cv::Mat i_inputFrame);

    void updateCutoutStandard(QPointF i_C, int i_anomalyType);
    void updateCutoutExtra(QPointF i_clickCoordinates,int i_anomalyType);

    Ui::ClickImageEvent *ui;
    QPixmap image;
    QImage  *imageObject = nullptr;
    QGraphicsScene *scene = nullptr;
    QGraphicsView *view = nullptr;
    double height, width;
    QGraphicsPathItem* pathItem = nullptr;
    QGraphicsPathItem* frangiCoords = nullptr;
    QGraphicsRectItem* standardCutout_GRI = nullptr;
    QGraphicsRectItem* extraCutout_GRI = nullptr;
    QRectF standardCutout,extraCutout;
    QString filePath = "";
    int anomalyType = -1;
    double frameCount = 0.0;
    double referencialFrameNo = -1.0;
    bool disabled = false;
    QLabel* selectedVideo = nullptr;
    QLineEdit* referencialFrame_LE = nullptr;
    QComboBox* videoSelection = nullptr;
    QStringList filePaths;
    cv::VideoCapture cap;
};

#endif // CLICKIMAGEEVENT_H
