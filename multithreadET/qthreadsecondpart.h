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
    explicit qThreadSecondPart(QStringList i_videosForAnalysis,
                               QVector<int> i_badVideos,
                               QVector<cv::Rect> i_cutoutStandard,
                               QVector<cv::Rect> i_cutoutExtra,
                               QVector<QVector<int>> i_badFramesCompleteList,
                               QVector<int> i_videoReferencialFramesList,
                               bool i_scaleChange,
                               double i_areaMaximum);
    void run() override;
    QVector<double> vectorForFWHM(QVector<int> &badFrames, int frameCount);
    QVector<double> computedCC();
    QVector<double> computedFWHM();
signals:
    void percentageCompleted(int);
    void typeOfMethod(int);
    void done(int);
    void actualVideo(int);
    void unexpectedTermination(int,QString);
    void readyForFinish();
private:
    /**
     * @brief If error, fill the vectors with zeros.
     */
    void fillEmpty();

    QStringList videoList;
    double videoCount;
    double frameCount;
    double areaMaximum;
    QVector<cv::Rect> obtainedCutoffStandard;//ziskany_VK_standard;
    QVector<cv::Rect> obtainedCutoffExtra;//ziskany_VK_extra;
    QVector<QVector<int>> badFramesComplete;
    bool scaleChanged;
    QVector<double> CC;
    QVector<double> FWHM;
    QVector<int> referencialFrames;
    QVector<int> notProcessThese;
private slots:
    void onDataObtained();
};

#endif // QTHREADSECONDPART_H
