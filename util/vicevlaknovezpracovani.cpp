#include "vicevlaknovezpracovani.h"
#include "dialogy/multiplevideoet.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>
#include <QVector>
#include <QThread>

using cv::Mat;
VicevlaknoveZpracovani::VicevlaknoveZpracovani(cv::VideoCapture capture, QVector<double> &entropie, QVector<double> &tennengrad, double pocetVidei, QObject *parent):QThread(parent)
{
    cap = capture;
    entropieAktual = entropie;
    tennengradAktual = tennengrad;
    pocetzpracovavanychVidei = pocetVidei;
}

void VicevlaknoveZpracovani::run()
{
    int uspech_analyzy = 0;
    if (cap.isOpened() == 0)
    {
        qDebug()<<"Video nelze pouzit pro analyzu entropie a tennengrada!";
        uspech_analyzy = 0;
    }
    else
    {
        double pocet_snimku_videa = (cap.get(CV_CAP_PROP_FRAME_COUNT));
        qDebug()<< "Analyza videa: ";
        for (double a = 0; a < pocet_snimku_videa; a++)
        {
            //qDebug()<<a;
            //double procento = (a/pocet_snimku_videa)*100.0;
            //qDebug()<<"emitting "<<a;
            emit percentageCompleted(qRound(((a/pocet_snimku_videa)*100.0)/pocetzpracovavanychVidei));

            //QCoreApplication::processEvents(); // tato funkce frčí v jiném vlákně - mohu sledovat
            //percentageComplete(procento);
            // vytížení procesoru v reálném čase

            cv::Mat snimek;
            double hodnota_entropie;
            cv::Scalar hodnota_tennengrad;
            cap.set(CV_CAP_PROP_POS_FRAMES,(a));
            if (!cap.read(snimek))
                    continue;
            else
            {
                vypocet_entropie(snimek,hodnota_entropie,hodnota_tennengrad); /// výpočty proběhnou v pořádku
                double pom = hodnota_tennengrad[0];
                //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
                entropieAktual[int(a)] = (hodnota_entropie);
                tennengradAktual[int(a)] = (pom);
                snimek.release();
            }
        }
        uspech_analyzy = 1;
    }
    emit hotovo();
}

void VicevlaknoveZpracovani::vypocet_entropie(cv::Mat &zkoumany_snimek, double &entropie, cv::Scalar &tennengrad)
{
    Mat filtrovany;//,filtrovany32f;

    medianBlur(zkoumany_snimek,filtrovany,5);
    Mat Sobelx,Sobely;
    Mat abs_grad_x, abs_grad_y,grad,suma,sum_abs_x,sum_abs_y;

    Sobel(filtrovany, Sobelx, CV_32FC1, 1, 0);
    Sobel(filtrovany,Sobely,CV_32FC1,0,1);
    convertScaleAbs(Sobelx, abs_grad_x);
    convertScaleAbs(Sobely, abs_grad_y);
    abs_grad_x.convertTo(abs_grad_x, CV_32FC1);
    abs_grad_y.convertTo(abs_grad_y, CV_32FC1);
    sum_abs_x = abs_grad_x.mul(abs_grad_x);
    sum_abs_y = abs_grad_y.mul(abs_grad_y);

    cv::add(sum_abs_x,sum_abs_y,suma);
    tennengrad = mean(suma);
    cv::sqrt(suma,grad);
    kontrola_typu_snimku_8C1(grad);
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };
    Mat hist;
    cv::calcHist(&grad, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    grad.release();
    sum_abs_x.release();
    sum_abs_y.release();
    abs_grad_x.release();
    abs_grad_y.release();
    Sobelx.release();
    Sobely.release();
    double f,p,e = 0.0;
    f = frekvence_binu(hist,histSize);
    for (int i=0; i<histSize; i++)
    {
        p=double(abs(hist.at<float>(i)))/f;
        if (p>0)
            e+=-p*log2(p);
    }
    entropie = e;
    f=0;e=0;p=0;
    hist.release();
}
double VicevlaknoveZpracovani::frekvence_binu(cv::Mat &histogram, int &velikost_histogramu)
{
    double frekvence = 0.0;
    for( int i = 0; i < velikost_histogramu; i++ )
    {
        double Hc = double(abs(histogram.at<float>(i)));
        frekvence += Hc;
    }
    return frekvence;
}

QVector<double> VicevlaknoveZpracovani::vypocitanaEntropie()
{
    return entropieAktual;
}

QVector<double> VicevlaknoveZpracovani::vypocitanyTennengrad()
{
    return tennengradAktual;
}
