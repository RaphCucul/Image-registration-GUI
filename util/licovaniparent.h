#ifndef LICOVANIPARENT_H
#define LICOVANIPARENT_H

#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QProgressBar>
#include <QMap>
#include <QString>
#include <QLabel>
#include <QPointer>

class LicovaniParent : public QWidget
{
    Q_OBJECT
public:
    explicit LicovaniParent(QWidget *parent = nullptr);
signals:

public slots:

protected:
    int licovaniDokonceno = 0;
    QVector<QVector<double>> entropy;
    QVector<QVector<double>> tennengrad;
    QVector<QVector<int>> framesFirstFullCompleteEntropyEvaluation; //snimkyPrvotniOhodnoceniEntropieKomplet;
    QVector<QVector<int>> framesFirstFullCompleteTennengradEvaluation; //snimkyPrvotniOhodnoceniTennengradKomplet;
    QVector<QVector<int>> framesFirstFullCompleteDecision; //snimkyPrvniRozhodovaniKomplet;
    QVector<QVector<int>> framesSecondFullCompleteDecision; //snimkyDruheRozhodovaniKomplet;
    QVector<QVector<int>> framesFinalCompleteDecision; //snimkyOhodnoceniKomplet;
    QVector<QVector<int>> badFramesComplete; //spatnesnimkyohodnocenikomplet
    QVector<QVector<double>> framesFrangiX; //snimkyFrangiX;
    QVector<QVector<double>> framesFrangiY; //snimkyFrangiY;
    QVector<QVector<double>> framesFrangiEuklid; //snimkyFrangiEuklid;
    QVector<QVector<double>> framesPOCX; //snimkyPOCX;
    QVector<QVector<double>> framesPOCY; //snimkyPOCY;
    QVector<QVector<double>> framesAngle; //snimkyUhel;

    QVector<int> horizontalAnomalyPresent;
    QVector<int> verticalAnomalyPresent;
    QStringList videoList;
    QStringList videoListFull;
    QStringList videoListNames;

    bool canceled;
    int processedVideoNo = 0;

   /* QStringList videoParameters = {"entropie","tennengrad","FrangiX","FrangiX","FrangiEuklid","POCX","POCY",
                             "Uhel","Ohodnoceni","PrvotOhodEntropie","PrvotOhodTennengrad"
                             "PrvniRozhod","DruheRozhod","VerticalAnomaly","HorizontalAnomaly"};*/
    QStringList videoParameters = {"FrangiX","FrangiY","FrangiEuklid","POCX","POCY",
                                 "Uhel","Ohodnoceni","VerticalAnomaly","HorizontalAnomaly"};
    QMap<QString,QVector<QVector<double>>> videoParametersDouble;
    QMap<QString,QVector<QVector<int>>> videoParametersInt;
    QMap<QString,QVector<int>> videoAnomalies;
};

#endif // LICOVANIPARENT_H
