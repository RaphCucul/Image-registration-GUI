#ifndef SINGLEVIDEOLICOVANI_H
#define SINGLEVIDEOLICOVANI_H

#include "util/licovaniparent.h"
#include "licovani/registrationthread.h"
#include <QWidget>

namespace Ui {
class SingleVideoLicovani;
}

class SingleVideoLicovani : public LicovaniParent
{
    Q_OBJECT

public:
    explicit SingleVideoLicovani(QWidget *parent = nullptr);
    ~SingleVideoLicovani();
    void processVideoParameters(QJsonObject& videoData);
private slots:
    void on_vybraneVideoLE_textChanged(const QString &arg1);
    void on_vyberVideaPB_clicked();
    void slicovatSnimkyVidea();
    void createAndRunThreads(int indexProcVid, cv::VideoCapture& cap, int lowerLimit,
                             int upperLimit);
private:
    Ui::SingleVideoLicovani *ui;
    QString fullVideoPath;
    QWidget* widgetForScroll;
    QVector<QString> vybraneVideoLicovaniSingle;
    QVector<QString> vybranyJSONLicovaniSingle;
    QList<QPointer<RegistrationThread>> registrationThreadsList;
    QJsonObject videoParametersJson;
};

#endif // SINGLEVIDEOLICOVANI_H
