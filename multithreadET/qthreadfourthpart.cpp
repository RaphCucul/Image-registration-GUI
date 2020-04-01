#include "qthreadfourthpart.h"
#include "util/files_folders_operations.h"

#include <QDebug>
#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

qThreadFourthPart::qThreadFourthPart(QStringList i_videos,
                                     QVector<QString> i_badVideos,
                                     QMap<QString, QVector<int> > i_framesFirstEvaluationDefinitive,
                                     QMap<QString, QVector<int> > i_evalCompl,
                                     QMap<QString, QVector<double> > i_CCproblematic,
                                     QMap<QString, QVector<double> > i_FWHMproblematic,
                                     QMap<QString, QVector<double> > i_POCX,
                                     QMap<QString, QVector<double> > i_POCY,
                                     QMap<QString, QVector<double> > i_Angles,
                                     QMap<QString, QVector<double> > i_Fr_X,
                                     QMap<QString, QVector<double> > i_Fr_Y,
                                     QMap<QString, QVector<double> > i_Fr_Eu,
                                     QMap<QString, double> i_averageCC,
                                     QMap<QString, double> i_averageFWHM,
                                     QObject* parent):QThread(parent)
{
    videoList = i_videos;
    framesFirstEvaluationComplete = i_framesFirstEvaluationDefinitive;
    framesCompleteEvaluation = i_evalCompl;
    computedCC = i_CCproblematic;
    computedFWHM = i_FWHMproblematic;
    POC_x = i_POCX;
    POC_y = i_POCY;
    angle = i_Angles;
    frangi_x = i_Fr_X;
    frangi_y = i_Fr_Y;
    frangi_euklid = i_Fr_Eu;
    averageCCcomplete = i_averageCC;
    averageFWHMcomplete = i_averageFWHM;
    notProcessThese = i_badVideos;

    emit setTerminationEnabled(true);
}

void qThreadFourthPart::run()
{
    // Sevent part - based on the previous part, this is the second chance for bad frame to be marked as suitable
    // for registration
    emit typeOfMethod(3);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        QString fullPath = videoList.at(videoIndex);
        QString folder,filename,suffix;
        processFilePath(fullPath,folder,filename,suffix);
        if (notProcessThese.indexOf(filename) == -1){
            QVector<int> snimky_k_provereni_druhy;
            framesToAnalyse = double(framesFirstEvaluationComplete[filename].length());

            emit actualVideo(videoIndex);
            for (int b = 0; b < framesFirstEvaluationComplete[filename].length(); b++)
            {
                emit percentageCompleted(qRound((double(videoIndex)/videoCount)*100.0+((double(b)/framesToAnalyse)*100.0)/videoCount));
                if ((averageCCcomplete[filename] - computedCC[filename][b]) <= 0.01)
                {
                    if (computedFWHM[filename][b] < (averageFWHMcomplete[filename] + 2))
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[filename][b]<< " suitable for registration.";
                        framesCompleteEvaluation[filename][framesFirstEvaluationComplete[filename][b]] = 0;
                    }
                    else
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[filename][b]<< " will be analysed in the next step.";
                        snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[filename][b]);
                    }
                    continue;
                }
                else if ((averageCCcomplete[filename] - computedCC[filename][b]) >0.01)
                {
                    if (computedFWHM[filename][b] < (averageFWHMcomplete[filename] + 2.5))
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[filename][b]<< " suitable for registration.";
                        framesCompleteEvaluation[filename][framesFirstEvaluationComplete[filename][b]] = 0;
                    }
                    else if (computedFWHM[filename][b] > (averageFWHMcomplete[filename] + 10))
                    {
                        qDebug() << "Frame "<< framesFirstEvaluationComplete[filename][b]<< " will not be registrated.";
                        framesCompleteEvaluation[filename][framesFirstEvaluationComplete[filename][b]] = 5.0;
                        POC_x[filename][framesFirstEvaluationComplete[filename][b]] = 999.0;
                        POC_y[filename][framesFirstEvaluationComplete[filename][b]] = 999.0;
                        angle[filename][framesFirstEvaluationComplete[filename][b]] = 999.0;
                        frangi_x[filename][framesFirstEvaluationComplete[filename][b]] = 999.0;
                        frangi_y[filename][framesFirstEvaluationComplete[filename][b]] = 999.0;
                        frangi_euklid[filename][framesFirstEvaluationComplete[filename][b]] = 999.0;
                    }
                    else
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[filename][b]<< " will be analysed in the next step.";
                        snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[filename][b]);
                    }
                    continue;
                }
                else if ((averageCCcomplete[filename] - computedCC[filename][b]) >= 0.05)
                {
                    if (computedFWHM[filename][b] <= (averageFWHMcomplete[filename] + 3))
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[filename][b]<< " suitable for registration.";
                        framesCompleteEvaluation[filename][framesFirstEvaluationComplete[filename][b]] = 0;
                    }
                    else
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[filename][b]<< " will be analysed in the next step.";
                        snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[filename][b]);
                    }
                    continue;
                }
            }
            framesSecondEvaluationComplete.insert(filename,snimky_k_provereni_druhy);
        }
        else{
            fillEmpty(filename,260);
        }
    }
    emit percentageCompleted(100);
    emit done(4);
}

void qThreadFourthPart::fillEmpty(QString i_videoName, int i_frameCount){
    /*QVector<double> pomVecD(i_frameCount,0.0);
    QVector<int> pomVecI(i_frameCount,0);

    framesCompleteEvaluation.insert(i_videoName,pomVecI);
    framesSecondEvaluationComplete.insert(i_videoName,pomVecI);
    frangi_x.insert(i_videoName,pomVecD);
    frangi_y.insert(i_videoName,pomVecD);
    frangi_euklid.insert(i_videoName,pomVecD);
    POC_x.insert(i_videoName,pomVecD);
    POC_y.insert(i_videoName,pomVecD);
    angle.insert(i_videoName,pomVecD);*/
    notProcessThese.push_back(i_videoName);
}

QVector<QString> qThreadFourthPart::unprocessableVideos() {
    return notProcessThese;
}

QMap<QString,QVector<int>> qThreadFourthPart::framesUpdateEvaluationComplete()
{
    return framesCompleteEvaluation;
}
QMap<QString, QVector<int>> qThreadFourthPart::framesSecondEvaluation()
{
    return framesSecondEvaluationComplete;
}
QMap<QString, QVector<double>> qThreadFourthPart::framesFrangiXestimated()
{
    return frangi_x;
}
QMap<QString,QVector<double>> qThreadFourthPart::framesFrangiYestimated()
{
    return frangi_y;
}
QMap<QString, QVector<double> > qThreadFourthPart::framesFrangiEuklidestimated()
{
    return frangi_euklid;
}
QMap<QString,QVector<double>> qThreadFourthPart::framesPOCXestimated()
{
    return POC_x;
}
QMap<QString, QVector<double> > qThreadFourthPart::framesPOCYestimated()
{
    return POC_y;
}
QMap<QString,QVector<double>> qThreadFourthPart::framesAngleestimated()
{
    return angle;
}

void qThreadFourthPart::onDataObtained(){
    emit readyForFinish();
}
