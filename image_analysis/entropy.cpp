#include "image_analysis/entropy.h"
#include "util/files_folders_operations.h"
#include "image_analysis/frangi_utilization.h"
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
bool calculateParametersET(cv::Mat &i_specificImage, double &i_entropy, cv::Scalar &i_tennengrad)
{
    try {
        Mat filtered;

        medianBlur(i_specificImage,filtered,5);
        Mat Sobelx,Sobely;
        Mat abs_grad_x, abs_grad_y,grad,totalSum,sum_abs_x,sum_abs_y;

        Sobel(filtered, Sobelx, CV_32FC1, 1, 0);
        Sobel(filtered,Sobely,CV_32FC1,0,1);
        convertScaleAbs(Sobelx, abs_grad_x);
        convertScaleAbs(Sobely, abs_grad_y);
        abs_grad_x.convertTo(abs_grad_x, CV_32FC1);
        abs_grad_y.convertTo(abs_grad_y, CV_32FC1);
        sum_abs_x = abs_grad_x.mul(abs_grad_x);
        sum_abs_y = abs_grad_y.mul(abs_grad_y);

        cv::add(sum_abs_x,sum_abs_y,totalSum);
        i_tennengrad = mean(totalSum);
        cv::sqrt(totalSum,grad);
        transformMatTypeTo8C1(grad);
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
        i_entropy = e;
        f=0;e=0;p=0;
        hist.release();
        return true;
    } catch (std::exception e) {
        return false;
    }

}
bool entropy_tennengrad_video(cv::VideoCapture& i_capture,
                              QVector<double> &i_entropy,
                              QVector<double> &i_tennengrad)
{
    int success = false;

    if (i_capture.isOpened() == 0)
    {
        success = false;
    }
    else
    {
        int frameCount = int(i_capture.get(CV_CAP_PROP_FRAME_COUNT));
        for (int a = 0; a < frameCount; a++)
        {
            cv::Mat image;
            double entropyValue = 0.0;
            cv::Scalar tennengradValue;
            i_capture.set(CAP_PROP_POS_FRAMES,double(a));
            if (!i_capture.read(image)){
                i_entropy[a] = 0.0;
                i_tennengrad[a] = 0.0;
            }
            else
            {
                if (calculateParametersET(image,entropyValue,tennengradValue)){ /// výpočty proběhnou v pořádku
                    double pom = tennengradValue[0];
                    i_entropy[a] = (entropyValue);
                    i_tennengrad[a] = (pom);
                    image.release();
                }
            }
        }
        success = true;
    }
    return success;
}

void dataStandardization(QVector<double>& i_dataStandardized, QVector<double> &i_dataOriginal, double i_minimum, double i_maximum)
{
    for (int a = 0; a < i_dataStandardized.length(); a++)
    {
        double new_value = (i_dataOriginal[a]-i_minimum)/(i_maximum-i_minimum);
        i_dataStandardized[a] = (new_value);
    }
}
