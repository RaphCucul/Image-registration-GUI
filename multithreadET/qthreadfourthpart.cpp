#include "qthreadfourthpart.h"
#include "util/souborove_operace.h"

#include <QDebug>
#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

qThreadFourthPart::qThreadFourthPart(QStringList &videos,
                                     QVector<QVector<int> > &framesFirstEvaluationDefinitive,
                                     QVector<QVector<int> > &evalCompl,
                                     QVector<QVector<double> > &CCproblematic,
                                     QVector<QVector<double> > &FWHMproblematic,
                                     QVector<QVector<double> > &POCX,
                                     QVector<QVector<double> > &POCY,
                                     QVector<QVector<double> > &Angles,
                                     QVector<QVector<double> > &Fr_X,
                                     QVector<QVector<double> > &Fr_Y,
                                     QVector<QVector<double> > &Fr_Eu,
                                     QVector<double> &averageCC,
                                     QVector<double> &averageFWHM,
                                     QObject* parent):QThread(parent)
{
    videoList = videos;
    framesFirstEvaluationComplete = framesFirstEvaluationDefinitive;
    framesCompleteEvaluation = evalCompl;
    computedCC = CCproblematic;
    computedFWHM = FWHMproblematic;
    POC_x = POCX;
    POC_y = POCY;
    angle = Angles;
    frangi_x = Fr_X;
    frangi_y = Fr_Y;
    frangi_euklid = Fr_Eu;
    averageCCcomplete = averageCC;
    averageFWHMcomplete = averageFWHM;
}

void qThreadFourthPart::run()
{
    emit typeOfMethod(3);
    emit percentageCompleted(0);
    videoCount = double(videoList.count());
    for (int indexVidea = 0; indexVidea < videoList.count(); indexVidea++)
    {
        QVector<int> snimky_k_provereni_druhy;
        frameCount = double(framesFirstEvaluationComplete[indexVidea].length());
        QString fullPath = videoList.at(indexVidea);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(indexVidea);
        for (int b = 0; b < framesFirstEvaluationComplete[indexVidea].length(); b++)
        {
            emit percentageCompleted(qRound((double(indexVidea)/videoCount)*100.0+((double(b)/frameCount)*100.0)/videoCount));
            if ((averageCCcomplete[indexVidea] - computedCC[indexVidea][b]) <= 0.01)
            {
                if (computedFWHM[indexVidea][b] < (averageFWHMcomplete[indexVidea] + 2))
                {
                    qDebug()<< "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " je vhodny ke slicovani.";
                    framesCompleteEvaluation[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 0;
                }
                else
                {
                    qDebug()<< "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " bude proveren.";
                    snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[indexVidea][b]);
                }
                continue;
            }
            else if ((averageCCcomplete[indexVidea] - computedCC[indexVidea][b]) >0.01)
            {
                if (computedFWHM[indexVidea][b] < (averageFWHMcomplete[indexVidea] + 2.5))
                {
                    qDebug()<< "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " je vhodny ke slicovani.";
                    framesCompleteEvaluation[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 0;
                }
                else if (computedFWHM[indexVidea][b] > (averageFWHMcomplete[indexVidea] + 10))
                {
                    qDebug() << "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " nepripusten k analyze.";
                    framesCompleteEvaluation[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 5.0;
                    POC_x[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 999.0;
                    POC_y[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 999.0;
                    angle[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 999.0;
                    frangi_x[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 999.0;
                    frangi_y[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 999.0;
                    frangi_euklid[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 999.0;
                }
                else
                {
                    qDebug()<< "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " bude proveren.";
                    snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[indexVidea][b]);
                }
                continue;
            }
            else if ((averageCCcomplete[indexVidea] - computedCC[indexVidea][b]) >= 0.05)
            {
                if (computedFWHM[indexVidea][b] <= (averageFWHMcomplete[indexVidea] + 3))
                {
                    qDebug()<< "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " je vhodny ke slicovani.";
                    framesCompleteEvaluation[indexVidea][framesFirstEvaluationComplete[indexVidea][b]] = 0;
                }
                else
                {
                    qDebug()<< "Snimek "<< framesFirstEvaluationComplete[indexVidea][b]<< " bude proveren.";
                    snimky_k_provereni_druhy.push_back(framesFirstEvaluationComplete[indexVidea][b]);
                }
                continue;
            }
        }
        framesSecondEvaluationComplete.append(snimky_k_provereni_druhy);
    }
    emit done(4);
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
