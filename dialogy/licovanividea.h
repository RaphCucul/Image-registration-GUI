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

private:
    Ui::LicovaniVidea *ui;    
};

#endif // LICOVANIVIDEA_H
