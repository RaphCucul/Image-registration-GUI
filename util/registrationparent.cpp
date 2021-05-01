#include "util/registrationparent.h"
#include "image_analysis/image_processing.h"
#include "shared_staff/globalsettings.h"

#include <opencv2/opencv.hpp>
#include <QDebug>

RegistrationParent::RegistrationParent(QWidget *parent) : QWidget(parent)
{
    numberOfThreads = GlobalSettings::getSettings()->getUsedCores();
}

void RegistrationParent::onFinishThread(int threadIndex){
    if (threadPool[threadIndex]->isRunning()){
        threadPool[threadIndex]->terminate();
        threadPool[threadIndex]->wait(10);
        threadPool[threadIndex]->deleteLater();
    }
    else
        threadPool[threadIndex]->deleteLater();
    finishedThreadCounter++;
    if (finishedThreadCounter == numberOfThreads) {
        threadPool.clear();
        finishedThreadCounter=0;
    }
}

void RegistrationParent::cancelAllCalculations(){
    if (!threadPool.isEmpty()){
        for (int threadIndex = 1; threadIndex <= threadPool.count(); threadIndex++){
            if (threadPool.contains(threadIndex)) {
                if (threadPool[threadIndex]->isRunning()){
                    threadPool[threadIndex]->terminate();
                    threadPool[threadIndex]->wait(100);
                }
                threadPool[threadIndex]->dataObtained();
                threadPool[threadIndex]->deleteLater();
            }
        }
    }
    canProceed = false;
    /*if (!threadPool.isEmpty()){
        threadPool.clear();
    }*/
    initMaps();
    emit calculationStopped();
}

void RegistrationParent::initMaps(){
    QVector<double> pomD;
    QVector<int> pomI;
    for (int index = 0; index < videoParameters.count(); index++){
        if (index < 6){
            videoParametersDouble[videoParameters.at(index)] = pomD;
        }
        else if (index == 6){
            videoParametersInt[videoParameters.at(index)] = pomI;
        }
        else if (index > 6){
            videoAnomalies[videoParameters.at(index)] = pomI;
        }
    }    
}

VideoWriter::VideoWriter(QString i_videoFullPath,QMap<QString,QVector<double>> i_data,
                         QVector<int> i_evaluationInformation, QString i_writePath, bool onlyBest){
    videoReadPath = i_videoFullPath;
    obtainedData = i_data;
    videoWritePath = i_writePath;
    onlyBestFrames = onlyBest;
    evaluation = i_evaluationInformation;
}

VideoWriter::~VideoWriter(){
}

void VideoWriter::writeVideo(){
    cv::VideoCapture cap = cv::VideoCapture(videoReadPath.toLocal8Bit().constData());
    if (!cap.isOpened()){
        emit errorOccured(6);
    }

    double _width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double _height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Size _frameSize = cv::Size(int(_width),int(_height));
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    cv::VideoWriter writer;
    writer.open(videoWritePath.toLocal8Bit().constData(),static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)),cap.get(CV_CAP_PROP_FPS),
                _frameSize,true);
    if (!writer.isOpened()){
        errorOccured(12);
    }

    qDebug()<<"Writing video to "<<videoWritePath;

    for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
        cv::Mat shiftedOrig;
        cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
        if (cap.read(shiftedOrig)!=1)
        {
            QString errorMessage = QString(tr("Frame %1 could not be loaded from the video for registration. Process "
                                              "interrupted")).arg(indexImage);
            errorOccured(errorMessage);
        }
        else if (obtainedData["POCX"][indexImage] == 999.0 && !onlyBestFrames){
            writer.write(shiftedOrig);
            shiftedOrig.release();
        }
        else if (onlyBestFrames && (obtainedData["POCX"][indexImage] == 999.0 || (evaluation[indexImage] !=0 && evaluation[indexImage] !=2))) {
            qDebug()<<"Frame "<<indexImage<<" skipped because "<<evaluation[indexImage]<<" "<<evaluation[indexImage];
            continue;
        }
        else{
            cv::Point3d finalTranslation(0.0,0.0,0.0);
            cv::Mat _fullyRegistrated = cv::Mat::zeros(cv::Size(shiftedOrig.cols,shiftedOrig.rows), CV_32FC3);
            transformMatTypeTo8C3(shiftedOrig);
            finalTranslation.x = obtainedData["POCX"][indexImage];
            finalTranslation.y = obtainedData["POCY"][indexImage];
            finalTranslation.z = 0.0;
            qDebug()<<"For frame "<<indexImage<<" the translation is: "<<finalTranslation.x<<" "<<finalTranslation.y;
            _fullyRegistrated = frameTranslation(shiftedOrig,finalTranslation,shiftedOrig.rows,shiftedOrig.cols);
            _fullyRegistrated = frameRotation(_fullyRegistrated,obtainedData["angle"][indexImage]);
            writer.write(_fullyRegistrated);
            shiftedOrig.release();
            _fullyRegistrated.release();
       }
    }
    qDebug()<<"Finished emitted";
    writer.release();
    emit finishedSuccessfully();
}
