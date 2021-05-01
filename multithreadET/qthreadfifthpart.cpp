#include "multithreadET/qthreadfifthpart.h"
#include "registration/phase_correlation_function.h"
#include "registration/multiPOC_Ai1.h"
#include "util/files_folders_operations.h"
#include "image_analysis/frangi_utilization.h"
#include "image_analysis/correlation_coefficient.h"
#include "shared_staff/sharedvariables.h"

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
                                   QVector<QString> i_badVideos,
                                   QMap<QString, cv::Rect> i_standardCutout,
                                   QMap<QString, cv::Rect> i_extraCutout,
                                   QMap<QString, QVector<double>> i_POCX,
                                   QMap<QString, QVector<double>> i_POCY,
                                   QMap<QString, QVector<double>> i_Angle,
                                   QMap<QString, QVector<double>> i_Fr_X,
                                   QMap<QString, QVector<double>> i_Fr_Y,
                                   QMap<QString, QVector<double>> i_Fr_E,
                                   cutoutType i_cutoutType,
                                   QMap<QString, QVector<int>> i_EvaluationComplete,
                                   QMap<QString, QVector<int>> i_frEvalSec,
                                   QMap<QString, int> i_referFrames,
                                   int i_iteration,
                                   double i_areaMaximum,
                                   double i_maximalAngle,
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
    selectedCutout = i_cutoutType;
    framesCompleteEvaluation = i_EvaluationComplete;
    framesSecondEval = i_frEvalSec;
    referentialFrames = i_referFrames;
    iteration = i_iteration;
    areaMaximum = i_areaMaximum;
    maximalAngle = i_maximalAngle;
    emit setTerminationEnabled(true);
}

void qThreadFifthPart::run()
{
    // The last part - the coordinates of Frangi Filter maximum pixel are calculated and compared with the coords
    // of referential frame. The euclidean distance of these two pixels can show, if the Frangi filter is usable
    // for this frame.
    emit typeOfMethod(4);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        QString fullPath = videoList.at(videoIndex);
        QString folder,filename,suffix;
        processFilePath(fullPath,folder,filename,suffix);
        qDebug()<<"Processing video "<<filename;
        if (notProcessThese.indexOf(filename) == -1)
        {
            framesToAnalyse = double(framesSecondEval[filename].length());

            QMap<QString,double> FrangiParameters = SharedVariables::getSharedVariables()->getFrangiParameterWrapper(frangiType::VIDEO_SPECIFIC,filename);
            QMap<QString, int> margins;

            emit actualVideo(videoIndex);
            cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
            if (!cap.isOpened())
            {
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(filename);
                continue;
            }
            cv::Mat referentialFrame_temp,referentialFrame,translatedFrame,referentialFrame32f;
            cap.set(CV_CAP_PROP_POS_FRAMES,referentialFrames[filename]);
            if (!cap.read(referentialFrame_temp))
            {
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(filename);
                continue;
            }

            int rows;
            int cols;
            cv::Rect _tempStandard,_tempExtra;
            _tempStandard = obtainedCutoffStandard.contains(filename) ? obtainedCutoffStandard[filename] : cv::Rect(0,0,0,0);
            _tempExtra = obtainedCutoffExtra.contains(filename) ? obtainedCutoffExtra[filename] : cv::Rect(0,0,0,0);

            if (selectedCutout == cutoutType::EXTRA  && (_tempExtra.width > 0 && _tempExtra.height > 0))
            {
                qDebug()<<_tempExtra.width<<" "<<_tempExtra.height;
                scaleChanged = true;
                qDebug()<<"Referential frame size before registration "<<referentialFrame_temp.cols<<" "<<referentialFrame_temp.rows;
                referentialFrame_temp(_tempExtra).copyTo(referentialFrame);
                rows = referentialFrame.rows;
                cols = referentialFrame.cols;
                qDebug()<<"Referential frame 2 size before registration "<<referentialFrame.cols<<" "<<referentialFrame.rows;
            }
            else {
                referentialFrame_temp.copyTo(referentialFrame);
                rows = referentialFrame.rows;
                cols = referentialFrame.cols;
                qDebug()<<"Referential frame 2 size before registration "<<referentialFrame.cols<<" "<<referentialFrame.rows;
                //referentialFrame(_tempStandard).copyTo(referentialFrame_cutout);
            }            
            cv::Point3d pt_temp(0,0,0);
            Point3d frame_FrangiReverse;
            int mode = FrangiParameters.contains("mode") ? FrangiParameters["mode"] : 1;
            qDebug()<<"Applying frangi mode "<<mode;
            frame_FrangiReverse = frangi_analysis(referentialFrame,mode,2,0,"",1,pt_temp,FrangiParameters,margins);
            qDebug()<<"Referential frame size before registration "<<referentialFrame_temp.cols<<" "<<referentialFrame_temp.rows;
            qDebug()<<"Frangi coordinates: "<<frame_FrangiReverse.x<<" "<<frame_FrangiReverse.y;
            qDebug()<<"Analysing "<<framesToAnalyse<<" of "<<filename;
            for (int i = 0; i < framesSecondEval[filename].length(); i++)
            {
                emit percentageCompleted(qRound((videoIndex/videoCount)*100+((i/framesToAnalyse)*100.0)/videoCount));
                Mat registrated = cv::Mat::zeros(referentialFrame_temp.size(), CV_32FC3);
                Point3d calculatedTranslation(0.0,0.0,0.0);
                QVector<double> pocX;
                QVector<double> pocY;
                QVector<double> fullAngle;

                int registrartionDone = completeRegistration(cap,
                                                             referentialFrame,
                                                             translatedFrame,
                                                             framesSecondEval[filename][i],
                                                             iteration,
                                                             areaMaximum,
                                                             maximalAngle,
                                                             _tempExtra,
                                                             _tempStandard,
                                                             scaleChanged,
                                                             registrated,
                                                             pocX,pocY,fullAngle);
                translatedFrame.release();
                if (registrartionDone == 0)
                {
                    emit unexpectedTermination(videoIndex,"hardError");
                    framesCompleteEvaluation[filename][framesSecondEval[filename][i]] = 5.0;
                    POC_x[filename][framesSecondEval[filename][i]] = 999.0;
                    POC_y[filename][framesSecondEval[filename][i]] = 999.0;
                    angle[filename][framesSecondEval[filename][i]] = 999.0;
                    frangi_x[filename][framesSecondEval[filename][i]] = 999.0;
                    frangi_y[filename][framesSecondEval[filename][i]] = 999.0;
                    frangi_euklid[filename][framesSecondEval[filename][i]] = 999.0;
                    fillEmpty(filename);
                    continue;
                }
                else
                {
                    qDebug() << framesSecondEval[filename][i] <<" -> ";

                    //qDebug()<<"Referential frame size after registration "<<referentialFrame.cols<<" "<<referentialFrame.rows;
                    //qDebug()<<"Registrated frame size after registration "<<registrated.cols<<" "<<registrated.rows;
                    double R1;
                    R1 = calculateCorrCoef(referentialFrame,registrated,_tempStandard);
                    Point3d registrationCorrection(0,0,0);
                    if (selectedCutout != cutoutType::NO_CUTOUT)
                    {
                        registrationCorrection = pc_translation(referentialFrame,registrated,areaMaximum);
                        if (std::abs(registrationCorrection.x)>=290.0 || std::abs(registrationCorrection.y)>=290.0)
                        {
                            registrationCorrection = pc_translation_hann(referentialFrame,registrated,areaMaximum);
                        }
                    }
                    else
                    {
                        registrationCorrection = pc_translation_hann(referentialFrame,registrated,areaMaximum);
                    }
                    Mat correctionMat = frameTranslation(registrated,registrationCorrection,rows,cols);
                    double R2 = calculateCorrCoef(referentialFrame,correctionMat,_tempStandard);
                    Point3d registratedFrangiReverse(0,0,0);
                    double difference = R2-R1;
                    if (difference>0.015)
                    {
                        cv::Point3d extra_translace(0,0,0);
                        extra_translace.x = calculatedTranslation.x+registrationCorrection.x;
                        extra_translace.y = calculatedTranslation.y+registrationCorrection.y;
                        extra_translace.z = calculatedTranslation.z;
                        qDebug()<< "Frame was translated for more objective frangi filter analysis.";
                        registratedFrangiReverse = frangi_analysis(correctionMat,mode,2,0,"",2,extra_translace,
                                                                   FrangiParameters,margins);
                    }
                    else
                    {
                        registratedFrangiReverse = frangi_analysis(registrated,mode,2,0,"",2,calculatedTranslation,
                                                                   FrangiParameters,margins);
                    }
                    registrated.release();
                    if (registratedFrangiReverse.z == 0.0)
                    {
                        emit unexpectedTermination(videoIndex,"hardError");
                        framesCompleteEvaluation[filename][framesSecondEval[filename][i]] = 5.0;
                        POC_x[filename][framesSecondEval[filename][i]] = 999.0;
                        POC_y[filename][framesSecondEval[filename][i]] = 999.0;
                        angle[filename][framesSecondEval[filename][i]] = 999.0;
                        frangi_x[filename][framesSecondEval[filename][i]] = 999.0;
                        frangi_y[filename][framesSecondEval[filename][i]] = 999.0;
                        frangi_euklid[filename][framesSecondEval[filename][i]] = 999.0;
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
                            framesCompleteEvaluation[filename][framesSecondEval[filename][i]] = 0;
                            qDebug()<< "euclidean "<<euklid<<"-> category: 0";
                        }
                        else if (euklid > 1.2 && euklid < 10)
                        {
                            framesCompleteEvaluation[filename][framesSecondEval[filename][i]] = 1;
                            qDebug()<< "euclidean "<<euklid<<", category: 1";
                            frangi_x[filename][framesSecondEval[filename][i]] = registratedFrangiReverse.x;
                            frangi_y[filename][framesSecondEval[filename][i]] = registratedFrangiReverse.y;
                            frangi_euklid[filename][framesSecondEval[filename][i]] = euklid;
                        }
                        else if (euklid >=10)
                        {
                            framesCompleteEvaluation[filename][framesSecondEval[filename][i]] = 4;
                            qDebug()<< "euclidean "<<euklid<<", category: 4";
                            frangi_x[filename][framesSecondEval[filename][i]] = registratedFrangiReverse.x;
                            frangi_y[filename][framesSecondEval[filename][i]] = registratedFrangiReverse.y;
                            frangi_euklid[filename][framesSecondEval[filename][i]] = euklid;
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
            fillEmpty(filename);
        }
    }
    emit percentageCompleted(100);
    emit done(5);
}

void qThreadFifthPart::fillEmpty(QString i_videoName){
    /*QVector<double> pomVecD(i_frameCount,0.0);
    QVector<int> pomVecI(i_frameCount,0);

    framesCompleteEvaluation.insert(i_videoName,pomVecI);
    frangi_x.insert(i_videoName,pomVecD);
    frangi_y.insert(i_videoName,pomVecD);
    frangi_euklid.insert(i_videoName,pomVecD);
    POC_x.insert(i_videoName,pomVecD);
    POC_y.insert(i_videoName,pomVecD);
    angle.insert(i_videoName,pomVecD);*/
    notProcessThese.push_back(i_videoName);
}

QVector<QString> qThreadFifthPart::unprocessableVideos() {
    return notProcessThese;
}

QMap<QString, QVector<int>> qThreadFifthPart::framesUpdateEvaluationComplete()
{
    return framesCompleteEvaluation;
}
QMap<QString,QVector<double>> qThreadFifthPart::framesFrangiXestimated()
{
    return frangi_x;
}
QMap<QString,QVector<double>> qThreadFifthPart::framesFrangiYestimated()
{
    return frangi_y;
}
QMap<QString,QVector<double>> qThreadFifthPart::framesFrangiEuklidestimated()
{
    return frangi_euklid;
}
QMap<QString,QVector<double>> qThreadFifthPart::framesPOCXestimated()
{
    return POC_x;
}
QMap<QString,QVector<double>> qThreadFifthPart::framesPOCYestimated()
{
    return POC_y;
}
QMap<QString,QVector<double>> qThreadFifthPart::framesAngleestimated()
{
    return angle;
}

void qThreadFifthPart::onDataObtained(){
    emit readyForFinish();
}
