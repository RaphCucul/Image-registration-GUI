#include "qthreadsecondpart.h"
#include <QDebug>
#include <QVector>
#include <QStringList>

#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "licovani/fazova_korelace_funkce.h"
#include "util/prace_s_vektory.h"
#include "util/souborove_operace.h"

qThreadSecondPart::qThreadSecondPart(QStringList i_videosForAnalysis,
                                     QVector<int> i_badVideos,
                                     QVector<cv::Rect> i_cutoutStandard,
                                     QVector<cv::Rect> i_cutoutExtra,
                                     QVector<QVector<int> > i_badFramesCompleteList,
                                     QVector<int> i_videoReferencialFramesList, bool i_scaleChange)
{
    videoList = i_videosForAnalysis;
    obtainedCutoffStandard = i_cutoutStandard;
    obtainedCutoffExtra = i_cutoutExtra;
    badFramesComplete = i_badFramesCompleteList;
    scaleChanged = i_scaleChange;
    referencialFrames = i_videoReferencialFramesList;
    notProcessThese = i_badVideos;

    emit setTerminationEnabled(true);
}

void qThreadSecondPart::run()
{
    /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
    /// for decision algorithms
    emit percentageCompleted(0);
    emit typeOfMethod(1);
    videoCount = double(videoList.count());
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        if (notProcessThese.indexOf(videoIndex) == -1){
            QString fullPath = videoList.at(videoIndex);
            QString folder,filename,suffix;
            processFilePath(fullPath,folder,filename,suffix);
            emit actualVideo(videoIndex);
            cv::VideoCapture capture = cv::VideoCapture(fullPath.toLocal8Bit().constData());
            if (!capture.isOpened()){
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty();
                continue;
            }

            int frameCount = int(capture.get(CV_CAP_PROP_FRAME_COUNT));
            QVector<double> pom(frameCount,0);
            QVector<double> framesForSigma;
            QVector<int> spatne_snimky_videa = badFramesComplete[videoIndex];
            framesForSigma = vectorForFWHM(spatne_snimky_videa,frameCount);
            if (!capture.isOpened())
            {
                emit unexpectedTermination(videoIndex,"hardError");
                fillEmpty();
                continue;
            }
            cv::Mat referencialImage_temp,referencialImage,referencialImage32f,referencni_vyrez;
            capture.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[videoIndex]);
            capture.read(referencialImage_temp);
            int rows = 0;
            int cols = 0;
            cv::Rect _tempStandard,_tempExtra;
            _tempStandard = obtainedCutoffStandard[videoIndex];
            _tempExtra = obtainedCutoffExtra[videoIndex];
            if (scaleChanged == true)
            {
                referencialImage_temp(_tempExtra).copyTo(referencialImage);
                rows = referencialImage.rows;
                cols = referencialImage.cols;
                referencialImage(_tempStandard).copyTo(referencni_vyrez);
                referencialImage_temp.release();
            }
            else
            {
                referencialImage_temp.copyTo(referencialImage);
                rows = referencialImage.rows;
                cols = referencialImage.cols;
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
                    fillEmpty();
                    continue;
                }
                else
                {
                    if (scaleChanged == true)
                    {
                        translated_temp(_tempExtra).copyTo(translated);
                        translated(_tempStandard).copyTo(translated_vyrez);
                        translated_temp.release();
                    }
                    else
                    {
                        translated_temp.copyTo(translated);
                        translated(_tempStandard).copyTo(translated_vyrez);
                        translated_temp.release();
                    }
                    cv::Point3d pt;
                    pt.x = 0.0;pt.y = 0.0;pt.z = 0.0;
                    if (scaleChanged == true)
                    {
                        pt = fk_translace_hann(referencialImage,translated);
                        if (std::abs(pt.x)>=55 || std::abs(pt.y)>=55)
                        {
                            qDebug()<<"Too big translation, recalculating";
                            pt = fk_translace(referencialImage,translated);
                        }
                    }
                    if (scaleChanged == false)
                    {
                        pt = fk_translace_hann(referencialImage,translated);
                    }
                    cv::Mat registrated;
                    registrated = translace_snimku(translated,pt,rows,cols);
                    translated.release();
                    double zCoords = pt.z;
                    double sigma_gauss = 0.0;
                    sigma_gauss = 1/(std::sqrt(2*CV_PI)*zCoords);
                    double FWHM = 0.0;
                    FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                    computedFWHM[j] = FWHM;
                    double cc = vypocet_KK(referencialImage,registrated,_tempStandard);
                    registrated.release();
                    computedCC[j] = cc;
                    qDebug()<<"Frame "<< framesForSigma[j] <<" has CC "<< cc << " a FWHM "<< FWHM;
                }

            }
            double FWHMcomputed = median_VectorDouble(computedFWHM);
            double Rcomputed = median_VectorDouble(computedCC);
            qDebug()<<"Medians: FWHM: "<<FWHMcomputed<<" and CC: "<<Rcomputed;
            CC.push_back(Rcomputed);
            FWHM.push_back(FWHMcomputed);
        }
        else{
            fillEmpty();
        }
    }
    emit percentageCompleted(100);
    emit done(2);

}

void qThreadSecondPart::fillEmpty(){
    CC.push_back(0.0);
    FWHM.push_back(0.0);
}

QVector<double> qThreadSecondPart::computedCC()
{
    return CC;
}
QVector<double> qThreadSecondPart::computedFWHM()
{
    return  FWHM;
}
QVector<double> qThreadSecondPart::vectorForFWHM(QVector<int>& badFrames,
                                                 int frameCount)
{
    int velikost_spojeneho_vektoru = badFrames.size();
    QVector<double> framesForSigma((frameCount-velikost_spojeneho_vektoru-10),0);
    QVector<double> cisla_pro_generator(frameCount,0);
    std::generate(cisla_pro_generator.begin(), cisla_pro_generator.end(), [n = 0] () mutable { return n++; });

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, frameCount-1);
    QVector<int>::iterator it;
    for (int i = 0; i < framesForSigma.size(); i++)
    {
        int kontrola_ulozeni = 0;
        while (kontrola_ulozeni == 0)
        {
            int vygenerovane_cislo = distr(eng);
            while (cisla_pro_generator[vygenerovane_cislo] == 0.0)
            {
                vygenerovane_cislo = distr(eng);
            }
            it = std::find(badFrames.begin(), badFrames.end(), cisla_pro_generator[vygenerovane_cislo]);
            if (it != badFrames.end())
            {kontrola_ulozeni = 0;}
            else
            {framesForSigma[i] = vygenerovane_cislo;
                cisla_pro_generator[vygenerovane_cislo] = 0;
                kontrola_ulozeni = 1;}
        }
    }
    std::sort(framesForSigma.begin(),framesForSigma.end());
    qDebug()<<"Frames for sigma calculation: "<<framesForSigma;
    return framesForSigma;
}

void qThreadSecondPart::onDataObtained(){
    emit readyForFinish();
}
