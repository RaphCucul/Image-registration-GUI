#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/upravy_obrazu.h"
using namespace std;
using cv::Mat;
/*float sum(vector<double> a)
{
	double s = 0;
	for (int i = 0; i < a.size(); i++)
	{
		s += a[i];
	}
	return s;
}*/

/*float mean(vector<double> a)
{
	return sum(a) / a.size();
}*/

/*float sqsum(vector<double> a)
{
	double s = 0;
	for (int i = 0; i < a.size(); i++)
	{
		s += pow(a[i], 2);
	}
	return s;
}*/

/*float stdev(vector<double> nums)
{
	double N = nums.size();
	return pow(sqsum(nums) / N - pow(sum(nums) / N, 2), 0.5);
}*/

/*vector<double> operator-(vector<double> a, double b)
{
	vector<double> retvect;
	for (double i = 0; i < a.size(); i++)
	{
		retvect.push_back(a[i] - b);
	}
	return retvect;
}*/

/*vector<double> operator*(vector<double> a, vector<double> b)
{
	vector<double> retvect;
	for (double i = 0; i < a.size() ; i++)
	{
		retvect.push_back(a[i] * b[i]);
	}
	return retvect;
}*/

/*float pearsoncoeff(vector<double> X, vector<double> Y)
{
	return sum((X - mean(X))*(Y - mean(Y))) / (X.size()*stdev(X)* stdev(Y));
}*/

/*float vypocet_korel_koef_puvodni(cv::Mat &obraz,cv::Mat &zfazovany,double cislo,double sirka_vyrezu)
{
    Mat radek_obraz,radek_zfazovany,corr;
    radek_obraz = Mat::zeros(1,sirka_vyrezu,CV_8UC1);
    radek_zfazovany = Mat::zeros(1,sirka_vyrezu,CV_8UC1);
    obraz.row(cislo).copyTo(radek_obraz);
    zfazovany.row(cislo).copyTo(radek_zfazovany);
    cv::matchTemplate(radek_obraz, radek_zfazovany, corr, cv::TM_CCOEFF_NORMED);
    return corr.at<float>(0,0);  // corr only has one pixel

}*/

double vypocet_KK(const cv::Mat &referencni, const cv::Mat &slicovany, cv::Rect vyrez_korelace)
{
    Mat referencni_vyrez,slicovany_vyrez,referencni64,slicovany64;
    referencni.copyTo(referencni64);
    slicovany.copyTo(slicovany64);
    referencni64 = kontrola_typu_snimku_64C1(referencni64);
    slicovany64 = kontrola_typu_snimku_64C1(slicovany64);
    referencni64(vyrez_korelace).copyTo(referencni_vyrez);
    slicovany64(vyrez_korelace).copyTo(slicovany_vyrez);
    //int typ_reference = referencni_vyrez.type();
    //int typ_slicovany = slicovany_vyrez.type();
    cv::Scalar prumer_obraz,prumer_posunuty;
    prumer_obraz = cv::mean(referencni_vyrez);
    prumer_posunuty = cv::mean(slicovany_vyrez);
    subtract(referencni_vyrez,prumer_obraz[0],referencni_vyrez);
    subtract(slicovany_vyrez,prumer_posunuty[0],slicovany_vyrez);
    //r = sum(sum(a.*b))/sqrt(sum(sum(a.*a))*sum(sum(b.*b)));
    cv::Scalar pom1,pom2,pom3;
    double r;
    pom1 = cv::sum(referencni_vyrez.mul(slicovany_vyrez));
    pom2 = cv::sum(referencni_vyrez.mul(referencni_vyrez));
    pom3 = cv::sum(slicovany_vyrez.mul(slicovany_vyrez));
    //cout << pom1[0]<<" "<<pom2[0]<<" "<<pom3[0]<<endl;
    r = pom1[0]/std::sqrt(pom2[0]*pom3[0]);
    return r;
}
