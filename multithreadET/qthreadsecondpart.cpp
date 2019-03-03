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

qThreadSecondPart::qThreadSecondPart(QStringList &_videosForAnalysis, cv::Rect &_cutoutStandard,
                                     cv::Rect &_cutoutExtra,
                                     QVector<QVector<int> > &_badFramesCompleteList,
                                     QVector<int> &_videoReferencialFramesList, bool _scaleChange)
{
    videoList = _videosForAnalysis;
    obtainedCutoffStandard = _cutoutStandard;
    obtainedCutoffExtra = _cutoutExtra;
    badFramesComplete = _badFramesCompleteList;
    scaleChanged = _scaleChange;
    referencialFrames = _videoReferencialFramesList;
}

void qThreadSecondPart::run()
{
    /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
    /// for decision algorithms
    emit percentageCompleted(0);
    emit typeOfMethod(1);
    bool errorOccured = false;
    videoCount = double(videoList.count());
    for (int videoIndex = 0; videoIndex < videoList.count(); videoIndex++)
    {
        QString fullPath = videoList.at(videoIndex);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(videoIndex);
        cv::VideoCapture capture = cv::VideoCapture(fullPath.toLocal8Bit().constData());

        int frameCount = int(capture.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> pom(frameCount,0);
        QVector<double> snimky_pro_sigma;
        QVector<int> spatne_snimky_videa = badFramesComplete[videoIndex];
        snimky_pro_sigma = vectorForFWHM(spatne_snimky_videa,frameCount);
        if (!capture.isOpened())
        {
            emit unexpectedTermination("Cannot open a video for analysis",2,"hardError");
            errorOccured = true;
            break;
        }
        cv::Mat referencialImage_temp,referencialImage,referencialImage32f,referencni_vyrez;
        capture.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[videoIndex]);
        capture.read(referencialImage_temp);
        int rows = 0;
        int cols = 0;
        if (scaleChanged == true)
        {
            referencialImage_temp(obtainedCutoffExtra).copyTo(referencialImage);
            rows = referencialImage.rows;
            cols = referencialImage.cols;
            referencialImage(obtainedCutoffStandard).copyTo(referencni_vyrez);
            referencialImage_temp.release();
        }
        else
        {
            referencialImage_temp.copyTo(referencialImage);
            rows = referencialImage.rows;
            cols = referencialImage.cols;
            referencialImage(obtainedCutoffStandard).copyTo(referencni_vyrez);
            referencialImage_temp.release();
        }
        QVector<double> computedFWHM(snimky_pro_sigma.size(),0.0);
        QVector<double> computedCC(snimky_pro_sigma.size(),0.0);
        frameCount = snimky_pro_sigma.size();
        qDebug()<<"Frame count: "<<frameCount;
        for (int j = 0; j < snimky_pro_sigma.size(); j++)
        {
            emit percentageCompleted(qRound((double(videoIndex)/videoCount)*100.0+(double(j)/frameCount)*100.0));
            cv::Mat translated_temp,translated,translated_vyrez;
            capture.set(CV_CAP_PROP_POS_FRAMES,snimky_pro_sigma[j]);
            if (!capture.read(translated_temp))
            {
                QString errorMessage = QString("Frame number %1 could not be opened").arg(j);
                unexpectedTermination(errorMessage,2,"softError");
                continue;
            }
            else
            {
                if (scaleChanged == true)
                {
                    translated_temp(obtainedCutoffExtra).copyTo(translated);
                    translated(obtainedCutoffStandard).copyTo(translated_vyrez);
                    translated_temp.release();
                }
                else
                {
                    translated_temp.copyTo(translated);
                    translated(obtainedCutoffStandard).copyTo(translated_vyrez);
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
                double zSouradnice = pt.z;
                double sigma_gauss = 0.0;
                sigma_gauss = 1/(std::sqrt(2*CV_PI)*zSouradnice);
                double FWHM = 0.0;
                FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                computedFWHM[j] = FWHM;
                double kk = vypocet_KK(referencialImage,registrated,obtainedCutoffStandard);
                registrated.release();
                computedCC[j] = kk;
                qDebug()<<"Frame "<< snimky_pro_sigma[j] <<" has CC "<< kk << " a FWHM "<< FWHM;
            }

        }
        double FWHMcomputed = median_VectorDouble(computedFWHM);
        double Rcomputed = median_VectorDouble(computedCC);
        qDebug()<<"Medians: FWHM: "<<FWHMcomputed<<" and CC: "<<Rcomputed;
        CC.push_back(Rcomputed);
        FWHM.push_back(FWHMcomputed);
    }
    if (!errorOccured){
        emit percentageCompleted(100);
        emit done(2);
    }
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
    QVector<double> snimky_pro_sigma((frameCount-velikost_spojeneho_vektoru-10),0);
    QVector<double> cisla_pro_generator(frameCount,0);
    std::generate(cisla_pro_generator.begin(), cisla_pro_generator.end(), [n = 0] () mutable { return n++; });

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, frameCount-1);
    QVector<int>::iterator it;
    for (int i = 0; i < snimky_pro_sigma.size(); i++)
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
            {snimky_pro_sigma[i] = vygenerovane_cislo;
                cisla_pro_generator[vygenerovane_cislo] = 0;
                kontrola_ulozeni = 1;}
        }
    }
    std::sort(snimky_pro_sigma.begin(),snimky_pro_sigma.end());
    qDebug()<<"Frames for sigma calculation: "<<snimky_pro_sigma;
    return snimky_pro_sigma;
}
