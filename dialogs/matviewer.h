#ifndef MATVIEWER_H
#define MATVIEWER_H

#include <QObject>
#include <QDialog>
#include <QLabel>

#include <opencv2/opencv.hpp>

/**
 * @class MatViewer
 * @brief The MatViewer class object enables to replace cv::imshow function because created window was not able
 * to see when the program runs in "release" mode. Everything is ok in "debug" mode.
 * Everything is set up in the contructor.
 */
class MatViewer : public QDialog
{
    Q_OBJECT
public:
    MatViewer(cv::Mat imageToBeShown, QString dialogLabel);
private:
};

#endif // MATVIEWER_H
