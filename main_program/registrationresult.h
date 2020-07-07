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

/**
 * @class RegistrationResult
 * @brief The RegistrationResult class is a simple dialog showing registration result - it can be a video or just two frames. It
 * depends when the program is called. It is not stand-alone dialog, it is always called from the program by some algorithm.
 * For example registrated videos can be viewed in the moment they are created. Not later.
 */
class RegistrationResult : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationResult(cv::Mat &_referencial, cv::Mat &_translated, QDialog *parent = nullptr);
    ~RegistrationResult();
    /**
     * @brief Sets the size of the dialog.
     * @param width
     * @param height
     */
    void setWindowsSize(int width, int height);

    /**
     * @brief Initializes necessary variables with information about two frames which will be compared.
     */
    void displayTwo();

    /**
     * @brief Initializes necessary variables with information about video and a frame.
     * @param video
     */
    void displayVideo(cv::VideoCapture video);

    /**
     * @brief Request the initialization of "two frames result" layout.
     * @sa displayTwo()
     */
    void callTwo();

    /**
     * @brief Request the initialization of "video result" layout.
     * @sa displayVideo(cv::VideoCapture video)
     */
    void callVideo();

    /**
     * @brief Initializes widgets to display the referential frame when comparing two or first frame when controling
     * video registration.
     * @param startMethod - frames (1) or video (2)
     */
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
