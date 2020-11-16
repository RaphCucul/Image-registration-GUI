#include "matviewer.h"
#include "image_analysis/image_processing.h"

#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>

MatViewer::MatViewer(cv::Mat imageToBeShown, QString dialogLabel)
{
    int imageFormat = 0;
    cv::Mat processed;
    if (MatType2String(imageToBeShown.type()).contains("8U")) {
        transformMatTypeTo8C3(imageToBeShown);
        imageFormat = 13;
    }
    else if (MatType2String(imageToBeShown.type()).contains("32F")) {
        processed = convertMatFrom32FTo8U(imageToBeShown);
        imageFormat = 24;
        processed.copyTo(imageToBeShown);
    }
    QImage *_image = new QImage(imageToBeShown.data,
                               imageToBeShown.cols,
                               imageToBeShown.rows,
                                static_cast<int>(imageToBeShown.step),
                                QImage::Format(imageFormat));
    QHBoxLayout* layout = new QHBoxLayout;
    QGraphicsView *_view = new QGraphicsView;
    QGraphicsScene *_scene = new QGraphicsScene;
    _view->setScene(_scene);
    QPixmap pixmap = QPixmap::fromImage(*_image);
    QGraphicsPixmapItem *_item = new QGraphicsPixmapItem(pixmap);

    _view->scene()->addItem(_item);
    layout->addWidget(_view);
    this->setLayout(layout);
    this->updateGeometry();
    this->setWindowTitle(dialogLabel);
}
