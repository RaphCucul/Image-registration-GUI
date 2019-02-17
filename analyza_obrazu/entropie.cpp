#include "entropie.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/pouzij_frangiho.h"
//#include "util/vicevlaknovezpracovani.h"
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

double binFrequency(cv::Mat &inputImage, int &histogramSize)
{
    double frequency = 0.0;
    for( int i = 0; i < histogramSize; i++ )
    {
        double Hc = double(abs(inputImage.at<float>(i)));
        frequency += Hc;
    }
    return frequency;
}
void calculateParametersET(cv::Mat &specificImage, double &entropy, cv::Scalar &tennengrad)
{
    Mat filtered;//,filtrovany32f;

    medianBlur(specificImage,filtered,5);
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

    Sobel(filtered, Sobelx, CV_32FC1, 1, 0);
    Sobel(filtered,Sobely,CV_32FC1,0,1);
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
    f = binFrequency(hist,histSize);
    for (int i=0; i<histSize; i++)
    {
        p=double(abs(hist.at<float>(i)))/f;
        if (p>0)
            e+=-p*log2(p);
    }
    entropy = e;
    f=0;e=0;p=0;
    hist.release();

}
bool entropy_tennengrad_video(cv::VideoCapture& capture,
                              QVector<double> &entropy,
                              QVector<double> &tennengrad, QProgressBar *progbar)
{
    int uspech_analyzy = false;
    int percentage = 0;

    if (capture.isOpened() == 0)
    {
        uspech_analyzy = false;
    }
    else
    {
        int pocet_snimku_videa = int(capture.get(CV_CAP_PROP_FRAME_COUNT));
        qDebug()<< "Analyza videa: ";
        for (int a = 0; a < pocet_snimku_videa; a++)
        {
            //qDebug()<<a;
            if (a == 0)
                percentage = 0;
            else if (a == (pocet_snimku_videa-1))
                percentage = 100;
            else
                percentage = ((a/pocet_snimku_videa)*100);

            QCoreApplication::processEvents(); // tato funkce frčí v jiném vlákně - mohu sledovat
            progbar->setValue(percentage);
            //percentageComplete(procento);
            // vytížení procesoru v reálném čase

            cv::Mat image;
            double entropyValue = 0;
            cv::Scalar tennengradValue;
            capture.set(CAP_PROP_POS_FRAMES,double(a));
            if (!capture.read(image)){

            }
            else
            {
                calculateParametersET(image,entropyValue,tennengradValue); /// výpočty proběhnou v pořádku
                double pom = tennengradValue[0];
                //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
                entropy[a] = (entropyValue);
                tennengrad[a] = (pom);
                image.release();
            }
        }
        uspech_analyzy = true;
    }
    return uspech_analyzy;
}

void standardizedData(QVector<double>& dataStandardized, QVector<double> &dataOriginal, double minimum, double maximum)
{
    for (int a = 0; a < dataStandardized.length(); a++)
    {
        double hodnotaStandardizovana = (dataOriginal[a]-minimum)/(maximum-minimum);
        dataStandardized[a] = (hodnotaStandardizovana);
    }
}

void aktualizaceProgressBaru(QProgressBar* pb, int procento)
{
    pb->setValue(procento);
}
