#include "qthreadfourthpart.h"
#include "util/souborove_operace.h"

#include <QDebug>
#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

qThreadFourthPart::qThreadFourthPart(QStringList i_videos,
                                     QVector<int> i_badVideos,
                                     QVector<QVector<int> > i_framesFirstEvaluationDefinitive,
                                     QVector<QVector<int> > i_evalCompl,
                                     QVector<QVector<double> > i_CCproblematic,
                                     QVector<QVector<double> > i_FWHMproblematic,
                                     QVector<QVector<double> > i_POCX,
                                     QVector<QVector<double> > i_POCY,
                                     QVector<QVector<double> > i_Angles,
                                     QVector<QVector<double> > i_Fr_X,
                                     QVector<QVector<double> > i_Fr_Y,
                                     QVector<QVector<double> > i_Fr_Eu,
                                     QVector<double> i_averageCC,
                                     QVector<double> i_averageFWHM,
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
    /// Sevent part - based on the previous part, this is the second chance for bad frame to be marked as suitable
    /// for registration
    emit typeOfMethod(3);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        if (notProcessThese.indexOf(videoIndex) == -1){
            QVector<int> snimky_k_provereni_druhy;
            framesToAnalyse = double(framesFirstEvaluationComplete[videoIndex].length());
            QString fullPath = videoList.at(videoIndex);
            QString folder,filename,suffix;
            processFilePath(fullPath,folder,filename,suffix);
            emit actualVideo(videoIndex);
            for (int b = 0; b < framesFirstEvaluationComplete[videoIndex].length(); b++)
            {
                emit percentageCompleted(qRound((double(videoIndex)/videoCount)*100.0+((double(b)/framesToAnalyse)*100.0)/videoCount));
                if ((averageCCcomplete[videoIndex] - computedCC[videoIndex][b]) <= 0.01)
                {
                    if (computedFWHM[videoIndex][b] < (averageFWHMcomplete[videoIndex] + 2))
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " suitable for registration.";
                        framesCompleteEvaluation[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 0;
                    }
                    else
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " will be analysed in the next step.";
                        snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[videoIndex][b]);
                    }
                    continue;
                }
                else if ((averageCCcomplete[videoIndex] - computedCC[videoIndex][b]) >0.01)
                {
                    if (computedFWHM[videoIndex][b] < (averageFWHMcomplete[videoIndex] + 2.5))
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " suitable for registration.";
                        framesCompleteEvaluation[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 0;
                    }
                    else if (computedFWHM[videoIndex][b] > (averageFWHMcomplete[videoIndex] + 10))
                    {
                        qDebug() << "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " will not be registrated.";
                        framesCompleteEvaluation[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 5.0;
                        POC_x[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 999.0;
                        POC_y[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 999.0;
                        angle[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 999.0;
                        frangi_x[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 999.0;
                        frangi_y[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 999.0;
                        frangi_euklid[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 999.0;
                    }
                    else
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " will be analysed in the next step.";
                        snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[videoIndex][b]);
                    }
                    continue;
                }
                else if ((averageCCcomplete[videoIndex] - computedCC[videoIndex][b]) >= 0.05)
                {
                    if (computedFWHM[videoIndex][b] <= (averageFWHMcomplete[videoIndex] + 3))
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " suitable for registration.";
                        framesCompleteEvaluation[videoIndex][framesFirstEvaluationComplete[videoIndex][b]] = 0;
                    }
                    else
                    {
                        qDebug()<< "Frame "<< framesFirstEvaluationComplete[videoIndex][b]<< " will be analysed in the next step.";
                        snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[videoIndex][b]);
                    }
                    continue;
                }
            }
            framesSecondEvaluationComplete.append(snimky_k_provereni_druhy);
        }
        else{
            fillEmpty(260);
        }
    }
    emit percentageCompleted(100);
    emit done(4);
}

void qThreadFourthPart::fillEmpty(int i_frameCount){
    QVector<double> pomVecD(i_frameCount,0.0);
    QVector<int> pomVecI(i_frameCount,0);

    framesCompleteEvaluation.push_back(pomVecI);
    framesSecondEvaluationComplete.push_back(pomVecI);
    frangi_x.push_back(pomVecD);
    frangi_y.push_back(pomVecD);
    frangi_euklid.push_back(pomVecD);
    POC_x.push_back(pomVecD);
    POC_y.push_back(pomVecD);
    angle.push_back(pomVecD);
}

QVector<QVector<int>> qThreadFourthPart::framesUpdateEvaluationComplete()
{
    return framesCompleteEvaluation;
}
QVector<QVector<int>> qThreadFourthPart::framesSecondEvaluation()
{
    return framesSecondEvaluationComplete;
}
QVector<QVector<double>> qThreadFourthPart::framesFrangiXestimated()
{
    return frangi_x;
}
QVector<QVector<double>> qThreadFourthPart::framesFrangiYestimated()
{
    return frangi_y;
}
QVector<QVector<double>> qThreadFourthPart::framesFrangiEuklidestimated()
{
    return frangi_euklid;
}
QVector<QVector<double>> qThreadFourthPart::framesPOCXestimated()
{
    return POC_x;
}
QVector<QVector<double>> qThreadFourthPart::framesPOCYestimated()
{
    return POC_y;
}
QVector<QVector<double>> qThreadFourthPart::framesAngleestimated()
{
    return angle;
}

void qThreadFourthPart::onDataObtained(){
    emit readyForFinish();
}
