#ifndef MATVIEWER_H
#define MATVIEWER_H

#include <QObject>
#include <QDialog>
#include <QLabel>

#include <opencv2/opencv.hpp>

/**
 * @class MatViewer
 * @brief The MatViewer class object replaces cv::imshow function because created window was not visible
 *  in the "release" mode.
 *
 * Necessary information is provided in the class constructor.
 */
class MatViewer : public QDialog
{
    Q_OBJECT
public:
    MatViewer(cv::Mat imageToBeShown, QString dialogLabel);
private:
};

#endif // MATVIEWER_H
