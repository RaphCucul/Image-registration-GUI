#include "entropie.h"
#include "util/souborove_operace.h"
#include "util/pouzij_frangiho.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <math.h>
#include <string>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QObject>
using namespace cv;

double frekvence_binu(cv::Mat &histogram, int &velikost_histogramu)
{
    double frekvence = 0.0;
    for( int i = 0; i < velikost_histogramu; i++ )
    {
        double Hc = double(abs(histogram.at<float>(i)));
        frekvence += Hc;
    }
    return frekvence;
}
void vypocet_entropie(cv::Mat &zkoumany_snimek, double &entropie, cv::Scalar &tennengrad)
{
    Mat filtrovany;//,filtrovany32f;

    medianBlur(zkoumany_snimek,filtrovany,5);
    //imshow("Filtrovany",filtrovany);
    //Mat ch1, ch2, ch3;
    /*if (zkoumany_snimek.channels() == 3)
    {
        cvtColor(zkoumany_snimek,filtrovany,CV_BGR2GRAY);
        filtrovany.convertTo(filtrovany, CV_32FC1);
    }
    else
    {
        kontrola_typu_snimku_32(zkoumany_snimek);
    }*/
    //filtrovany.copyTo(filtrovany32f);
    //kontrola_typu_snimku_32(filtrovany);
    Mat Sobelx,Sobely;
    Mat abs_grad_x, abs_grad_y,grad,suma,sum_abs_x,sum_abs_y;

    Sobel(filtrovany, Sobelx, CV_32FC1, 1, 0);
    Sobel(filtrovany,Sobely,CV_32FC1,0,1);
    convertScaleAbs(Sobelx, abs_grad_x);
    convertScaleAbs(Sobely, abs_grad_y);
    abs_grad_x.convertTo(abs_grad_x, CV_32FC1);
    abs_grad_y.convertTo(abs_grad_y, CV_32FC1);
    //cv::pow(Sobelx,2,sum_abs_x);
    //cv::pow(Sobely,2,sum_abs_y);
    sum_abs_x = abs_grad_x.mul(abs_grad_x);
    sum_abs_y = abs_grad_y.mul(abs_grad_y);

    cv::add(sum_abs_x,sum_abs_y,suma);
    tennengrad = mean(suma);
    cv::sqrt(suma,grad);
    //grad.convertTo(grad, CV_8UC1);
    kontrola_typu_snimku_8C1(grad);
    //addWeighted(abs_grad_x, 0.7, abs_grad_y, 0.7, 0, grad);
    //imshow("Sobel",grad);
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };
    //bool uniform = true;
    //bool accumulate = false;
    Mat hist;
    cv::calcHist(&grad, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    grad.release();
    sum_abs_x.release();
    sum_abs_y.release();
    abs_grad_x.release();
    abs_grad_y.release();
    Sobelx.release();
    Sobely.release();
    /*for (int i=0; i<histSize; i++)
    {
        f+= abs(hist.at<float>(i));
    }*/
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
    //return e;
    /*Mat filtrovany;
    medianBlur(nacteny_snimek,filtrovany,5);
    //imshow("Filtrovany",filtrovany);
    Mat ch1, ch2, ch3;
    std::vector<Mat> channels(3);
    split(filtrovany, channels);
    ch1 = channels[0];
    filtrovany = channels[1];
    ch3 = channels[2];
    Mat Sobelx,Sobely;
    Mat abs_grad_x, abs_grad_y,grad;
    Sobel(filtrovany, Sobelx, CV_32FC1, 1, 0);
    Sobel(filtrovany,Sobely,CV_32FC1,0,1);
    convertScaleAbs(Sobelx, abs_grad_x);
    convertScaleAbs(Sobely, abs_grad_y);
    addWeighted(abs_grad_x, 0.7, abs_grad_y, 0.7, 0, grad);*/
    //imshow("Sobel",grad);
    /*int histSize = 128;
    float range[] = { 0, 128 } ;
    const float* histRange = { range };
    bool uniform = true;
    bool accumulate = false;
    Mat hist;
    cv::calcHist(&filtrovany, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    double f=0,p=0,e=0;
    for (int i=0; i<histSize; i++)
    {
        f+= (hist.at<double>(i));
    }
    for (int i=0; i<histSize; i++)
    {
        p=(hist.at<double>(i))/f;
        if (p>0)
            e+=-p*log2(p);
    }*/
}
int entropie_tennengrad_videa(cv::VideoCapture& capture,
                              QVector<double> &entropie,
                              QVector<double> &tennengrad,
                              QProgressBar *progbar)
{
    int uspech_analyzy;
    int procento;
    /*QTimer * timer = new QTimer();
    QObject::connect(timer, SIGNAL(timeout()), progbar, SLOT(updateProgress()));
    timer->start(100);*/
    /*if (!entropie.empty() || !tennengrad.empty())
    {
        entropie.clear();
        tennengrad.clear();
    }*/ // protože ukládání do vektorů řeším už jinak, toto není potřeba - jen to dělá binec

    if (capture.isOpened() == 0)
    {
        qDebug()<<"Video nelze pouzit pro analyzu entropie a tennengrada!";
        uspech_analyzy = 0;
    }
    else
    {
        int pocet_snimku_videa = int(capture.get(CV_CAP_PROP_FRAME_COUNT));
        qDebug()<< "Analyza videa: ";
        for (int a = 0; a < pocet_snimku_videa; a++)
        {
            //qDebug()<<a;
            if (a == 0)
                procento = 0;
            else if (a == (pocet_snimku_videa-1))
                procento = 100;
            else
                procento = ((a/pocet_snimku_videa)*100);

            QCoreApplication::processEvents(); // tato funkce frčí v jiném vlákně - mohu sledovat
            progbar->setValue(procento);            
            // vytížení procesoru v reálném čase
            /// po sem to funguje všechno normálně

            cv::Mat snimek;
            double hodnota_entropie = 0;
            cv::Scalar hodnota_tennengrad;
            capture.set(CAP_PROP_POS_FRAMES,double(a));
            capture.read(snimek);
            /// tato pasáž také funguje

            vypocet_entropie(snimek,hodnota_entropie,hodnota_tennengrad); /// výpočty proběhnou v pořádku
            double pom = hodnota_tennengrad[0];
            //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
            entropie[a] = (hodnota_entropie);
            tennengrad[a] = (pom);
            snimek.release();
        }
        /*for (int i = 0; i < pocet_snimku_videa; i++)
        {
            qDebug()<<i;

            if (i == 0)
                procento = 0;
            else if (i == (pocet_snimku_videa-1))
                procento = 100;
            else
                procento = ((i/pocet_snimku_videa)*100);

            progbar->setValue(procento);
            cv::Mat snimek;
            double hodnota_entropie = 0;
            cv::Scalar hodnota_tennengrad;
            capture.set(CAP_PROP_POS_FRAMES,image_count);
            if (capture.read(snimek)!= 1)
            {
                qDebug()<< "Snimek " << image_count << " videa se nepodarilo nacist pro analyzu entropie a tennengradu!";
                continue;
            }    
            vypocet_entropie(snimek,hodnota_entropie,hodnota_tennengrad);
            entropie[image_count] = hodnota_entropie;
            double pom = hodnota_tennengrad[0];
            qDebug()<<"Zpracovan snimek "<<i<<" s E: "<<hodnota_entropie<<" a T: "<<pom;
            tennengrad[image_count] = pom;
            image_count+=1;
        }*/
        //std::cout << endl;
        uspech_analyzy = 1;
        return uspech_analyzy;
    }
}

void standardizaceVektoruDat(QVector<double>& dataStandardizovana, QVector<double> &dataOriginalni, double minimum, double maximum)
{
    for (int a = 0; a < dataStandardizovana.length(); a++)
    {
        double hodnotaStandardizovana = (dataOriginalni[a]-minimum)/(maximum-minimum);
        dataStandardizovana[a] = (hodnotaStandardizovana);
    }
}

void aktualizaceProgressBaru(QProgressBar* pb, int procento)
{
    pb->setValue(procento);
}
