#ifndef REGISTRATIONRESULT_H
#define REGISTRATIONRESULT_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>

#include <opencv2/opencv.hpp>

namespace Ui {
class RegistrationResult;
}

class RegistrationResult : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationResult(cv::Mat &_referencial, cv::Mat &_translated, QDialog *parent = nullptr);
    ~RegistrationResult();
    void setWindowsSize(int sirka, int vyska);
    void displayTwo();
    void displayVideo(cv::VideoCapture video);
    void callTwo();
    void callVideo();
    void start(int startMethod);
private slots:
    void changeDisplayed(int value);
private:


    Ui::RegistrationResult *ui;
    int wantToDisplay = 0;
    QPixmap image;
    QImage *imageReference;
    QImage *imageSlicovany;
    QImage *actualFrame;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsWidget* graphicalWidget;
    cv::Mat referencialFrame, registratedFrame, frame0;
    cv::VideoCapture actualVideo;
};

#endif // REGISTRATIONRESULT_H
