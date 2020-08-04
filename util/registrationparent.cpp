#include "util/registrationparent.h"
#include "image_analysis/image_processing.h"

#include <opencv2/opencv.hpp>
#include <QDebug>

RegistrationParent::RegistrationParent(QWidget *parent) : QWidget(parent)
{

}

void RegistrationParent::onFinishThread(int threadIndex){
    if (threadPool[threadIndex]->isRunning()){
        threadPool[threadIndex]->terminate();
        threadPool[threadIndex]->wait(10);
        threadPool[threadIndex]->deleteLater();
    }
    else
        threadPool[threadIndex]->deleteLater();
}

void RegistrationParent::cancelAllCalculations(){
    if (!threadPool.isEmpty()){
        for (int threadIndex = 1; threadIndex <= threadPool.count(); threadIndex++){
            if (threadPool[threadIndex]->isRunning()){
                threadPool[threadIndex]->terminate();
                threadPool[threadIndex]->wait(10);
                threadPool[threadIndex]->dataObtained();
            }
        }
    }
    canProceed = false;
    initMaps();
    emit calculationStopped();
}

void RegistrationParent::initMaps(){
    QVector<double> pomD;
    QVector<int> pomI;
    for (int index = 0; index < videoParameters.count(); index++){
        if (index < 8){
            videoParametersDouble[videoParameters.at(index)] = pomD;
        }
        else if (index >= 8 && index < 13){
            videoParametersInt[videoParameters.at(index)] = pomI;
        }
        else if (index >= 13 && index <= 14){
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
    writer.open(videoWritePath.toLocal8Bit().constData(),static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)),cap.get(CV_CAP_PROP_FPS),_frameSize,true);
    if (!writer.isOpened()){
        errorOccured(12);
    }

    for (int indexImage = 0; indexImage < int(frameCount); indexImage++){
        cv::Mat shiftedOrig;
        cap.set(CV_CAP_PROP_POS_FRAMES,indexImage);
        if (cap.read(shiftedOrig)!=1)
        {
            QString errorMessage = QString(tr("Frame %1 could not be loaded from the video for registration. Process interrupted")).arg(indexImage);
            errorOccured(errorMessage);
        }
        else if (obtainedData["POCX"][indexImage] == 999.0 && !onlyBestFrames){
            writer.write(shiftedOrig);
            shiftedOrig.release();
        }
        else if (onlyBestFrames && (obtainedData["POCX"][indexImage] == 999.0 || evaluation[indexImage] !=0 || evaluation[indexImage] !=2)) {
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
            //qDebug()<<"For frame "<<indexImage<<" the translation is: "<<finalTranslation.x<<" "<<finalTranslation.y;
            _fullyRegistrated = frameTranslation(shiftedOrig,finalTranslation,shiftedOrig.rows,shiftedOrig.cols);
            _fullyRegistrated = frameRotation(_fullyRegistrated,obtainedData["angle"][indexImage]);
            writer.write(_fullyRegistrated);
            shiftedOrig.release();
            _fullyRegistrated.release();
       }
    }
    emit finishedSuccessfully();
}
