#ifndef LICOVANIVIDEA_H
#define LICOVANIVIDEA_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QString>
#include <opencv2/opencv.hpp>
#include <QProgressBar>

namespace Ui {
class LicovaniVidea;
}

class LicovaniVidea : public QWidget
{
    Q_OBJECT

public:
    explicit LicovaniVidea(QWidget *parent = nullptr);
    ~LicovaniVidea();

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
    Ui::LicovaniVidea *ui;    
};

#endif // LICOVANIVIDEA_H
