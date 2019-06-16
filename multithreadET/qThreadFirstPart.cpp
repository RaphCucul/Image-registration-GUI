#include "multithreadET/qThreadFirstPart.h"
#include "util/vector_operations.h"
#include "util/files_folders_operations.h"
#include "dialogs/multiplevideoet.h"
#include "image_analysis/image_processing.h"
#include "image_analysis/entropy.h"
#include "image_analysis/correlation_coefficient.h"
#include "image_analysis/frangi_utilization.h"
#include "registration/phase_correlation_function.h"
#include "registration/multiPOC_Ai1.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>
#include <QVector>
#include <QThread>

using cv::Mat;
using cv::Point3d;
qThreadFirstPart::qThreadFirstPart(QStringList i_videosForAnalysis,
                                   cv::Point2d i_verticalAnomalyCoords,
                                   cv::Point2d i_horizontalAnomalyCoords,
                                   QVector<double> i_FrangiParametersValues,
                                   QMap<QString, int> i_margins,
                                   QMap<QString, double> i_ratios,
                                   QObject *parent):QThread(parent)
{
    processVideos = i_videosForAnalysis;
    videoCount = i_videosForAnalysis.count();
    verticalAnomaly = i_verticalAnomalyCoords;
    horizontalAnomaly = i_horizontalAnomalyCoords;
    FrangiParameters = i_FrangiParametersValues;
    margins = i_margins;
    ratios = i_ratios;

    if (horizontalAnomaly.y != 0.0 || verticalAnomaly.x != 0.0)
        anomalyPresence = true;

    emit setTerminationEnabled(true);
}
void qThreadFirstPart::run()
{
    /// Firstly, entropy and tennengrad are computed for each frame of each video
    /// This task is indicated by emiting 0 in typeOfMethod
    emit typeOfMethod(0);
    emit percentageCompleted(0);
    percent = 0;
    for (int videoIndex = 0; videoIndex < processVideos.count(); videoIndex++)
    {
        QString fullPath = processVideos.at(videoIndex);
        QString folder,filename,suffix;
        processFilePath(fullPath,folder,filename,suffix);
        emit actualVideo(videoIndex);
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        if (!cap.isOpened()){
            emit unexpectedTermination(videoIndex,"hardError");
            fillEmpty(260);
            continue;
        }

        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> pomVecD(frameCount,0.0);
        QVector<int> pomVecI(frameCount,0);

        cv::Rect anomalyArea(0,0,0,0);
        QVector<double> entropyActual,tennengradActual;
        entropyActual.fill(0.0,frameCount);
        tennengradActual.fill(0.0,frameCount);

        double frameCountD = (cap.get(CV_CAP_PROP_FRAME_COUNT));
        qDebug()<< "Video analysis: "<<videoIndex;
        for (double a = 0; a < frameCountD; a++)
        {
            emit percentageCompleted(qRound((double(videoIndex)/double(videoCount))*100.0+1.0+((double(a)/frameCountD)*100.0)/double(videoCount)));
            cv::Mat frame;
            double entropy_value;
            cv::Scalar tennengrad_value;
            cap.set(CV_CAP_PROP_POS_FRAMES,(a));
            if (!cap.read(frame)){
                QString errorMessage = QString("Frame number %1 could not be opened").arg(a);
                unexpectedTermination(videoIndex,"hardError");
                fillEmpty(frameCount);
                continue;
            }
            else
            {
                transformMatTypeTo8C3(frame);
                calculateParametersET(frame,entropy_value,tennengrad_value);
                double pom = tennengrad_value[0];
                entropyActual[int(a)] = (entropy_value);
                tennengradActual[int(a)] = (pom);
                frame.release();
            }
        }

        entropyComplete.push_back(entropyActual);
        tennengradComplete.push_back(tennengradActual);
        qDebug()<<entropyComplete.size()<<tennengradComplete.size();

        /// Secondly, it is necessary to select proper maximal value for later selection of bad images
        /// after this procedure, windows for detail analysis of entropy and tennengrad vectors are computed
        /// medians of values in the windows are computed
        double correctEntropyMax = checkMaximum(entropyComplete[videoIndex]);
        double correctTennengradMax = checkMaximum(tennengradComplete[videoIndex]);
        QVector<double> windows_tennengrad,windows_entropy,windowsEntropy_medians,windowsTennengrad_medians;
        double restEntropy = 0.0,restTennengrad = 0.0;
        vectorWindows(entropyComplete[videoIndex],windows_entropy,restEntropy);
        vectorWindows(tennengradComplete[videoIndex],windows_tennengrad,restTennengrad);
        windowsEntropy_medians = mediansOfVector(entropyComplete[videoIndex],windows_entropy,restEntropy);
        windowsTennengrad_medians = mediansOfVector(tennengradComplete[videoIndex],windows_tennengrad,restTennengrad);

        /// Thirdly, values of entropy and tennengrad are evaluated and frames get mark good/bad, if they are
        /// or they are not suitable for image registration
        QVector<double> thresholdsEntropy(2,0);
        QVector<double> thresholdsTennengrad(2,0);
        thresholdsEntropy[0] = 0.01;thresholdsEntropy[1] = 0.01;
        thresholdsTennengrad[0] = 10;thresholdsTennengrad[1] = 10;
        double toleranceEntropy = 0.001;
        double toleranceTennengrad = 0.1;
        int dmin = 1;
        QVector<int> badFramesEntropy, badFramesTennengrad;
        QVector<double> nextAnalysisEntropy, nextAnalysisTennengrad;
        bool analysisOfValuesResult = analysisFunctionValues(entropyComplete[videoIndex],windowsEntropy_medians,windows_entropy,
                               correctEntropyMax,thresholdsEntropy,toleranceEntropy,dmin,restEntropy,
                               badFramesEntropy,nextAnalysisEntropy);
        if (!analysisOfValuesResult){
            emit unexpectedTermination(videoIndex,"hardError");
            fillEmpty(frameCount);
            continue;
        }
        analysisOfValuesResult = analysisFunctionValues(tennengradComplete[videoIndex],windowsTennengrad_medians,windows_tennengrad,
                               correctTennengradMax,thresholdsTennengrad,toleranceTennengrad,dmin,restTennengrad,
                               badFramesTennengrad,nextAnalysisTennengrad);
        if (!analysisOfValuesResult){
            emit unexpectedTermination(videoIndex,"hardError");
            fillEmpty(frameCount);
            continue;
        }
        int referencialImageNo = findReferencialNumber(correctEntropyMax,nextAnalysisEntropy,
                                                             entropyComplete[videoIndex]);
        referencialFrames.push_back(referencialImageNo);

        /// Fourth part - frangi filter is applied on the frame marked as the reference
        Mat referencialMat;
        cap.set(CV_CAP_PROP_POS_FRAMES,referencialImageNo);
        cap.read(referencialMat);

        cv::Rect cutoutExtra(0,0,0,0);
        cv::Rect cutoutStandard(0,0,0,0);
        cv::Point3d pt_temp(0.0,0.0,0.0);
        cv::Mat frameForAnalysis;
        cv::Point3d frangi_bod(0,0,0);
        bool scaleChange;
        bool preprocessingSuccessfull = preprocessingCompleteRegistration(
                                        referencialMat,
                                        frameForAnalysis,
                                        FrangiParameters,
                                        verticalAnomaly,
                                        horizontalAnomaly,
                                        anomalyArea,                                        
                                        cutoutExtra,
                                        cutoutStandard,
                                        cap,
                                        scaleChange,
                                        ratios,margins);
        if(!preprocessingSuccessfull){
            emit unexpectedTermination(videoIndex,"hardError");
            fillEmpty(frameCount);
            continue;
        }
        qDebug()<<"Standard "<<cutoutStandard.height<<" "<<cutoutStandard.width;
        qDebug()<<"Extra "<<cutoutExtra.height<<" "<<cutoutExtra.width;
        obtainedCutoffStandard.push_back(cutoutStandard);
        obtainedCutoffExtra.push_back(cutoutExtra);
        QVector<int> badFrame_completeList = mergeVectors(badFramesEntropy,badFramesTennengrad);
        integrityCheck(badFrame_completeList);
        QVector<int> framesEvaluation;
        framesEvaluation = pomVecI;
        if (restEntropy == 1.0)
        {
            badFrame_completeList.push_back(frameCount-1);
        }
        int rest = int(restEntropy-1.0);
        if (restEntropy > 1)
        {
            while (rest >= 0)
            {
                badFrame_completeList.push_back(frameCount-1-rest);
                rest -= 1;
            }
            badFrame_completeList.push_back(frameCount-1);
        }
        for (int a = 0; a < badFrame_completeList.size(); a++)
        {
            framesEvaluation[int(badFrame_completeList[a])] = 1;
        }
        framesFullCompleteDecision.append(framesEvaluation);
        badFramesComplete.append(badFrame_completeList);
        framesFirstFullCompleteEntropyEvaluation.append(badFramesEntropy);
        framesFirstFullCompleteTennengradEvaluation.append(badFramesTennengrad);
    }
    emit percentageCompleted(100);
    emit done(1);
}

void qThreadFirstPart::fillEmpty(int i_frameCount){
    QVector<double> pomVecD(i_frameCount,0.0);
    QVector<int> pomVecI(i_frameCount,0);

    entropyComplete.push_back(pomVecD);
    tennengradComplete.push_back(pomVecD);
    framesFirstFullCompleteEntropyEvaluation.push_back(pomVecI);
    framesFirstFullCompleteTennengradEvaluation.push_back(pomVecI);
    framesFullCompleteDecision.push_back(pomVecI);
    badFramesComplete.push_back(pomVecI);
    referencialFrames.push_back(0);
    obtainedCutoffStandard.push_back(cv::Rect(0,0,0,0));
    obtainedCutoffExtra.push_back(cv::Rect(0,0,0,0));
}
QVector<QVector<double> > qThreadFirstPart::computedEntropy()
{
    return entropyComplete;
}

QVector<QVector<double> > qThreadFirstPart::computedTennengrad()
{
    return tennengradComplete;
}
QVector<int> qThreadFirstPart::estimatedReferencialFrames()
{
    return referencialFrames;
}
QVector<QVector<int> > qThreadFirstPart::computedFirstEntropyEvaluation()
{
    return framesFirstFullCompleteEntropyEvaluation;
}
QVector<QVector<int> > qThreadFirstPart::computedFirstTennengradEvalueation()
{
    return framesFirstFullCompleteTennengradEvaluation;
}
QVector<QVector<int>> qThreadFirstPart::computedBadFrames()
{
    return badFramesComplete;
}
QVector<cv::Rect> qThreadFirstPart::computedCOstandard()
{
    return obtainedCutoffStandard;
}
QVector<QVector<int> > qThreadFirstPart::computedCompleteEvaluation()
{
    return framesFullCompleteDecision;
}
QVector<cv::Rect> qThreadFirstPart::computedCOextra()
{
    return obtainedCutoffExtra;
}

void qThreadFirstPart::onDataObtained(){
    emit readyForFinish();
}
