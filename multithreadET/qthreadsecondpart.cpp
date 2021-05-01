#include "qthreadsecondpart.h"
#include <QDebug>
#include <QVector>
#include <QStringList>
#include <random>

#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "image_analysis/correlation_coefficient.h"
#include "image_analysis/frangi_utilization.h"
#include "registration/phase_correlation_function.h"
#include "registration/multiPOC_Ai1.h"
#include "util/vector_operations.h"
#include "util/files_folders_operations.h"

qThreadSecondPart::qThreadSecondPart(QStringList i_videosForAnalysis,
                                     QVector<QString> i_badVideos,
                                     QMap<QString, cv::Rect> i_cutoutStandard,
                                     QMap<QString, cv::Rect> i_cutoutExtra,
                                     QMap<QString, QVector<int> > i_badFramesCompleteList,
                                     QMap<QString,int> i_videoReferencialFramesList,
                                     cutoutType i_cutoutType,
                                     double i_areaMaximum)
{
    videoList = i_videosForAnalysis;
    obtainedCutoffStandard = i_cutoutStandard;
    obtainedCutoffExtra = i_cutoutExtra;
    badFramesComplete = i_badFramesCompleteList;
    selectedCutout = i_cutoutType;
    referencialFrames = i_videoReferencialFramesList;
    notProcessThis = i_badVideos;
    areaMaximum = i_areaMaximum;

    emit setTerminationEnabled(true);
}

void qThreadSecondPart::run()
{
    // Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
    // for decision algorithms
    emit percentageCompleted(0);
    emit typeOfMethod(1);
    videoCount = double(videoList.count());
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        QString fullPath = videoList.at(videoIndex);
        QString folder,filename,suffix;
        processFilePath(fullPath,folder,filename,suffix);
        if (notProcessThis.indexOf(filename) == -1)
        {
            emit actualVideo(videoIndex);
            cv::VideoCapture capture = cv::VideoCapture(fullPath.toLocal8Bit().constData());
            if (!capture.isOpened()){
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(filename);
                continue;
            }

            int frameCount = int(capture.get(CV_CAP_PROP_FRAME_COUNT));
            QVector<double> pom(frameCount,0);
            QVector<double> framesForSigma;
            QVector<int> bad_frames_vector = badFramesComplete[filename];
            framesForSigma = vectorForFWHM(bad_frames_vector,frameCount);
            if (!capture.isOpened())
            {
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty(filename);
                continue;
            }
            cv::Mat referencialImage_temp,referencialImage,referencialImage32f,referencni_vyrez;
            capture.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[filename]);
            capture.read(referencialImage_temp);
            int rows = 0;
            int cols = 0;
            cv::Rect _tempStandard,_tempExtra;
            _tempStandard = obtainedCutoffStandard.contains(filename) ? obtainedCutoffStandard[filename] : cv::Rect(0,0,0,0);
            _tempExtra = obtainedCutoffExtra.contains(filename) ? obtainedCutoffExtra[filename] : cv::Rect(0,0,0,0);

            if (selectedCutout == cutoutType::EXTRA && (_tempExtra.width > 0 && _tempExtra.height > 0))
            {
                qDebug()<<"Extra cutout: "<<_tempExtra.height<<" "<<_tempExtra.width;
                referencialImage_temp(_tempExtra).copyTo(referencialImage);
                rows = referencialImage.rows;
                cols = referencialImage.cols;
                qDebug()<<"Frame size "<<rows<<" "<<cols;
                qDebug()<<"Standard cutout: "<<_tempStandard.height<<" "<<_tempStandard.width<<" "<<_tempStandard.x<<" "<<_tempStandard.y;
                referencialImage(_tempStandard).copyTo(referencni_vyrez);
                referencialImage_temp.release();
            }
            else
            {
                referencialImage_temp.copyTo(referencialImage);
                rows = referencialImage.rows;
                cols = referencialImage.cols;
                qDebug()<<"Frame size "<<rows<<" "<<cols;
                qDebug()<<"Standard cutout: "<<_tempStandard.height<<" "<<_tempStandard.width;
                referencialImage(_tempStandard).copyTo(referencni_vyrez);
                referencialImage_temp.release();
            }
            QVector<double> computedFWHM(framesForSigma.size(),0.0);
            QVector<double> computedCC(framesForSigma.size(),0.0);
            frameCount = framesForSigma.size();
            qDebug()<<"Frame count: "<<frameCount;
            for (int j = 0; j < framesForSigma.size(); j++)
            {
                emit percentageCompleted(qRound((double(videoIndex)/double(videoCount))*100.0+1.0+((double(j)/double(frameCount))*100.0)/double(videoCount)));
                cv::Mat translated_temp,translated,translated_vyrez;
                capture.set(CV_CAP_PROP_POS_FRAMES,framesForSigma[j]);
                if (!capture.read(translated_temp))
                {
                    QString errorMessage = QString("Frame number %1 could not be opened").arg(j);
                    unexpectedTermination(videoIndex,"hardError");
                    fillEmpty(filename);
                    continue;
                }
                else
                {
                    if (selectedCutout == cutoutType::EXTRA  && (_tempExtra.width > 0 && _tempExtra.height > 0))
                    {
                        translated_temp(_tempExtra).copyTo(translated);
                    }
                    else
                    {
                        translated_temp.copyTo(translated);
                    }
                    translated(_tempStandard).copyTo(translated_vyrez);
                    translated_temp.release();
                    cv::Point3d pt;
                    pt.x = 0.0;pt.y = 0.0;pt.z = 0.0;

                    if (selectedCutout != cutoutType::NO_CUTOUT)
                    {
                        pt = pc_translation_hann(referencialImage,translated,areaMaximum);
                        if (std::abs(pt.x)>=55 || std::abs(pt.y)>=55)
                        {
                            qDebug()<<"Too big translation, recalculating";
                            pt = pc_translation(referencialImage,translated,areaMaximum);
                        }
                    }
                    else
                    {
                        pt = pc_translation_hann(referencialImage,translated,areaMaximum);
                    }
                    cv::Mat registrated;
                    registrated = frameTranslation(translated,pt,rows,cols);
                    translated.release();
                    double zCoords = pt.z;
                    double sigma_gauss = 0.0;
                    sigma_gauss = 1/(std::sqrt(2*CV_PI)*zCoords);
                    double FWHM = 0.0;
                    FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                    computedFWHM[j] = FWHM;
                    double cc = calculateCorrCoef(referencialImage,registrated,_tempStandard);
                    registrated.release();
                    computedCC[j] = cc;
                    qDebug()<<"Frame "<< framesForSigma[j] <<" has CC "<< cc << " a FWHM "<< FWHM;
                }

            }
            double FWHMcomputed = median_VectorDouble(computedFWHM);
            double Rcomputed = median_VectorDouble(computedCC);
            qDebug()<<"Medians: FWHM: "<<FWHMcomputed<<" and CC: "<<Rcomputed;
            CC.insert(filename,Rcomputed);
            FWHM.insert(filename,FWHMcomputed);
        }
        else{
            fillEmpty(filename);
        }
    }
    emit percentageCompleted(100);
    emit done(2);
}

void qThreadSecondPart::fillEmpty(QString i_videoName){
    /*CC.insert(i_videoName,0.0);
    FWHM.insert(i_videoName,0.0);*/
    notProcessThis.push_back(i_videoName);
}

QVector<QString> qThreadSecondPart::unprocessableVideos() {
    return notProcessThis;
}

QMap<QString, double> qThreadSecondPart::computedCC()
{
    return CC;
}
QMap<QString,double> qThreadSecondPart::computedFWHM()
{
    return  FWHM;
}
QVector<double> qThreadSecondPart::vectorForFWHM(QVector<int>& badFrames,
                                                 int frameCount)
{
    int mergedVectorSize = badFrames.size();
    QVector<double> framesForSigma((frameCount-mergedVectorSize-10),0);
    QVector<double> numberForGenerator(frameCount,0);
    std::generate(numberForGenerator.begin(), numberForGenerator.end(), [n = 0] () mutable { return n++; });

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, frameCount-1);
    QVector<int>::iterator it;
    for (int i = 0; i < framesForSigma.size(); i++)
    {
        int saveControl = 0;
        while (saveControl == 0)
        {
            int generatedNumber = distr(eng);
            while (numberForGenerator[generatedNumber] == 0.0)
            {
                generatedNumber = distr(eng);
            }
            it = std::find(badFrames.begin(), badFrames.end(), numberForGenerator[generatedNumber]);
            if (it != badFrames.end())
            {saveControl = 0;}
            else
            {framesForSigma[i] = generatedNumber;
                numberForGenerator[generatedNumber] = 0;
                saveControl = 1;}
        }
    }
    std::sort(framesForSigma.begin(),framesForSigma.end());
    qDebug()<<"Frames for sigma calculation: "<<framesForSigma;
    return framesForSigma;
}

void qThreadSecondPart::onDataObtained(){
    emit readyForFinish();
}
