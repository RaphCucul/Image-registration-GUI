#ifndef QTHREADSECONDPART_H
#define QTHREADSECONDPART_H

#include <QThread>
#include <QObject>
#include <QVector>
#include <opencv2/opencv.hpp>
#include <QStringList>

class qThreadSecondPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadSecondPart(QStringList& seznamVidei,cv::Rect& VK_standard,cv::Rect& VK_extra,
                               QVector<QVector<int>>& spatneSnimkyVideiKomplet,QVector<int>& seznamReferenci,
                               bool zM);
    void run() override;
    QVector<double> vectorForFWHM(QVector<int> &spatne_snimky_komplet, int pocet_snimku_videa);
    QVector<double> vypoctenyR();
    QVector<double> vypocteneFWHM();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void hotovo(int);
    void actualVideo(int);
    void unexpectedTermination();
private:
    QStringList sezVid;
    double pocetVidei;
    double pocetSnimku;
    cv::Rect vyrez_korelace_standard;
    cv::Rect vyrez_korelace_extra;
    QVector<QVector<int>> spatne_snimky_komplet;
    bool zmenaMeritka;
    QVector<double> R;
    QVector<double> FWHM;
    QVector<int> referenceVidei;
};

#endif // QTHREADSECONDPART_H
