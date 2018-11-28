#ifndef QTHREADFIRSTPART_H
#define QTHREADFIRSTPART_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QStringList>

class qThreadFirstPart : public QThread
{
    Q_OBJECT
public:
   explicit qThreadFirstPart(QStringList videaKanalyza, cv::Point2f souradniceAnomalie,
                                   cv::Point2f souradniceCasZnac,QVector<double>PF,
                                   bool An, bool CZ, QObject* parent=nullptr);
    QVector<QVector<double>> vypocitanaEntropie();
    QVector<QVector<double>> vypocitanyTennengrad();
    QVector<QVector<int>> vypocitanePrvotniOhodnoceniEntropie();
    QVector<QVector<int>> vypocitanePrvotniOhodnoceniTennengrad();
    QVector<QVector<int>> vypocitaneKompletniOhodnoceni();
    QVector<QVector<int>> vypocitaneSpatneSnimkyKomplet();
    cv::Rect vypocitanyVKstandard();
    cv::Rect vypocitanyVKextra();
    QVector<int> urceneReferenceVidei();    
    void run() override;

private:
    QVector<QVector<double>> entropieKomplet;
    QVector<QVector<double>> tennengradKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> snimkyKompletniOhodnoceni;
    QVector<QVector<int>> spatneSnimkyKomplet;
    QVector<int> snimkyReferencni;    
    QVector<double> parametryFrangi;
    QStringList zpracujVidea;
    int pocetVidei;
    double procento;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point2f ziskane_hranice_CasZnac;
    cv::Rect ziskany_VK_standard;
    cv::Rect ziskany_VK_extra;
    bool AnomalieBool;
    bool CasZnacBool;

signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void hotovo(int);
    void actualVideo(int);
    void unexpectedTermination();
};

#endif // QTHREADFIRSTPART_H
