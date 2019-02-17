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

qThreadSecondPart::qThreadSecondPart(QStringList &videos, cv::Rect &CO_standard, cv::Rect &CO_extra,
                                     QVector<QVector<int> > &badFramesCompleteList,
                                     QVector<int> &videoReferences, bool sC)
{
    videoList = videos;
    obtainedCutoffStandard = CO_standard;
    obtainedCutoffExtra = CO_extra;
    badFramesComplete = badFramesCompleteList;
    scaleChanged = sC;
    referencialFrames = videoReferences;
}

void qThreadSecondPart::run()
{
    /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
    /// for decision algorithms
    emit percentageCompleted(0);
    emit typeOfMethod(1);
    videoCount = double(videoList.count());
    qDebug()<<"Pocet videi: "<<videoCount;
    for (int kolikateVideo = 0; kolikateVideo < videoList.count(); kolikateVideo++)
    {
        QString fullPath = videoList.at(kolikateVideo);
        QString slozka,jmeno,koncovka;
        processFilePath(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(kolikateVideo);
        cv::VideoCapture capture = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        if (!capture.isOpened())
        {
            qWarning()<<"Unable to open"+fullPath;
            break;
        }
        int frameCount = int(capture.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> pom(frameCount,0);
        //emit typeOfMethod(4);
        QVector<double> snimky_pro_sigma;
        QVector<int> spatne_snimky_videa = badFramesComplete[kolikateVideo];
        snimky_pro_sigma = vectorForFWHM(spatne_snimky_videa,frameCount);
        if (capture.isOpened() == 0)
        {
            qWarning()<<"Video nelze pouzit pro analyzu entropie a tennengrada!";
            //return 0.0;
        }
        cv::Mat referencni_snimek_temp,referencni_snimek,referencni_snimek32f,referencni_vyrez;
        capture.set(CV_CAP_PROP_POS_FRAMES,referencialFrames[kolikateVideo]);
        capture.read(referencni_snimek_temp);
        int rows = 0;
        int cols = 0;
        if (scaleChanged == true)
        {
            referencni_snimek_temp(obtainedCutoffExtra).copyTo(referencni_snimek);
            rows = referencni_snimek.rows;
            cols = referencni_snimek.cols;
            referencni_snimek(obtainedCutoffStandard).copyTo(referencni_vyrez);
            referencni_snimek_temp.release();
        }
        else
        {
            referencni_snimek_temp.copyTo(referencni_snimek);
            rows = referencni_snimek.rows;
            cols = referencni_snimek.cols;
            referencni_snimek(obtainedCutoffStandard).copyTo(referencni_vyrez);
            referencni_snimek_temp.release();
        }
        QVector<double> zaznamenane_FWHM(snimky_pro_sigma.size(),0.0);
        QVector<double> zaznamenane_R(snimky_pro_sigma.size(),0.0);
        //kontrola_typu_snimku_32C1(referencni_snimek);
        //referencni_snimek.copyTo(referencni_snimek32f);
        //kontrola_typu_snimku_32(referencni_snimek32f);
        //cout << snimky_pro_sigma.size()<<" "<<zaznamenane_FWHM.size()<<endl;
        qDebug()<< "Analyza snimku pro urceni prumerneho korelacniho koeficientu a hodnoty FWHM";
        frameCount = snimky_pro_sigma.size();
        qDebug()<<"Pocet snimku: "<<frameCount;
        for (int j = 0; j < snimky_pro_sigma.size(); j++)
        {
            //qDebug()<<((kolikateVideo/videoCount)*100+((j/frameCount)*100.0)/videoCount);
            emit percentageCompleted(qRound((double(kolikateVideo)/videoCount)*100.0+(double(j)/frameCount)*100.0));
            cv::Mat posunuty_temp,posunuty,posunuty_vyrez;
            capture.set(CV_CAP_PROP_POS_FRAMES,snimky_pro_sigma[j]);
            if (capture.read(posunuty_temp) != 1)
            {
                qWarning()<<"Snimek "<<j<<" nelze slicovat!";
                continue;
            }
            else
            {
                if (scaleChanged == true)
                {
                    posunuty_temp(obtainedCutoffExtra).copyTo(posunuty);
                    posunuty(obtainedCutoffStandard).copyTo(posunuty_vyrez);
                    posunuty_temp.release();
                }
                else
                {
                    posunuty_temp.copyTo(posunuty);
                    posunuty(obtainedCutoffStandard).copyTo(posunuty_vyrez);
                    posunuty_temp.release();
                }
                //emit percentageCompleted(qRound((j/snimky_pro_sigma.size())*100.0));
                //percentsFromFunction(qRound((j/snimky_pro_sigma.size())*100.0));
                //cout << "\r" << procento << "%";
                cv::Point3d pt;
                pt.x = 0.0;pt.y = 0.0;pt.z = 0.0;
                //kontrola_typu_snimku_32C1(posunuty);
                if (scaleChanged == true)
                {
                    pt = fk_translace_hann(referencni_snimek,posunuty);
                    if (std::abs(pt.x)>=55 || std::abs(pt.y)>=55)
                    {
                        qDebug()<<"Too big translation, recalculating";
                        pt = fk_translace(referencni_snimek,posunuty);
                    }
                }
                if (scaleChanged == false)
                {
                    pt = fk_translace_hann(referencni_snimek,posunuty);
                }
                cv::Mat slicovany,slicovany_vyrez;
                //qDebug()<<pt.x<<pt.y<<pt.z;
                slicovany = translace_snimku(posunuty,pt,rows,cols);
                posunuty.release();
                slicovany(obtainedCutoffStandard).copyTo(slicovany_vyrez);
                double zSouradnice = pt.z;
                double sigma_gauss = 0.0;
                sigma_gauss = 1/(std::sqrt(2*CV_PI)*zSouradnice);
                double FWHM = 0.0;
                FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                zaznamenane_FWHM[j] = FWHM;
                double kk = vypocet_KK(referencni_snimek,slicovany,obtainedCutoffStandard);
                slicovany.release();
                slicovany_vyrez.release();
                zaznamenane_R[j] = kk;
                //qDebug()<<"snimek "<< snimky_pro_sigma[j] <<" ma R "<< kk << " a FWHM "<< FWHM;
            }

        }
        //cout << endl;
        //std::copy ( zaznamenane_R.begin(), zaznamenane_R.end(), out_it );
        //cout<<endl;
        double vypocteneFWHM = median_VectorDouble(zaznamenane_FWHM);
        double vypocteneR = median_VectorDouble(zaznamenane_R);
        qDebug()<<"Medians: FWHM: "<<vypocteneFWHM<<" and CC: "<<vypocteneR;
        CC.push_back(vypocteneR);
        FWHM.push_back(vypocteneFWHM);
    }
    emit percentageCompleted(100);
    emit done(2);
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
    //std::iota(cisla_pro_generator.begin(),cisla_pro_generator.end(),0);
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
    qDebug()<<snimky_pro_sigma;
    return snimky_pro_sigma;
}
