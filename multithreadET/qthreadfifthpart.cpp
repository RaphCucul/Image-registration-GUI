#include "multithreadET/qthreadfifthpart.h"
#include "registration/phase_correlation_function.h"
#include "registration/multiPOC_Ai1.h"
#include "util/files_folders_operations.h"
#include "image_analysis/frangi_utilization.h"
#include "image_analysis/correlation_coefficient.h"

#include <QThread>
#include <QStringList>
#include <QVector>
#include <QDebug>

#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using cv::Mat;
using cv::Rect;
using cv::Point3d;

qThreadFifthPart::qThreadFifthPart(QStringList i_videos,
                                   QVector<int> i_badVideos,
                                   QVector<cv::Rect> i_standardCutout,
                                   QVector<cv::Rect> i_extraCutout,
                                   QVector<QVector<double> > i_POCX,
                                   QVector<QVector<double> > i_POCY,
                                   QVector<QVector<double> > i_Angle,
                                   QVector<QVector<double> > i_Fr_X,
                                   QVector<QVector<double> > i_Fr_Y,
                                   QVector<QVector<double> > i_Fr_E,
                                   bool i_scaleChanged,
                                   QVector<QVector<int> > i_EvaluationComplete,
                                   QVector<QVector<int> > i_frEvalSec,
                                   QVector<int> i_referFrames,
                                   QVector<double> i_FrangiParams,
                                   int i_iteration,
                                   double i_areaMaximum,
                                   double i_maximalAngle,
                                   QMap<QString, int> i_margins,
                                   QObject *parent):QThread(parent)
{
    videoList = i_videos;
    notProcessThese = i_badVideos;
    obtainedCutoffStandard = i_standardCutout;
    obtainedCutoffExtra = i_extraCutout;
    POC_x = i_POCX;
    POC_y = i_POCY;
    angle = i_Angle;
    frangi_x = i_Fr_X;
    frangi_y = i_Fr_Y;
    frangi_euklid = i_Fr_E;
    scaleCh = i_scaleChanged;
    framesCompleteEvaluation = i_EvaluationComplete;
    framesSecondEval = i_frEvalSec;
    FrangiParameters = i_FrangiParams;
    referencialFrames = i_referFrames;
    iteration = i_iteration;
    areaMaximum = i_areaMaximum;
    maximalAngle = i_maximalAngle;

    margins = i_margins;

    emit setTerminationEnabled(true);
}

void qThreadFifthPart::run()
{
    /// The last part - the coordinates of Frangi Filter maximum pixel are calculated and compared with the coords
    /// of referencial frame. The euclidean distance of these two pixels can show, if the Frangi filter is usable
    /// for this frame.
    emit typeOfMethod(4);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    qDebug()<<videoCount<<" videos ready for the analysis.";
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        if (notProcessThese.indexOf(videoIndex) == -1){
            framesToAnalyse = double(framesSecondEval[videoIndex].length());
            QString fullPath = videoList.at(videoIndex);
            QString folder,filename,suffix;
            processFilePath(fullPath,folder,filename,suffix);
            emit actualVideo(videoIndex);
            cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
            if (!cap.isOpened())
            {
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(260);
                continue;
            }
            cv::Mat referencialFrame_temp,referencialFrame,referencialFrame32f,referencialFrame_cutout;
            cap.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[videoIndex]);
            if (!cap.read(referencialFrame_temp))
            {
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(260);
                continue;
            }
            int videoFrameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
            int rows;
            int cols;
            cv::Rect _tempStandard,_tempExtra;
            _tempStandard = obtainedCutoffStandard[videoIndex];
            _tempExtra = obtainedCutoffExtra[videoIndex];

            if (scaleCh == true)
            {
                referencialFrame_temp(_tempExtra).copyTo(referencialFrame);
                rows = referencialFrame.rows;
                cols = referencialFrame.cols;
                referencialFrame(_tempStandard).copyTo(referencialFrame_cutout);
                referencialFrame_temp.release();
            }
            else
            {
                referencialFrame_temp.copyTo(referencialFrame);
                rows = referencialFrame.rows;
                cols = referencialFrame.cols;
                referencialFrame(_tempStandard).copyTo(referencialFrame_cutout);
                referencialFrame_temp.release();
            }

            cv::Point3d pt_temp(0,0,0);
            Point3d frame_FrangiReverse = frangi_analysis(referencialFrame,2,2,0,"",1,pt_temp,FrangiParameters,margins);
            referencialFrame(_tempStandard).copyTo(referencialFrame_cutout);
            qDebug()<<"Analysing "<<framesToAnalyse<<" of "<<filename;
            for (int i = 0; i < framesSecondEval[videoIndex].length(); i++)
            {
                emit percentageCompleted(qRound((videoIndex/videoCount)*100+((i/framesToAnalyse)*100.0)/videoCount));
                Mat registrated = cv::Mat::zeros(referencialFrame.size(), CV_32FC3);
                Point3d calculatedTranslation(0.0,0.0,0.0);
                QVector<double> pocX;
                QVector<double> pocY;
                QVector<double> fullAngle;
                int registrartionDone = completeRegistration(cap,referencialFrame,
                                                           framesSecondEval[videoIndex][i],
                                                           iteration,
                                                           areaMaximum,
                                                           maximalAngle,
                                                           _tempExtra,
                                                           _tempStandard,
                                                           scaleCh,
                                                           registrated,
                                                           pocX,pocY,fullAngle);
                qDebug() << framesSecondEval[videoIndex][i] <<" -> ";
                if (registrartionDone == 0)
                {

                    emit unexpectedTermination(videoIndex,"hardError");
                    framesCompleteEvaluation[videoIndex][framesSecondEval[videoIndex][i]] = 5.0;
                    POC_x[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                    POC_y[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                    angle[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                    frangi_x[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                    frangi_y[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                    frangi_euklid[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                    fillEmpty(videoFrameCount);
                    continue;
                }
                else
                {
                    Mat interresult32f,interresult32f_cutout;
                    registrated.copyTo(interresult32f);
                    transformMatTypeTo32C1(interresult32f);
                    interresult32f(_tempStandard).copyTo(interresult32f_cutout);
                    double R1 = calculateCorrCoef(referencialFrame,registrated,_tempStandard);
                    interresult32f.release();
                    interresult32f_cutout.release();
                    Point3d registrationCorrection(0,0,0);
                    if (scaleCh == true)
                    {
                        registrationCorrection = pc_translation(referencialFrame,registrated,areaMaximum);
                        if (std::abs(registrationCorrection.x)>=290.0 || std::abs(registrationCorrection.y)>=290.0)
                        {
                            registrationCorrection = pc_translation_hann(referencialFrame,registrated,areaMaximum);
                        }
                    }
                    else
                    {
                        registrationCorrection = pc_translation_hann(referencialFrame,registrated,areaMaximum);
                    }
                    Mat correctionMat = frameTranslation(registrated,registrationCorrection,rows,cols);
                    correctionMat.copyTo(interresult32f);
                    transformMatTypeTo32C1(interresult32f);
                    interresult32f(_tempStandard).copyTo(interresult32f_cutout);
                    double R2 = calculateCorrCoef(referencialFrame,correctionMat,_tempStandard);
                    Point3d registratedFrangiReverse(0,0,0);
                    double difference = R2-R1;
                    if (difference>0.015)
                    {
                        cv::Point3d extra_translace(0,0,0);
                        extra_translace.x = calculatedTranslation.x+registrationCorrection.x;
                        extra_translace.y = calculatedTranslation.y+registrationCorrection.y;
                        extra_translace.z = calculatedTranslation.z;
                        qDebug()<< "Frame was translated for more objective frangi filter analysis.";
                        registratedFrangiReverse = frangi_analysis(correctionMat,2,2,0,"",2,extra_translace,
                                                                   FrangiParameters,margins);
                    }
                    else
                    {
                        registratedFrangiReverse = frangi_analysis(registrated,2,2,0,"",2,calculatedTranslation,
                                                                   FrangiParameters,margins);
                    }
                    registrated.release();
                    if (registratedFrangiReverse.z == 0.0)
                    {
                        emit unexpectedTermination(videoIndex,"hardError");
                        framesCompleteEvaluation[videoIndex][framesSecondEval[videoIndex][i]] = 5.0;
                        POC_x[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                        POC_y[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                        angle[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                        frangi_x[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                        frangi_y[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                        frangi_euklid[videoIndex][framesSecondEval[videoIndex][i]] = 999.0;
                        continue;
                    }
                    else
                    {
                        double difference_x = frame_FrangiReverse.x - registratedFrangiReverse.x;
                        double difference_y = frame_FrangiReverse.y - registratedFrangiReverse.y;
                        double differenceSum = std::pow(difference_x,2.0) + std::pow(difference_y,2.0);
                        double euklid = std::sqrt(differenceSum);
                        if (euklid <= 1.2)
                        {
                            framesCompleteEvaluation[videoIndex][framesSecondEval[videoIndex][i]] = 0;
                            qDebug()<< "euclidean "<<euklid<<"-> category: 0";
                        }
                        else if (euklid > 1.2 && euklid < 10)
                        {
                            framesCompleteEvaluation[videoIndex][framesSecondEval[videoIndex][i]] = 1;
                            qDebug()<< "euclidean "<<euklid<<", category: 1";
                            frangi_x[videoIndex][framesSecondEval[videoIndex][i]] = registratedFrangiReverse.x;
                            frangi_y[videoIndex][framesSecondEval[videoIndex][i]] = registratedFrangiReverse.y;
                            frangi_euklid[videoIndex][framesSecondEval[videoIndex][i]] = euklid;
                        }
                        else if (euklid >=10)
                        {
                            framesCompleteEvaluation[videoIndex][framesSecondEval[videoIndex][i]] = 4;
                            qDebug()<< "euclidean "<<euklid<<", category: 4";
                            frangi_x[videoIndex][framesSecondEval[videoIndex][i]] = registratedFrangiReverse.x;
                            frangi_y[videoIndex][framesSecondEval[videoIndex][i]] = registratedFrangiReverse.y;
                            frangi_euklid[videoIndex][framesSecondEval[videoIndex][i]] = euklid;
                        }
                        difference_x = 0;
                        difference_y = 0;
                        differenceSum = 0;
                        euklid = 0;
                    }
                }
            }
        }
        else{
            fillEmpty(260);
        }
    }
    emit percentageCompleted(100);
    emit done(5);
}

void qThreadFifthPart::fillEmpty(int i_frameCount){
    QVector<double> pomVecD(i_frameCount,0.0);
    QVector<int> pomVecI(i_frameCount,0);

    framesCompleteEvaluation.push_back(pomVecI);
    frangi_x.push_back(pomVecD);
    frangi_y.push_back(pomVecD);
    frangi_euklid.push_back(pomVecD);
    POC_x.push_back(pomVecD);
    POC_y.push_back(pomVecD);
    angle.push_back(pomVecD);
}

QVector<QVector<int>> qThreadFifthPart::framesUpdateEvaluationComplete()
{
    return framesCompleteEvaluation;
}
QVector<QVector<double>> qThreadFifthPart::framesFrangiXestimated()
{
    return frangi_x;
}
QVector<QVector<double>> qThreadFifthPart::framesFrangiYestimated()
{
    return frangi_y;
}
QVector<QVector<double>> qThreadFifthPart::framesFrangiEuklidestimated()
{
    return frangi_euklid;
}
QVector<QVector<double>> qThreadFifthPart::framesPOCXestimated()
{
    return POC_x;
}
QVector<QVector<double>> qThreadFifthPart::framesPOCYestimated()
{
    return POC_y;
}
QVector<QVector<double>> qThreadFifthPart::framesAngleestimated()
{
    return angle;
}

void qThreadFifthPart::onDataObtained(){
    emit readyForFinish();
}
