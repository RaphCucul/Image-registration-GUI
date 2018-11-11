#include "multithreadET/qthreadfifthpart.h"
#include "licovani/fazova_korelace_funkce.h"
#include "licovani/multiPOC_Ai1.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/korelacni_koeficient.h"

#include <QThread>
#include <QStringList>
#include <QVector>
#include <QDebug>

#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using cv::Mat;
using cv::Rect;
using cv::Point3d;

qThreadFifthPart::qThreadFifthPart(QStringList &sV,
                                   cv::Rect &VK_s,
                                   cv::Rect &VK_e,
                                   QVector<QVector<double> > &POCX,
                                   QVector<QVector<double> > &POCY,
                                   QVector<QVector<double> > &U,
                                   QVector<QVector<double> > &F_X,
                                   QVector<QVector<double> > &F_Y,
                                   QVector<QVector<double> > &F_E,
                                   bool zmenaMeritka,
                                   bool casovaZnacka,
                                   QVector<QVector<int>> &hodnoceni_vsech_snimku_videa,
                                   QVector<QVector<int>>& sProverDruhy, QVector<int> &referencniSnimkyVidei,
                                   QVector<double> &parametryFrangianalyzy,
                                   QObject *parent):QThread(parent)
{
    seznamVidei = sV;
    vyrez_korelace_standard = VK_s;
    vyrez_korelace_extra = VK_e;
    POC_x = POCX;
    POC_y = POCY;
    uhel = U;
    frangi_x = F_X;
    frangi_y = F_Y;
    frangi_euklid = F_E;
    zmena_meritka = zmenaMeritka;
    casova_znacka = casovaZnacka;
    ohodnoceniSnimkuKomplet = hodnoceni_vsech_snimku_videa;
    snimky_k_provereni_druhy = sProverDruhy;
    parametryFrangi = parametryFrangianalyzy;
    referenceKompletni = referencniSnimkyVidei;
}

void qThreadFifthPart::run()
{
    emit typeOfMethod(4);
    emit percentageCompleted(0);
    pocetVidei = double(seznamVidei.count());
    for (int indexVidea = 0; indexVidea < seznamVidei.count(); indexVidea++)
    {
        pocetSnimku = double(snimky_k_provereni_druhy[indexVidea].length());
        QString fullPath = seznamVidei.at(indexVidea);
        QString slozka,jmeno,koncovka;
        zpracujJmeno(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(jmeno);
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
        if (zmena_meritka == true)
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
        cv::Point3d pt_temp(0,0,0);
        Point3d obraz_frangi_reverse = frangi_analyza(referencni_snimek,2,2,0,"",1,false,pt_temp,parametryFrangi);
        Mat obraz_vyrez;
        referencni_snimek(vyrez_korelace_standard).copyTo(referencni_vyrez);
        //for (unsigned int i = 0; i < 1; i++)
        pocetSnimku = double(snimky_k_provereni_druhy[indexVidea].length());
        for (int i = 0; i < snimky_k_provereni_druhy[indexVidea].length(); i++) //snimky_k_provereni2.size()
        {
            emit percentageCompleted(qRound((indexVidea/pocetVidei)*100+((i/pocetSnimku)*100.0)/pocetVidei));
            Mat slicovan_kompletne = cv::Mat::zeros(referencni_snimek.size(), CV_32FC3);
            Point3d mira_translace(0.0,0.0,0.0);
            double celkovy_uhel = 0.0;
            int iterace = -1;double oblastMaxima = 5.0;double uhelMaximalni = 0.1;
            int uspech_licovani = kompletni_slicovani(cap,referencni_snimek,
                                                      snimky_k_provereni_druhy[indexVidea][i],
                                                      iterace,
                                                      oblastMaxima,
                                                      uhelMaximalni,
                                                      vyrez_korelace_extra,
                                                      vyrez_korelace_standard,
                                                      zmena_meritka,
                                                      slicovan_kompletne,
                                                      mira_translace,celkovy_uhel);
            qDebug() << snimky_k_provereni_druhy[indexVidea][i] <<" -> ";
            if (uspech_licovani == 0)
            {

                qDebug()  << "nelze slicovat, ohodnocení: 5";
                ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 5.0;
                POC_x[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                POC_y[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                uhel[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                frangi_x[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                frangi_y[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                frangi_euklid[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                continue;
            }
            else
            {
                Mat mezivysledek32f,mezivysledek32f_vyrez;
                slicovan_kompletne.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
                double R1 = vypocet_KK(referencni_snimek,slicovan_kompletne,vyrez_korelace_standard);
                mezivysledek32f.release();
                mezivysledek32f_vyrez.release();
                Point3d korekce_bod(0,0,0);
                if (zmena_meritka == true)
                {
                    korekce_bod = fk_translace(referencni_snimek,slicovan_kompletne);
                    if (std::abs(korekce_bod.x)>=290 || std::abs(korekce_bod.y)>=290)
                    {
                        korekce_bod = fk_translace_hann(referencni_snimek,slicovan_kompletne);
                    }
                }
                else
                {
                    korekce_bod = fk_translace_hann(referencni_snimek,slicovan_kompletne);
                }
                Mat korekce = translace_snimku(slicovan_kompletne,korekce_bod,rows,cols);
                korekce.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
                double R2 = vypocet_KK(referencni_snimek,korekce,vyrez_korelace_standard);
                Point3d slicovany_frangi_reverse(0,0,0);
                double rozdil = R2-R1;
                if (rozdil>0.015)
                {
                    cv::Point3d extra_translace(0,0,0);
                    extra_translace.x = mira_translace.x+korekce_bod.x;
                    extra_translace.y = mira_translace.y+korekce_bod.y;
                    extra_translace.z = mira_translace.z;
                    qDebug()<< "Provedena korekce posunuti pro objektivnejsi analyzu skrze cevy.";
                    slicovany_frangi_reverse = frangi_analyza(korekce,2,2,0,"",2,false,extra_translace,parametryFrangi);
                }
                else
                {
                    slicovany_frangi_reverse = frangi_analyza(slicovan_kompletne,2,2,0,"",2,false,mira_translace,parametryFrangi);
                }
                slicovan_kompletne.release();
                if (slicovany_frangi_reverse.z == 0.0)
                {

                    qDebug()<< "Nelze zjistit maximum Frangiho funkce, ohodnoceni: 5";
                    ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 5.0;
                    POC_x[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                    POC_y[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                    uhel[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                    frangi_x[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                    frangi_y[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                    frangi_euklid[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 999.0;
                    continue;
                }
                else
                {
                    double rozdil_x = obraz_frangi_reverse.x - slicovany_frangi_reverse.x;
                    double rozdil_y = obraz_frangi_reverse.y - slicovany_frangi_reverse.y;
                    double suma_rozdilu = std::pow(rozdil_x,2.0) + std::pow(rozdil_y,2.0);
                    double euklid = std::sqrt(suma_rozdilu);
                    if (euklid <= 1.2)
                    {
                        ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 0;
                        qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 0";
                    }
                    else if (euklid > 1.2 && euklid < 10)
                    {
                        ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 1;
                        qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 1";
                        frangi_x[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = slicovany_frangi_reverse.x;
                        frangi_y[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = slicovany_frangi_reverse.y;
                        frangi_euklid[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = euklid;
                    }
                    else if (euklid >=10)
                    {
                        ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = 4;
                        qDebug()<< "euklid. vzdal. je "<<euklid<<", ohodnoceni: 4";
                        frangi_x[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = slicovany_frangi_reverse.x;
                        frangi_y[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = slicovany_frangi_reverse.y;
                        frangi_euklid[indexVidea][snimky_k_provereni_druhy[indexVidea][i]] = euklid;
                    }
                    rozdil_x = 0;
                    rozdil_y = 0;
                    suma_rozdilu = 0;
                    euklid = 0;
                }

            }
        }
    }
    emit percentageCompleted(100);
    emit hotovo(5);
}
