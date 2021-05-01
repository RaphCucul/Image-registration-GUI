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
#include <QVector>
#include <QThread>
#include <QDebug>

using cv::Mat;
using cv::Point3d;
qThreadFirstPart::qThreadFirstPart(QStringList i_videosForAnalysis,
                                   cutoutType i_cutoutType,
                                   QMap<QString, QVector<double> > ETthresholds,
                                   QMap<QString, bool> ETthresholdsFound,
                                   bool i_previousThresholdsUsageAllowed,
                                   QObject *parent):QThread(parent)
{
    processVideos = i_videosForAnalysis;
    videoCount = i_videosForAnalysis.count();
    selectedCutout = i_cutoutType;
    previousThresholds = ETthresholds;
    usePreviousThresholds = i_previousThresholdsUsageAllowed;
    videosWithThresholdsFound = ETthresholdsFound;

    emit setTerminationEnabled(true);
}
void qThreadFirstPart::run()
{
    // Firstly, entropy and tennengrad are computed for each frame of each video
    // This task is indicated by emiting 0 in typeOfMethod
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
            fillEmpty(filename);
            continue;
        }

        maximum_frangi.insert(filename,cv::Point3d(0.0,0.0,0.0));
        cv::Rect cutoutExtra(0,0,0,0);
        bool cutoutExtraFound = false;
        qDebug()<<"Selected cutout: "<<selectedCutout;
        if (selectedCutout != cutoutType::NO_CUTOUT && SharedVariables::getSharedVariables()->checkVideoInformationPresence(filename)) {
            if (selectedCutout == cutoutType::EXTRA) {
                QRect _extraCutout = SharedVariables::getSharedVariables()->getVideoInformation(filename,"extra").toRect();
                cutoutExtra = transform_QRect_to_CV_RECT(_extraCutout);
                if (cutoutExtra.width > 0 && cutoutExtra.height > 0) {
                    cutoutExtraFound = true;
                    qDebug()<<"Extra discovered and implemented";
                }
            }
        }
        qDebug()<<"Extra cutout: "<<cutoutExtra.height<<" "<<cutoutExtra.width;
        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> pomVecD(frameCount,0.0);
        QVector<int> pomVecI(frameCount,0);

        QVector<double> entropyActual,tennengradActual;
        entropyActual.fill(0.0,frameCount);
        tennengradActual.fill(0.0,frameCount);

        double frameCountD = (cap.get(CV_CAP_PROP_FRAME_COUNT));
        for (double a = 0; a < frameCountD; a++)
        {
            emit percentageCompleted(qRound((double(videoIndex)/double(videoCount))*100.0+1.0+((double(a)/frameCountD)*100.0)/double(videoCount)));
            cv::Mat frame_original,frame;
            double entropy_value;
            cv::Scalar tennengrad_value;
            cap.set(CV_CAP_PROP_POS_FRAMES,(a));
            if (!cap.read(frame_original)){
                QString errorMessage = QString("Frame number %1 could not be opened").arg(a);
                unexpectedTermination(videoIndex,"hardError");
                fillEmpty(filename);
                continue;
            }
            else
            {
                // if extracutout was selected before this procedure, it is necessary to apply it
                // on the frames
                //rowsFullFrame = frame_original.rows;columnsFullFrame = frame_original.cols;
                if (cutoutExtraFound){
                    frame_original(cutoutExtra).copyTo(frame);
                    frame_original.release();
                }
                else{
                    frame_original.copyTo(frame);
                    frame_original.release();
                }
                transformMatTypeTo8C3(frame);
                calculateParametersET(frame,entropy_value,tennengrad_value);
                double pom = tennengrad_value[0];
                entropyActual[int(a)] = (entropy_value);
                tennengradActual[int(a)] = (pom);
                frame.release();
            }
        }
        entropyComplete.insert(filename,entropyActual);
        tennengradComplete.insert(filename,tennengradActual);

        // Secondly, it is necessary to select proper maximum value for the later selection of bad images
        // after this procedure, windows for detail analysis of entropy and tennengrad vectors are computed
        // medians of values in these windows are computed
        double correctEntropyMax = checkMaximum(entropyComplete[filename]);
        double correctTennengradMax = checkMaximum(tennengradComplete[filename]);
        QVector<double> windows_tennengrad,windows_entropy,windowsEntropy_medians,windowsTennengrad_medians;
        double restEntropy = 0.0,restTennengrad = 0.0;
        vectorWindows(entropyComplete[filename],windows_entropy,restEntropy);
        vectorWindows(tennengradComplete[filename],windows_tennengrad,restTennengrad);
        windowsEntropy_medians = mediansOfVector(entropyComplete[filename],windows_entropy,restEntropy);
        windowsTennengrad_medians = mediansOfVector(tennengradComplete[filename],windows_tennengrad,restTennengrad);

        // Thirdly, values of entropy and tennengrad are evaluated and frames are marked good/bad, if they are
        // suitable or not for image registration
        QVector<double> thresholdsEntropy(2,0);
        QVector<double> thresholdsTennengrad(2,0);
        // ############################################################################## //
        // threshold can be found empirically, but it is possible to change them later
        // in the entropy/tennengrad viewer
        if (usePreviousThresholds && videosWithThresholdsFound[filename]) {
            // video was analysed earlier + the usage of previous thresholds is allowed
            thresholdsEntropy[0] = previousThresholds[filename][0];
            thresholdsEntropy[1] = previousThresholds[filename][1]; // lower upper
            thresholdsTennengrad[0] = previousThresholds[filename][2];
            thresholdsTennengrad[1] = previousThresholds[filename][3]; // lower upper
        }
        else {
            thresholdsEntropy[0] = 0.01;thresholdsEntropy[1] = 0.01; // lower upper
            thresholdsTennengrad[0] = 10;thresholdsTennengrad[1] = 10; // lower upper
        }
        double toleranceEntropy = 0.001;
        double toleranceTennengrad = 0.1;
        int dmin = 1;
        QVector<int> badFramesEntropy, badFramesTennengrad;
        QVector<double> nextAnalysisEntropy, nextAnalysisTennengrad;
        bool analysisOfValuesResult = analysisFunctionValues(entropyComplete[filename],windowsEntropy_medians,windows_entropy,
                               correctEntropyMax,thresholdsEntropy,toleranceEntropy,dmin,restEntropy,
                               badFramesEntropy,nextAnalysisEntropy,
                               (usePreviousThresholds && videosWithThresholdsFound[filename]));
        if (!analysisOfValuesResult){
            emit unexpectedTermination(videoIndex,"hardError");
            fillEmpty(filename);
            continue;
        }

        QVector<double> _t;
        if (!videosWithThresholdsFound[filename]){
            _t.push_back(thresholdsEntropy[0]);
            _t.push_back(thresholdsEntropy[1]);
        }

        analysisOfValuesResult = analysisFunctionValues(tennengradComplete[filename],windowsTennengrad_medians,windows_tennengrad,
                               correctTennengradMax,thresholdsTennengrad,toleranceTennengrad,dmin,restTennengrad,
                               badFramesTennengrad,nextAnalysisTennengrad,
                               (usePreviousThresholds && videosWithThresholdsFound[filename]));
        if (!analysisOfValuesResult){
            emit unexpectedTermination(videoIndex,"hardError");
            fillEmpty(filename);
            continue;
        }

        if (!videosWithThresholdsFound[filename]){
            _t.push_back(thresholdsTennengrad[0]);
            _t.push_back(thresholdsTennengrad[1]);
            calculatedETthresholds.insert(filename,_t);
        }

        // only entropy is used to determine referential frame, because tennengrad was not reliable
        int referentialImageNo = findReferentialNumber(correctEntropyMax,nextAnalysisEntropy,
                                                             entropyComplete[filename]);
        qDebug()<<"### Referential frame for "<<filename<<": "<<referentialImageNo;
        referentialFrames.insert(filename,referentialImageNo);

        // Fourth part - frangi filter is applied on the frame marked as the reference
        Mat referential_temp,referentialMat;
        cap.set(CV_CAP_PROP_POS_FRAMES,referentialImageNo);
        cap.read(referential_temp);

        // here the analysis can be speed up, of the frangi filter was applied on the video before
        // this thread was started
        cv::Rect cutoutStandard(0,0,0,0);
        cv::Rect cutoutStandardPreprocessed(0,0,0,0);
        cv::Point3d frangi_bod(0,0,0);
        bool standardCutoutDiscovered = false;
        if (SharedVariables::getSharedVariables()->checkVideoInformationPresence(filename)){
            QPoint _frangiCoordinates = SharedVariables::getSharedVariables()->getVideoInformation(filename,"frangi").toPoint();
            frangi_bod = transform_QPoint_to_CV_Point3d(_frangiCoordinates);

            if (cutoutExtraFound){
                QRect _standardCutout = SharedVariables::getSharedVariables()->getVideoInformation(filename,"standard").toRect();
                qDebug()<<"Discovered standard cutout because extra"<<_standardCutout.height()<<" "<<_standardCutout.width()<<" "<<_standardCutout.x()<<" "<<_standardCutout.y();
                cutoutStandard = transform_QRect_to_CV_RECT(_standardCutout);
                referential_temp(cutoutExtra).copyTo(referentialMat);
                referential_temp.release();
            }
            else{
                QRect _standardCutout = SharedVariables::getSharedVariables()->getVideoInformation(filename,"standard").toRect();
                qDebug()<<"Discovered standard cutout"<<_standardCutout.height()<<" "<<_standardCutout.width()<<" "<<_standardCutout.x()<<" "<<_standardCutout.y();
                cutoutStandard = transform_QRect_to_CV_RECT(_standardCutout);
                referential_temp.copyTo(referentialMat);
                referential_temp.release();
            }            
        }

        if (referentialMat.rows==0 || referentialMat.cols==0) {
            referential_temp.copyTo(referentialMat);
            referential_temp.release();
        }
        if ((frangi_bod.x <=0 || frangi_bod.y <=0) || (cutoutStandard.width == 0 || cutoutStandard.height == 0)) {
            // the referential frame was not preprocessed -> no anomaly is present in the frame
            // video was not analysed in the past -> no data about referential frame and frangi coordinates
            // =>
            // extraCutout is not expected, only standard cutout must be calculated from the newly calculated
            // frangi coordinates -> preprocessing before complete registration
            qDebug()<<"Preprocessing needed";
            if (!preprocessingCompleteRegistration(referentialMat,
                                                   frangi_bod,
                                                   cutoutStandardPreprocessed,
                                                   SharedVariables::getSharedVariables()->getFrangiParameterWrapper(frangiType::VIDEO_SPECIFIC,filename),
                                                   SharedVariables::getSharedVariables()->getFrangiRatiosWrapper(frangiType::VIDEO_SPECIFIC,filename),
                                                   SharedVariables::getSharedVariables()->getFrangiMarginsWrapper(frangiType::VIDEO_SPECIFIC,filename)
                                                   )){
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(filename);
                continue;
            }
        }
        else
            standardCutoutDiscovered = true;
        if (cutoutExtraFound) {
            // if extra cutout is present, standard cutout size must be saved because each thread, and especially fifth thread,
            // calculates adjusted standard cutout from the standard cutout
            obtainedCutoffExtra.insert(filename,cutoutExtra);
        }
        qDebug()<<"### Referential frame "<<referentialImageNo<<frangi_bod.x<<" "<<frangi_bod.y;
        maximum_frangi[filename] = frangi_bod;
        if (standardCutoutDiscovered) {
            qDebug()<<"Final standard cutout "<<cutoutStandard.height<<" "<<cutoutStandard.width<<" "<<cutoutStandard.x<<" "<<cutoutStandard.y;
            obtainedCutoffStandard.insert(filename,cutoutStandard);
        }
        else {
            qDebug()<<"Final standard cutout "<<cutoutStandardPreprocessed.height<<" "<<cutoutStandardPreprocessed.width<<" "<<cutoutStandardPreprocessed.x<<" "<<cutoutStandardPreprocessed.y;
            obtainedCutoffStandard.insert(filename,cutoutStandardPreprocessed);
        }

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
        framesFullCompleteDecision.insert(filename,framesEvaluation);
        badFramesComplete.insert(filename, badFrame_completeList);
        framesFirstFullCompleteEntropyEvaluation.insert(filename,badFramesEntropy);
        framesFirstFullCompleteTennengradEvaluation.insert(filename,badFramesTennengrad);
    }
    emit percentageCompleted(100);
    emit done(1);
}

void qThreadFirstPart::fillEmpty(QString i_videoName){
    /*QVector<double> pomVecD(i_frameCount,0.0);
    QVector<int> pomVecI(i_frameCount,0);

    badFramesComplete.insert(i_videoName, pomVecI);
    referentialFrames.insert(i_videoName,0);
    obtainedCutoffStandard.insert(i_videoName, cv::Rect(0,0,0,0));
    obtainedCutoffExtra.insert(i_videoName, cv::Rect(0,0,0,0));*/
    doNotProcessThis.push_back(i_videoName);
}
QMap<QString,QVector<double> > qThreadFirstPart::computedEntropy()
{
    return entropyComplete;
}

QMap<QString,QVector<double> > qThreadFirstPart::computedTennengrad()
{
    return tennengradComplete;
}
QMap<QString, int> qThreadFirstPart::estimatedreferentialFrames()
{
    return referentialFrames;
}
QMap<QString,QVector<int> > qThreadFirstPart::computedFirstEntropyEvaluation()
{
    return framesFirstFullCompleteEntropyEvaluation;
}
QMap<QString,QVector<int> > qThreadFirstPart::computedFirstTennengradEvalueation()
{
    return framesFirstFullCompleteTennengradEvaluation;
}
QMap<QString,QVector<int>> qThreadFirstPart::computedBadFrames()
{
    return badFramesComplete;
}
QMap<QString,cv::Rect> qThreadFirstPart::computedCOstandard()
{
    return obtainedCutoffStandard;
}
QMap<QString,QVector<int> > qThreadFirstPart::computedCompleteEvaluation()
{
    return framesFullCompleteDecision;
}
QMap<QString,cv::Rect> qThreadFirstPart::computedCOextra()
{
    return obtainedCutoffExtra;
}
QMap<QString, QVector<double> > qThreadFirstPart::computedThresholds()
{
    return calculatedETthresholds;
}
QVector<QString> qThreadFirstPart::unprocessableVideos(){
    return doNotProcessThis;
}

void qThreadFirstPart::onDataObtained(){
    emit readyForFinish();
}
