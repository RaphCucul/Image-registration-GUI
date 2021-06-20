#include "registration/registrationthread.h"
#include "registration/multiPOC_Ai1.h"
#include "image_analysis/image_processing.h"
#include "image_analysis/correlation_coefficient.h"
#include "image_analysis/frangi_utilization.h"
#include "registration/phase_correlation_function.h"
#include "registration/registration_correction.h"

#include <opencv2/opencv.hpp>

#include <QDebug>
#include <QTableWidgetItem>
#include "image_analysis/frangi_utilization.h"
#include <exception>
using cv::Mat;
using cv::VideoCapture;
using cv::Point3d;
using cv::Rect;

RegistrationThread::RegistrationThread(int& i_indexOfThread,
                                       QString &i_fullVideoPath,
                                       QString& i_nameOfVideo,
                                       QMap<QString, double> i_frangiParameters,
                                       QVector<int>& i_frameEvaluation,
                                       cv::Mat& i_referencialFrame,
                                       int& startFrame,
                                       int& stopFrame,
                                       int& i_iteration,
                                       double& i_areaMaximum,
                                       double& i_angle,
                                       cv::Rect &i_cutoutExtra,
                                       cv::Rect &i_cutoutStandard,
                                       bool i_scaleChange,
                                       QMap<QString,int> i_margins,
                                       QMap<QString,double> i_ratios,
                                       QObject *parent) : QThread(parent),
    referencialImage(i_referencialFrame),
    frangiParameters(i_frangiParameters),
    framesEvaluation(i_frameEvaluation),
    iteration(i_iteration),
    startingFrame(startFrame),
    stoppingFrame(stopFrame),
    maximalArea(i_areaMaximum),
    angle(i_angle),
    standardCutout(i_cutoutStandard),
    extraCutout(i_cutoutExtra),
    scaling(i_scaleChange),
    threadIndex(i_indexOfThread),
    videoName(i_nameOfVideo),
    videoPath(i_fullVideoPath),
    margins(i_margins),
    ratios(i_ratios)
{
    emit setTerminationEnabled(true);
    capture = cv::VideoCapture(videoPath.toLocal8Bit().constData());
    QVector<double> pomD(static_cast<int>(capture.get(CV_CAP_PROP_FRAME_COUNT)),0.0);
    frangiX = pomD;frangiY = pomD;frangiEuklidean = pomD;finalPOCx = pomD;finalPOCy = pomD;maximalAngles = pomD;
    setTerminationEnabled(true);
    qDebug()<<"registration thread initialized";
}

/**
 * @brief Returns calculated results of frangi and POC.
 */
QMap<QString,QVector<double>> RegistrationThread::provideResults()
{
    vectors["FrangiX"] = frangiX;
    vectors["FrangiY"] = frangiY;
    vectors["FrangiEuklid"] = frangiEuklidean;
    vectors["POCX"] = finalPOCx;
    vectors["POCY"] = finalPOCy;
    vectors["angle"] = maximalAngles;
    return vectors;
}

/**
 * @brief Returns information about the range of frames in the current thread.
 */
QVector<int> RegistrationThread::threadFrameRange(){
    QVector<int> output;
    output.push_back(startingFrame);
    output.push_back(stoppingFrame);
    return output;
}

void RegistrationThread::run()
{
    /// define helpers for frame preprocessing
    bool scaling=false;
    if (extraCutout.width>0 && extraCutout.height>0) {
        scaling = true;
    }

    Point3d pt_temp(0.0,0.0,0.0);
    Mat preprocessed,preprocessed_cutout;
    if (scaling) {
        qDebug()<<"Extra cutout applied: "<<extraCutout.height<<" "<<extraCutout.width;
        referencialImage(extraCutout).copyTo(preprocessed);
    }
    else {
        referencialImage.copyTo(preprocessed);
    }
    qDebug()<<"Standard cutout applied: "<<standardCutout.height<<" "<<standardCutout.width<<" "<<standardCutout.x<<" "<<standardCutout.y;
    preprocessed(standardCutout).copyTo(preprocessed_cutout);

    int mode = frangiParameters.contains("mode") ? frangiParameters["mode"] : 1;
    qDebug()<<"Applying frangi mode "<<mode;
    Point3d frangiMaxReversal = frangi_analysis(preprocessed,mode,2,0,"",1,pt_temp,frangiParameters,margins);
    if (frangiMaxReversal.z == 0.0){
        emit errorDetected(threadIndex,QString(tr("Frangi filter for referencial image failed for %1.")).arg(videoName));
        return;
    }
    qDebug()<<"Beginning registration phase";
    for (int indexFrame = startingFrame; indexFrame <= stoppingFrame; indexFrame++)
    {
        bool errorOccured = false;
        if (framesEvaluation[indexFrame] == 0){
            Mat shifted_temp;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(shifted_temp)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                errorOccured = true;
                continue;
            }
            if (!registrateTheBest(shifted_temp,
                                   preprocessed,
                                   frangiMaxReversal,
                                   indexFrame,
                                   scaling)){
                errorOccured = true;
                continue;
            }
        }
        else if (framesEvaluation[indexFrame] == 2)
        {
            Mat shifted_temp;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(shifted_temp)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                errorOccured = true;
                continue;
            }
            else
            {
                if (scaling == true)
                {
                    Mat shifted;
                    shifted_temp(extraCutout).copyTo(shifted);
                    Point3d frangiCoords_preprocessed_reverse = frangi_analysis(shifted,mode,2,0,"",1,pt_temp,
                                                                                frangiParameters,margins);
                    if (frangiCoords_preprocessed_reverse.z == 0.0){
                        errorOccured = true;
                        continue;
                    }
                    frangiX[indexFrame] = frangiCoords_preprocessed_reverse.x;
                    frangiY[indexFrame] = frangiCoords_preprocessed_reverse.y;
                    frangiEuklidean[indexFrame] = 0.0;
                    //qDebug() << "Referencial frame "<<indexFrame<<" analysed.";
                    shifted_temp.release();
                    shifted.release();
                }
                else
                {
                    Point3d frangiCoords_preprocessed_reverse = frangi_analysis(shifted_temp,mode,2,0,"",1,pt_temp,
                                                                                frangiParameters,margins);
                    if (frangiCoords_preprocessed_reverse.z == 0.0){
                        errorOccured = true;
                        continue;
                    }
                    //qDebug() << "Referencial frame "<<indexFrame<<" analysed.";
                    shifted_temp.release();
                    frangiX[indexFrame] = frangiCoords_preprocessed_reverse.x;
                    frangiY[indexFrame] = frangiCoords_preprocessed_reverse.y;
                    frangiEuklidean[indexFrame] = 0.0;
                }
                maximalAngles[indexFrame] = 0.0;
                finalPOCx[indexFrame] = 0.0;
                finalPOCy[indexFrame] = 0.0;
            }
        }
        else if (framesEvaluation[indexFrame] == 5)
        {
            Mat shifted;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(shifted)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                errorOccured = true;
                continue;
            }
            else
            {
                //qDebug() << "Frame "<<indexFrame<<" written without any changes.";
                shifted.release();
                frangiX[indexFrame] = 999.0;
                frangiY[indexFrame] = 999.0;
                frangiEuklidean[indexFrame] = 999.0;
                maximalAngles[indexFrame] = 999;
                finalPOCx[indexFrame] = 999.0;
                finalPOCy[indexFrame] = 999.0;
            }
        }
        else if (framesEvaluation[indexFrame] == 1 || framesEvaluation[indexFrame] == 4)
        {
            Mat registratedFrame;
            Mat shifted;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(shifted)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                errorOccured = true;
                continue;
            }
            if (!fullRegistration(shifted,
                                  preprocessed,
                                  indexFrame,
                                  scaling,
                                  registratedFrame)){
                errorOccured = true;
                continue;
            }
            Mat shifted_temp;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(shifted_temp)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                continue;
            }
            else
            {
                Point3d translationCorrection(0.0,0.0,0.0);
                cv::Mat fully_registratedFrame;
                if (!registrationCorrection(registratedFrame,preprocessed,fully_registratedFrame,
                                            translationCorrection)){
                    errorOccured = true;
                    continue;
                }

                if (translationCorrection.x != 0.0 || translationCorrection.y != 0.0)
                {
                    finalPOCx[indexFrame] += translationCorrection.x;
                    finalPOCy[indexFrame] += translationCorrection.y;
                    Point3d pt6 = pc_translation_hann(preprocessed,fully_registratedFrame,maximalArea);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = pc_translation(preprocessed,fully_registratedFrame,maximalArea);
                    }
                    //qDebug() << "After correction of translation Y: " << pt6.y <<" X: "<<pt6.x;
                }
                Point3d _tempTranslation = Point3d(finalPOCx[indexFrame],finalPOCy[indexFrame],0.0);
                //qDebug()<<" Translation: "<<finalPOCx[indexFrame]<<" "<<finalPOCy[indexFrame]<<" "<<maximalAngles[indexFrame];
                if (scaling == true)
                {
                    int rows = shifted_temp.rows;
                    int cols = shifted_temp.cols;
                    Mat shifted_original = frameTranslation(shifted_temp,_tempTranslation,rows,cols);
                    Mat finalFrame = frameRotation(shifted_original,maximalAngles[indexFrame]);
                    fully_registratedFrame.release();
                    finalFrame.release();
                    shifted_original.release();
                    shifted_temp.release();
                }
                else
                {
                    fully_registratedFrame.release();
                    shifted_temp.release();
                }
            }
        }
        else {
            emit x_coordInfo(indexFrame,0,QString::number(999.0));
            emit y_coordInfo(indexFrame,1,QString::number(999.0));
            emit angleInfo(indexFrame,2,QString::number(999.0));
            emit statusInfo(indexFrame,3,QString("error"));
        }
        if (errorOccured){
            emit x_coordInfo(indexFrame,0,QString::number(999.0));
            emit y_coordInfo(indexFrame,1,QString::number(999.0));
            emit angleInfo(indexFrame,2,QString::number(999.0));
            emit statusInfo(indexFrame,3,QString("error"));
        }
        else if (!errorOccured){
            /*qDebug()<<"Sending: frame: "<<indexFrame<<" "<<QString::number(finalPOCx[indexFrame])<<" "<<
                      QString::number(finalPOCy[indexFrame])<<" "<<QString::number(maximalAngles[indexFrame]);*/

            emit x_coordInfo(indexFrame,0,QString::number(finalPOCx[indexFrame]));
            emit y_coordInfo(indexFrame,1,QString::number(finalPOCy[indexFrame]));
            emit angleInfo(indexFrame,2,QString::number(maximalAngles[indexFrame]));
            emit statusInfo(indexFrame,3,QString("done"));
        }
        //qDebug()<<"####################################################";
    }
    /*if (framesEvaluation[startingFrame] == 0 || framesEvaluation[startingFrame] == 1 ||
        framesEvaluation[startingFrame] == 4){
        qDebug()<<"Recalculating starting frame "<<startingFrame;
        Mat shifted_temp;
        capture.set(CV_CAP_PROP_POS_FRAMES,startingFrame);
        if (capture.read(shifted_temp)!=1)
        {
            qWarning()<<"Frame "<<startingFrame<<" could not be read!";
        }
        else {
            if (registrateTheBest(shifted_temp,preprocessed,frangiMaxReversal,startingFrame,iteration,maximalArea,
                                  totalAngle,extraCutout,
                                  standardCutout,scaling,frangiParameters,
                                  finalPOCx,finalPOCy,frangiX,frangiY,frangiEuklidean,maximalAngles,margins)){
                emit x_coordInfo(startingFrame,0,QString::number(finalPOCx[startingFrame]));
                emit y_coordInfo(startingFrame,1,QString::number(finalPOCy[startingFrame]));
                emit angleInfo(startingFrame,2,QString::number(maximalAngles[startingFrame]));
                emit statusInfo(startingFrame,3,QString("done"));
                qDebug()<<QString::number(frangiX[startingFrame])<<QString::number(frangiY[startingFrame]);
            }
        }
    }*/
    emit allWorkDone(threadIndex);
}

void RegistrationThread::dataObtained(){
    emit readyForFinish(threadIndex);
}

bool RegistrationThread::registrateTheBest(cv::Mat i_shifted,
                                           cv::Mat& i_referentialFrame,
                                           cv::Point3d i_coordsFrangiStandardReferencialReverse,
                                           int i_index_translated,
                                           bool i_scaleChanged)
{
    Mat fully_registratedFrame = cv::Mat::zeros(cv::Size(i_referentialFrame.cols,i_referentialFrame.rows), CV_32FC3);
    qDebug()<<"Registrating the best frames";
    if (i_index_translated == 0) qDebug()<<finalPOCx.length()<<" "<<finalPOCy.length();
    if (!fullRegistration(i_shifted,
                          i_referentialFrame,
                          i_index_translated,
                          i_scaleChanged,
                          fully_registratedFrame)){
        qWarning()<< "Registration error. Frame "<<i_index_translated<<" was not registrated.";
        frangiX[i_index_translated]=999.0;
        frangiY[i_index_translated]=999.0;
        frangiEuklidean[i_index_translated]=999.0;
        fully_registratedFrame.release();
        return false;
    }
    else
    {
        qDebug()<<"Full registration correct.";
        Mat refFrame_cutout;
        i_referentialFrame(standardCutout).copyTo(refFrame_cutout);
        int rows = i_referentialFrame.rows;
        int cols = i_referentialFrame.cols;
        if (std::abs(finalPOCx[i_index_translated]) == 999.0)
        {
            qWarning()<< "Frame "<<i_index_translated<<" written without changes.";
            fully_registratedFrame.release();
            frangiX[i_index_translated]=999.0;
            frangiY[i_index_translated]=999.0;
            frangiEuklidean[i_index_translated]=999.0;
            return false;
        }
        else
        {
            /*Mat shifted_temp;
            i_cap.set(CV_CAP_PROP_POS_FRAMES,i_index_translated);
            if (i_cap.read(shifted_temp)!=1)
            {
                qWarning()<<"Frame "<<i_index_translated<<" could not be read!";
            }*/
            Mat interresult32f,interresult32f_cutout;//,shifted;
            /*if (i_scaleChanged == true)
            {
                shifted_temp(i_cutoutExtra).copyTo(shifted);
            }
            else
            {
                shifted_temp.copyTo(shifted);
            }*/

            Point3d translationCorrection(0.0,0.0,0.0);
            Mat registratedCorrectedFrame = cv::Mat::zeros(cv::Size(i_referentialFrame.cols,i_referentialFrame.rows), CV_32FC3);
            if (!registrationCorrection(fully_registratedFrame,i_referentialFrame,registratedCorrectedFrame,
                                        translationCorrection)){
                qWarning()<<"Frame "<<i_index_translated<<" - registration correction failed";
                return false;
            }
            else{
                if (translationCorrection.x > 0.0 || translationCorrection.y > 0.0)
                {
                    qDebug()<<"Correction: "<<translationCorrection.x<<" "<<translationCorrection.y;
                    finalPOCx[i_index_translated] += translationCorrection.x;
                    finalPOCy[i_index_translated] += translationCorrection.y;
                    Point3d pt6 = pc_translation_hann(i_referentialFrame,registratedCorrectedFrame,maximalArea);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = pc_translation(i_referentialFrame,registratedCorrectedFrame,maximalArea);
                    }
                    qDebug()<<"Checking translation after correction: "<<pt6.x<<" "<<pt6.y;
                    finalPOCx[i_index_translated] += pt6.x;
                    finalPOCy[i_index_translated] += pt6.y;
                }

                registratedCorrectedFrame.copyTo(interresult32f);
                transformMatTypeTo32C1(interresult32f);
                interresult32f(standardCutout).copyTo(interresult32f_cutout);
                double R1 = calculateCorrCoef(i_referentialFrame,registratedCorrectedFrame,standardCutout);
                Point3d _tempTranslation = Point3d(finalPOCx[i_index_translated],finalPOCy[i_index_translated],0.0);

                int mode = frangiParameters.contains("mode") ? frangiParameters["mode"] : 1;
                qDebug()<<"Applying frangi mode "<<mode;
                Point3d frangiCoords_registrated_reverse = frangi_analysis(registratedCorrectedFrame,mode,2,0,"",2,
                                                                           _tempTranslation,frangiParameters,margins);

                frangiX[i_index_translated] = frangiCoords_registrated_reverse.x;
                frangiY[i_index_translated] = frangiCoords_registrated_reverse.y;
                double yydef = i_coordsFrangiStandardReferencialReverse.x - frangiCoords_registrated_reverse.x;
                double xxdef = i_coordsFrangiStandardReferencialReverse.y - frangiCoords_registrated_reverse.y;
                double diffSum = std::pow(xxdef,2.0) + std::pow(yydef,2.0);
                double euklid = std::sqrt(diffSum);
                frangiEuklidean[i_index_translated] = euklid;

                Point3d finalTranslation(0.0,0.0,0.0);
                finalTranslation.y = finalPOCy[i_index_translated] - yydef;
                finalTranslation.x = finalPOCx[i_index_translated] - xxdef;
                qDebug()<< "Frame "<<i_index_translated<<finalPOCy[i_index_translated]<<" "<<yydef<<" "<<finalPOCx[i_index_translated]<<" "<<xxdef;
                finalTranslation.z = 0;
                xxdef = 0.0;
                yydef = 0.0;
                diffSum = 0.0;
                euklid = 0.0;

                Mat shifted2 = frameTranslation(i_shifted,finalTranslation,rows,cols);
                Mat finalFrame = frameRotation(shifted2,maximalAngles[i_index_translated]);
                shifted2.release();
                double R2 = calculateCorrCoef(i_referentialFrame,finalFrame,standardCutout);

                if (R1 >= R2)
                {
                    qDebug()<< "Frame "<<i_index_translated<<" written after standard registration.";
                    //qDebug()<<"R1: "<<R1<<" R2: "<<R2;
                    qDebug()<<"Translation: "<<finalPOCx[i_index_translated]<<" "<<finalPOCy[i_index_translated]<<" "<<maximalAngles[i_index_translated];
                    if (i_scaleChanged == true)
                    {
                        int rows = i_shifted.rows;
                        int cols = i_shifted.cols;
                        Mat shifted_original = frameTranslation(i_shifted,_tempTranslation,rows,cols);
                        Mat finalFrame2 = frameRotation(shifted_original,maximalAngles[i_index_translated]);
                        fully_registratedFrame.release();
                        registratedCorrectedFrame.release();
                        finalFrame2.release();
                        finalFrame.release();
                        shifted_original.release();
                        //shifted_temp.release();
                    }
                    else
                    {
                        fully_registratedFrame.release();
                        finalFrame.release();
                    }
                }
                else
                {
                    qDebug()<< "Frame "<<i_index_translated<<" written after vein analysis";
                    qDebug()<<" Translation: "<<finalTranslation.x<<" "<<finalTranslation.y<<" "<<maximalAngles[i_index_translated];
                    if (i_scaleChanged == true)
                    {
                        int rows = i_shifted.rows;
                        int cols = i_shifted.cols;
                        Mat shifted_original = frameTranslation(i_shifted,finalTranslation,rows,cols);
                        Mat finalFrame2 = frameRotation(shifted_original,maximalAngles[i_index_translated]);
                        fully_registratedFrame.release();
                        registratedCorrectedFrame.release();
                        finalFrame2.release();
                        finalFrame.release();
                        shifted_original.release();
                        //shifted_temp.release();
                    }
                    else
                    {
                        fully_registratedFrame.release();
                        finalFrame.release();
                    }
                    finalPOCx[i_index_translated] = finalTranslation.x;
                    finalPOCy[i_index_translated] = finalTranslation.y;
                }
                interresult32f.release();
                interresult32f_cutout.release();
                return true;
            }
        }
    }
};

bool RegistrationThread::fullRegistration(cv::Mat shifted_temp,
                                          cv::Mat& i_referencialFrame,
                                          int i_shiftedNo,
                                          bool i_scaleChange,
                                          cv::Mat& i_fullyRegistrated)
{
    try {        
        double totalAngle = 0.0;
        transformMatTypeTo8C3(i_referencialFrame);
        transformMatTypeTo8C3(shifted_temp);
        int rows = i_referencialFrame.rows;
        int cols = i_referencialFrame.cols;
        Mat hann;
        createHanningWindow(hann, i_referencialFrame.size(), CV_32FC1);
        Mat referencialFrame_32f,referencialFrame_cutout;
        /*std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);
        cv::imwrite("referential.png",i_referencialFrame,compression_params);
        cv::imwrite("shifted.png",shifted_temp);*/
        i_referencialFrame.copyTo(referencialFrame_32f);
        transformMatTypeTo32C1(referencialFrame_32f);
        referencialFrame_32f(standardCutout).copyTo(referencialFrame_cutout);
        Mat shifted, shifted_cutout;
        if (i_scaleChange == true)
        {
            shifted_temp(extraCutout).copyTo(shifted);
            shifted(standardCutout).copyTo(shifted_cutout);
            shifted_temp.release();
        }
        else
        {
            shifted_temp.copyTo(shifted);
            shifted(standardCutout).copyTo(shifted_cutout);
            shifted_temp.release();
        }
        Mat shifted_32f;
        shifted.copyTo(shifted_32f);
        transformMatTypeTo32C1(shifted_32f);

        Mat registrated1;
        Point3d pt1(0.0,0.0,0.0);
        if (i_shiftedNo == 0) qDebug()<<referencialFrame_32f.rows<<" "<<shifted_32f.rows;
        if (i_scaleChange == true)
        {
            pt1 = pc_translation_hann(referencialFrame_32f,shifted_32f,maximalArea);
            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = pc_translation(referencialFrame_32f,shifted_32f,maximalArea);

            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = pc_translation(referencialFrame_cutout,shifted_cutout,maximalArea);
        }
        if (i_scaleChange == false)
        {
            pt1 = pc_translation_hann(referencialFrame_32f,shifted_32f,maximalArea);
        }

        if (pt1.x>=55 || pt1.y>=55)
        {
            finalPOCx[i_shiftedNo] = 999.0;
            finalPOCy[i_shiftedNo] = 999.0;
            maximalAngles[i_shiftedNo] = 999.0;
            return false;
        }
        else
        {
            //qDebug()<<"Filling pt1";
            finalPOCx[i_shiftedNo] = pt1.x;
            finalPOCy[i_shiftedNo] = pt1.y;
            //qDebug()<<"pt1 filled.";
            if (i_shiftedNo == 0)
                qDebug()<<"PT1: "<<pt1.x<<" "<<pt1.y;
            registrated1 = frameTranslation(shifted,pt1,rows,cols);
            cv::Mat registrated1_32f_rotace,registrated1_32f,registrated1_cutout;
            registrated1.copyTo(registrated1_32f);
            transformMatTypeTo32C1(registrated1_32f);
            Point3d rotation_result = pc_rotation(referencialFrame_32f,registrated1_32f,angle,pt1.z,pt1);
            if (std::abs(rotation_result.y) > angle)
                rotation_result.y=0;

            maximalAngles[i_shiftedNo] = rotation_result.y;
            registrated1_32f_rotace = frameRotation(registrated1_32f,rotation_result.y);
            registrated1_32f_rotace(standardCutout).copyTo(registrated1_cutout);

            Point3d pt2(0.0,0.0,0.0);
            if (i_shiftedNo == 0) qDebug()<<referencialFrame_cutout.rows<<" "<<registrated1_cutout.rows;
            pt2 = pc_translation(referencialFrame_cutout,registrated1_cutout,maximalArea);
            if (pt2.x >= 55 || pt2.y >= 55)
            {
                finalPOCx[i_shiftedNo] = 999.0;
                finalPOCy[i_shiftedNo] = 999.0;
                maximalAngles[i_shiftedNo] = 999.0;
                registrated1.copyTo(i_fullyRegistrated);
                registrated1.release();
                return false;
            }
            else
            {
                double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt2.z);
                double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                //qDebug()<<"FWHM for "<<cislo_shifted<<" = "<<FWHM;
                registrated1.release();
                registrated1_32f.release();
                registrated1_cutout.release();
                if (i_shiftedNo == 0)
                    qDebug()<<"PT2: "<<pt2.x<<" "<<pt2.y;

                Point3d pt3(0.0,0.0,0.0);
                pt3.x = pt1.x+pt2.x;
                pt3.y = pt1.y+pt2.y;
                pt3.z = pt2.z;
                finalPOCx[i_shiftedNo] = pt3.x;
                finalPOCy[i_shiftedNo] = pt3.y;
                Mat registrated2 = frameTranslation(shifted,pt3,rows,cols);
                Mat registrated2_32f,registrated2_cutout;
                registrated2.copyTo(registrated2_32f);
                transformMatTypeTo32C1(registrated2_32f);
                Mat registrated2_rotation = frameRotation(registrated2_32f,rotation_result.y);
                registrated2_rotation(standardCutout).copyTo(registrated2_cutout);
                Mat interresult_cutout,interresult;
                registrated2_rotation.copyTo(interresult);
                registrated2_cutout.copyTo(interresult_cutout);
                //registrated2.release();
                registrated2_cutout.release();
                registrated2_32f.release();
                registrated2_rotation.release();
                totalAngle+=rotation_result.y;
                rotation_result.y = 0;
                int maxIterationCount = 0;
                if (iteration == -1)
                {
                    if (FWHM <= 20){maxIterationCount = 2;}
                    else if (FWHM > 20 && FWHM <= 30){maxIterationCount = 4;}
                    else if (FWHM > 30 && FWHM <= 35){maxIterationCount = 6;}
                    else if (FWHM > 35 && FWHM <= 40){maxIterationCount = 8;}
                    else if (FWHM > 40 && FWHM <= 45){maxIterationCount = 10;}
                    else if (FWHM > 45){maxIterationCount = 5;}
                }
                if (iteration >= 1)
                {
                    maxIterationCount = iteration;
                }
                for (int i = 0; i < maxIterationCount; i++)
                {
                    Point3d rotation_ForLoop(0.0,0.0,0.0);
                    rotation_ForLoop = pc_rotation(i_referencialFrame,interresult,angle,pt3.z,pt3);
                    if (std::abs(rotation_ForLoop.y) > angle)
                        rotation_ForLoop.y = 0.0;
                    else if (std::abs(totalAngle+rotation_ForLoop.y) > angle)
                        rotation_ForLoop.y=0.0;
                    else
                        totalAngle+=rotation_ForLoop.y;

                    Mat rotatedFrame;
                    if (rotation_ForLoop.y != 0.0)
                        rotatedFrame = frameRotation(interresult,rotation_ForLoop.y);
                    else
                        rotatedFrame = interresult;

                    rotation_ForLoop.y = 0.0;
                    Mat rotatedFrame_cutout;
                    rotatedFrame(standardCutout).copyTo(rotatedFrame_cutout);
                    rotatedFrame.release();
                    Point3d pt4(0.0,0.0,0.0);
                    pt4 = pc_translation(referencialFrame_cutout,rotatedFrame_cutout,maximalArea);
                    rotatedFrame_cutout.release();
                    if (pt4.x >= 55 || pt4.y >= 55)
                    {
                        registrated2.copyTo(i_fullyRegistrated);
                        //qDebug()<<"Frame "<<cislo_shifted<<" terminated because \"condition 55\" reached.";
                        break;
                    }
                    else
                    {
                        pt3.x += pt4.x;
                        pt3.y += pt4.y;
                        pt3.z = pt4.z;                        
                        finalPOCx[i_shiftedNo] = pt3.x;
                        finalPOCy[i_shiftedNo] = pt3.y;
                        maximalAngles[i_shiftedNo] = totalAngle;
                        Mat shifted_temp = frameTranslation(shifted,pt3,rows,cols);
                        Mat rotatedFrame_temp = frameRotation(shifted_temp,totalAngle);
                        shifted_temp.release();
                        rotatedFrame_temp.copyTo(interresult);
                        transformMatTypeTo32C1(rotatedFrame_temp);
                        rotatedFrame_temp.release();
                    }
                }
                if (i_shiftedNo == 0)
                    qDebug()<<"Final after loop: "<<finalPOCx[i_shiftedNo]<<" "<<finalPOCy[i_shiftedNo];
                interresult.copyTo(i_fullyRegistrated);
                return true;
            }
        }
    } catch (std::exception &e) {
        qWarning()<<"Full registration error: "<<e.what();
        finalPOCx[i_shiftedNo] = 999.0;
        finalPOCy[i_shiftedNo] = 999.0;
        maximalAngles[i_shiftedNo] = 999.0;
        return false;
    }
};

bool RegistrationThread::registrationCorrection(cv::Mat& i_registratedFrame,
                                                cv::Mat& i_frame,
                                                cv::Mat &i_corrected,
                                                cv::Point3d& i_correction){
    try {
        Mat interresult,interresult32f,interresult_cutout,frame_cutoutArea;
        i_registratedFrame.copyTo(interresult);
        int rows = i_registratedFrame.rows;
        int cols = i_registratedFrame.cols;        
        i_frame(standardCutout).copyTo(frame_cutoutArea);
        interresult(standardCutout).copyTo(interresult_cutout);
        //qDebug()<<i_registratedFrame.rows<<" "<<i_registratedFrame.cols<<" vs "<<i_frame.rows<<" "<<i_frame.cols;
        //qDebug()<<interresult.rows<<" "<<interresult.cols<<" vs "<<i_frame.rows<<" "<<i_frame.cols;
        double R1 = calculateCorrCoef(i_frame,interresult,standardCutout);
        //qDebug()<<"Correction R1 "<<R1;
        Point3d shiftCorrection(0.0,0.0,0.0);
        shiftCorrection = pc_translation_hann(i_frame,interresult,maximalArea);
        //qDebug()<<"Shift correction "<<shiftCorrection.x<<" "<<shiftCorrection.y;
        if (std::abs(shiftCorrection.x) > 290 || std::abs(shiftCorrection.y) > 290)
        {
            shiftCorrection = pc_translation(i_frame,interresult,maximalArea);
        }
        if (std::abs(shiftCorrection.x) > 290 || std::abs(shiftCorrection.y) > 290)
        {
            shiftCorrection = pc_translation(frame_cutoutArea,interresult_cutout,maximalArea);
        }
        i_corrected = frameTranslation(interresult,shiftCorrection,rows,cols);
        double R2 = calculateCorrCoef(i_frame,i_corrected,standardCutout);
        if ((R2 > R1) && ((std::abs(shiftCorrection.x) > 0.3) || (std::abs(shiftCorrection.y) > 0.3)))
        {
            i_correction = shiftCorrection;
            return true;
        }
        else
            return true;

    } catch (std::exception &e) {
        qWarning()<<"Registration correction error: "<<e.what();
        return false;
    }
}
