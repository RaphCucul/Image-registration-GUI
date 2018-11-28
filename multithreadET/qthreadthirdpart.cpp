#include "multithreadET/qthreadthirdpart.h"
#include <QDebug>
#include <QVector>
#include <QStringList>
#include <QThread>

#include "licovani/fazova_korelace_funkce.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "util/souborove_operace.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using cv::Mat;
using cv::Rect;

qThreadThirdPart::qThreadThirdPart(QStringList &sV, QVector<QVector<int> >& sSprOhKomplet,
                                   QVector<QVector<int> > &ohodKomp, QVector<int>& refKomplet,
                                   QVector<double> &RK, QVector<double> &FWHMK,
                                   cv::Rect &VK_standard, cv::Rect &VK_extra, bool zM, QObject *parent):QThread(parent)
{
    seznamVidei = sV;
    spatne_snimky_prvotni_ohodnoceni = sSprOhKomplet;
    ohodnoceniSnimkuKomplet = ohodKomp;
    referenceKompletni = refKomplet;
    vyrez_korelace_standard = VK_standard;
    vyrez_korelace_extra = VK_extra;
    zmenaMeritka = zM;
    prumernyKKkomplet = RK;
    prumernyFWHMkomplet = FWHMK;
}

void qThreadThirdPart::run()
{
    emit typeOfMethod(2);
    emit percentageCompleted(0);
    pocetVidei = double(seznamVidei.count());
    for (int indexVidea=0; indexVidea < seznamVidei.count(); indexVidea++)
    {
        QVector<int> spatneSnimky;
        spatneSnimky = spatne_snimky_prvotni_ohodnoceni[indexVidea];
        QString fullPath = seznamVidei.at(indexVidea);
        QString slozka,jmeno,koncovka;
        zpracujJmeno(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(indexVidea);
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        if (!cap.isOpened())
        {
            qWarning()<<"Unable to open"+fullPath;
            break;
        }
        cv::Mat referencni_snimek_temp,referencni_snimek,referencni_snimek32f,referencni_vyrez;
        cap.set(CV_CAP_PROP_POS_FRAMES,referenceKompletni[indexVidea]);
        if (!cap.read(referencni_snimek_temp))
        {
                qWarning()<<"Referrence image cannot be read!";
                break;
        }
        int rows;
        int cols;
        if (zmenaMeritka == true)
        {
            referencni_snimek_temp(vyrez_korelace_extra).copyTo(referencni_snimek);
            rows = referencni_snimek.rows;
            cols = referencni_snimek.cols;
            referencni_snimek(vyrez_korelace_standard).copyTo(referencni_vyrez);
            referencni_snimek_temp.release();
        }
        else
        {
            referencni_snimek_temp.copyTo(referencni_snimek);
            rows = referencni_snimek.rows;
            cols = referencni_snimek.cols;
            referencni_snimek(vyrez_korelace_standard).copyTo(referencni_vyrez);
            referencni_snimek_temp.release();
        }
        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> pomVecD(frameCount,0.0);
        QVector<double> POC_x,POC_y,uhel,frangi_x,frangi_y,frangi_euklid,vypoctene_hodnoty_FWHM,
                vypoctene_hodnoty_R;
        frangi_x = pomVecD;frangi_y=pomVecD;frangi_euklid=pomVecD;
        POC_x=pomVecD;POC_y=pomVecD;uhel=pomVecD;
        QVector<int> snimky_k_provereni_prvni;
        pocetSnimku = double(spatneSnimky.length());
        for (int i = 0;i < spatneSnimky.length(); i++)
        {
            emit percentageCompleted(qRound((double(indexVidea)/pocetVidei)*100.0+((i/pocetSnimku)*100.0)/pocetVidei));
            Mat posunuty_temp,posunuty,posunuty_vyrez,slicovany,slicovany_vyrez;
            Mat obraz, obraz_vyrez;
            referencni_snimek.copyTo(obraz);
            obraz(vyrez_korelace_standard).copyTo(obraz_vyrez);
            int rows = obraz.rows;
            int cols = obraz.cols;
            cap.set(CV_CAP_PROP_POS_FRAMES,spatneSnimky[i]);
            if (cap.read(posunuty_temp) != 1)
            {
                qWarning()<<"Snimek "<<spatneSnimky[i]<<" nelze slicovat!";
                POC_x[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999.0;
                POC_y[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999.0;
                uhel[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999.0;
                frangi_x[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999.0;
                frangi_y[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999.0;
                frangi_euklid[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999.0;
                continue;
            }
            if (zmenaMeritka == true)
            {
                posunuty_temp(vyrez_korelace_extra).copyTo(posunuty);
                posunuty(vyrez_korelace_standard).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
                posunuty(vyrez_korelace_standard).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            cv::Point3d pt(0,0,0);
            if (zmenaMeritka == true)
            {
                pt = fk_translace_hann(obraz,posunuty);
                if (std::abs(pt.x)>=290 || std::abs(pt.y)>=290)
                {
                    pt = fk_translace(obraz,posunuty);
                }
            }
            if (zmenaMeritka == false)
            {
                pt = fk_translace_hann(obraz,posunuty);
            }
            if (pt.x >= 55 || pt.y >= 55)
            {
                qDebug()<< "Snimek "<< spatneSnimky[i]<< " nepripusten k analyze.";
                ohodnoceniSnimkuKomplet[indexVidea][spatneSnimky[i]] = 5;
                POC_x[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                POC_y[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                uhel[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                frangi_x[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                frangi_y[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                frangi_euklid[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                posunuty.release();
            }
            else
            {
                double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt.z);
                double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                slicovany = translace_snimku(posunuty,pt,rows,cols);
                slicovany(vyrez_korelace_standard).copyTo(slicovany_vyrez);
                double R = vypocet_KK(obraz,slicovany,vyrez_korelace_standard);
                qDebug() <<"Zkoumam snimek"<<spatneSnimky[i]<< "R " << R <<" a FWHM " << FWHM;
                slicovany.release();
                slicovany_vyrez.release();
                posunuty.release();
                double rozdilnostKK = prumernyKKkomplet[indexVidea]-R;
                double rozdilnostFWHM = prumernyFWHMkomplet[indexVidea]-FWHM;
                if ((std::abs(rozdilnostKK) < 0.02) && (FWHM < prumernyFWHMkomplet[indexVidea])) //1.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " je vhodny ke slicovani.";
                    ohodnoceniSnimkuKomplet[indexVidea][spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 0.0;
                    continue;
                }
                else if (R > prumernyKKkomplet[indexVidea] && (std::abs(rozdilnostFWHM)<=2||(FWHM < prumernyFWHMkomplet[indexVidea]))) //5.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " je vhodny ke slicovani.";
                    ohodnoceniSnimkuKomplet[indexVidea][spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 0.0;
                    continue;
                }
                else if (R >= prumernyKKkomplet[indexVidea] && FWHM > prumernyFWHMkomplet[indexVidea]) //4.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " je vhodny ke slicovani.";
                    ohodnoceniSnimkuKomplet[indexVidea][spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 0.0;
                    continue;
                }
                else if ((std::abs(rozdilnostKK) <= 0.02) && (FWHM > prumernyFWHMkomplet[indexVidea])) //2.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " bude proveren.";
                    snimky_k_provereni_prvni.push_back(spatne_snimky_prvotni_ohodnoceni[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }
                else if ((rozdilnostKK > 0.02) && (rozdilnostKK < 0.18)) //3.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " bude proveren.";
                    snimky_k_provereni_prvni.push_back(spatne_snimky_prvotni_ohodnoceni[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }
                else if ((rozdilnostKK >= 0.05 && rozdilnostKK < 0.18) && ((FWHM < prumernyFWHMkomplet[indexVidea]) || prumernyFWHMkomplet[indexVidea] > 35.0)) //6.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " bude proveren.";
                    snimky_k_provereni_prvni.push_back(spatne_snimky_prvotni_ohodnoceni[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }
                else if ((rozdilnostKK >= 0.05 && rozdilnostKK < 0.18) && (FWHM <= (prumernyFWHMkomplet[indexVidea]+10))) //8.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " bude proveren.";
                    snimky_k_provereni_prvni.push_back(spatne_snimky_prvotni_ohodnoceni[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);
                    continue;
                }

                else if ((rozdilnostKK >= 0.2) && (FWHM > (prumernyFWHMkomplet[indexVidea]+10))) //7.
                {
                    qDebug()<< "Snimek "<< spatneSnimky[i]<< " nepripusten k analyze.";
                    ohodnoceniSnimkuKomplet[indexVidea][spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 5;
                    POC_x[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                    POC_y[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                    uhel[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                    frangi_x[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                    frangi_y[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                    frangi_euklid[spatne_snimky_prvotni_ohodnoceni[indexVidea][i]] = 999;
                    continue;
                }
                else
                {
                    qDebug() << "Snimek "<< spatneSnimky[i]<< " bude proveren - nevyhovel nikde.";
                    snimky_k_provereni_prvni.push_back(spatne_snimky_prvotni_ohodnoceni[indexVidea][i]);
                    vypoctene_hodnoty_FWHM.push_back(FWHM);
                    vypoctene_hodnoty_R.push_back(R);

                }
            }

        }
        snimkyVypocteneR.append(vypoctene_hodnoty_R);
        snimkyVypocteneFWHM.append(vypoctene_hodnoty_FWHM);
        snimkyProvereniPrvniKomplet.append(snimky_k_provereni_prvni);
        snimkyFrangiX.append(frangi_x);
        snimkyFrangiY.append(frangi_y);
        snimkyFrangiEuklid.append(frangi_euklid);
        snimkyPOCX.append(POC_x);
        snimkyPOCY.append(POC_y);
        snimkyUhel.append(uhel);
    }
    emit percentageCompleted(100);
    emit hotovo(3);
}
QVector<QVector<int>> qThreadThirdPart::snimkyUpdateOhodnoceni()
{
    return ohodnoceniSnimkuKomplet;
}
QVector<QVector<int>> qThreadThirdPart::snimkyProvereniPrvniKompletestimated()
{
    return snimkyProvereniPrvniKomplet;
}
QVector<QVector<double>> qThreadThirdPart::snimkyProblematicke_R()
{
    return snimkyVypocteneR;
}
QVector<QVector<double>> qThreadThirdPart::snimkyProblematicke_FWHM()
{
    return snimkyVypocteneFWHM;
}
QVector<QVector<double>> qThreadThirdPart::snimkyFrangiXestimated()
{
    return snimkyFrangiX;
}
QVector<QVector<double>> qThreadThirdPart::snimkyFrangiYestimated()
{
    return snimkyFrangiY;
}
QVector<QVector<double>> qThreadThirdPart::snimkyFrangiEuklidestimated()
{
    return snimkyFrangiEuklid;
}
QVector<QVector<double>> qThreadThirdPart::snimkyPOCXestimated()
{
    return snimkyPOCX;
}
QVector<QVector<double>> qThreadThirdPart::snimkyPOCYestimated()
{
    return snimkyPOCY;
}
QVector<QVector<double>> qThreadThirdPart::snimkyUhelestimated()
{
    return snimkyUhel;
}
