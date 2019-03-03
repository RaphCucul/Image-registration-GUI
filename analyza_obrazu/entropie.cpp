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
bool calculateParametersET(cv::Mat &specificImage, double &entropy, cv::Scalar &tennengrad)
{
    try {
        Mat filtered;//,filtrovany32f;

        medianBlur(specificImage,filtered,5);
        Mat Sobelx,Sobely;
        Mat abs_grad_x, abs_grad_y,grad,suma,sum_abs_x,sum_abs_y;

        Sobel(filtered, Sobelx, CV_32FC1, 1, 0);
        Sobel(filtered,Sobely,CV_32FC1,0,1);
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
        return true;
    } catch (std::exception e) {
        return false;
    }

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
                if (calculateParametersET(image,entropyValue,tennengradValue)){ /// výpočty proběhnou v pořádku
                    double pom = tennengradValue[0];
                    entropy[a] = (entropyValue);
                    tennengrad[a] = (pom);
                    image.release();
                }
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
