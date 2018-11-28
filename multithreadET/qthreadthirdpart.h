#ifndef QTHREADTHIRDPART_H
#define QTHREADTHIRDPART_H

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

#include <opencv2/opencv.hpp>

class qThreadThirdPart : public QThread
{
    Q_OBJECT
public:
    explicit qThreadThirdPart(QStringList& sV, QVector<QVector<int>>& sSprOhKomplet,QVector<QVector<int>>& ohodKomp,
                     QVector<int>& refKomplet,QVector<double>& RK, QVector<double>& FWHMK,cv::Rect& VK_standard, cv::Rect& VK_extra,
                     bool zM, QObject* parent=nullptr);
    void run() override;
    QVector<QVector<int>> snimkyUpdateOhodnoceni();
    QVector<QVector<int>> snimkyProvereniPrvniKompletestimated();
    QVector<QVector<double>> snimkyProblematicke_R();
    QVector<QVector<double>> snimkyProblematicke_FWHM();
    QVector<QVector<double>> snimkyFrangiXestimated();
    QVector<QVector<double>> snimkyFrangiYestimated();
    QVector<QVector<double>> snimkyFrangiEuklidestimated();
    QVector<QVector<double>> snimkyPOCXestimated();
    QVector<QVector<double>> snimkyPOCYestimated();
    QVector<QVector<double>> snimkyUhelestimated();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void hotovo(int);
    void actualVideo(int);
    void unexpectedTermination();
private:
    QStringList seznamVidei;
    QVector<double> prumernyKKkomplet;
    QVector<double> prumernyFWHMkomplet;
    QVector<QVector<int>> spatne_snimky_prvotni_ohodnoceni;
    QVector<QVector<int>> ohodnoceniSnimkuKomplet;
    QVector<QVector<int>> snimkyProvereniPrvniKomplet;
    QVector<QVector<double>> snimkyVypocteneR,snimkyVypocteneFWHM;
    QVector<int> referenceKompletni;
    cv::Rect vyrez_korelace_standard;
    cv::Rect vyrez_korelace_extra;
    bool zmenaMeritka;
    QVector<QVector<double>> snimkyFrangiX;
    QVector<QVector<double>> snimkyFrangiY;
    QVector<QVector<double>> snimkyFrangiEuklid;
    QVector<QVector<double>> snimkyPOCX;
    QVector<QVector<double>> snimkyPOCY;
    QVector<QVector<double>> snimkyUhel;
    double pocetVidei;
    double pocetSnimku;
};

#endif // QTHREADTHIRDPART_H
