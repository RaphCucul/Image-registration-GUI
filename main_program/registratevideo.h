#ifndef REGISTRATEVIDEO_H
#define REGISTRATEVIDEO_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QString>
#include <opencv2/opencv.hpp>
#include <QProgressBar>

namespace Ui {
class RegistrateVideo;
}

class RegistrateVideo : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrateVideo(QWidget *parent = nullptr);
    ~RegistrateVideo();

    /**
     * @brief Function calls checkPath() functions for single ET analysis and single registration.
     */
    void checkPathinitialization();
signals:
    void calculationStarted();
    void calculationStopped();
    void checkFilePaths();
private slots:
    void enableTabs();
    void disableTabs();
private:
    Ui::RegistrateVideo *ui;
};

#endif // REGISTRATEVIDEO_H
