#include "prace_s_vektory.h"
#include "licovani/fazova_korelace_funkce.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "licovani/multiPOC_Ai1.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <QVector>
#include <math.h>
#include <string>
#include <algorithm>
#include <numeric>
#include <random>
#include <iterator>
#include <QDebug>

double median_vektoru_cisel(QVector<double> vektor_hodnot)
{
    int size = vektor_hodnot.length();

    if (size == 0)
    {
        return 0;
    }
    else if (size == 1)
    {
        return vektor_hodnot[0];
    }
    else
    {
        std::sort(vektor_hodnot.begin(), vektor_hodnot.end());
        if (size % 2 == 0)
        {
            return (vektor_hodnot[size / 2 - 1] + vektor_hodnot[size / 2]) / 2;
        }
        else
        {
            return vektor_hodnot[size / 2];
        }
    }
}

double kontrola_maxima(QVector<double> &vektor_hodnot)
{
    QVector<double>::iterator result;
    result = std::max_element(vektor_hodnot.begin(), vektor_hodnot.end());
    int pom = std::distance(vektor_hodnot.begin(), result);

    double maximum_vektoru = vektor_hodnot[pom];
    double pom2 = maximum_vektoru;
    int kontrolni_mnozstvi = 0;
    while (kontrolni_mnozstvi < 20)
    {
        kontrolni_mnozstvi = 0;
        pom2 -= 0.001;
        for (int i = 0; i < vektor_hodnot.size(); i++)
        {
            if(vektor_hodnot[i] > pom2)
            {
                kontrolni_mnozstvi+=1;
            }
        }       
    }
    return pom2;
}

QVector<int> spojeni_vektoru(QVector<int> &vektor1, QVector<int> &vektor2)
{
    //std::ostream_iterator<double> out_it (std::cout," ");
    int celkovaVelikost = vektor1.length()+vektor2.length();
    QVector<int> sjednoceny_vektor(celkovaVelikost,0.0);

    for (int a = 0; a < 2; a++)
    {
        if (a == 0)
        {
            for (int b = 0; b < vektor1.length(); b++)
            {
                sjednoceny_vektor[b] = int(vektor1[b]);
            }
        }
        if (a == 1)
        {
            int c=0;
            for (int b = vektor1.length(); b < sjednoceny_vektor.length(); b++)
            {
                sjednoceny_vektor[b] = int(vektor2[c]);
                c+=1;
            }
        }
    }
    /*sjednoceny_vektor.insert(sjednoceny_vektor.end(),*vektor1.begin(),*vektor1.end());
    sjednoceny_vektor.insert(sjednoceny_vektor.end(),*vektor2.begin(),*vektor2.end());*/
    std::sort( sjednoceny_vektor.begin(), sjednoceny_vektor.end() );
    auto last = std::unique(sjednoceny_vektor.begin(), sjednoceny_vektor.end());
    sjednoceny_vektor.erase(last, sjednoceny_vektor.end());
    //std::copy ( sjednoceny_vektor.begin(), sjednoceny_vektor.end(), out_it );
    //cout<<endl;
    return sjednoceny_vektor;
}

void okna_vektoru(QVector<double>& vektor_hodnot, QVector<double>& okna, double &zbytek_do_konce)
{

    int velikost_vektoru = vektor_hodnot.size();
    while (velikost_vektoru%9 != 0)
    {
        velikost_vektoru -= 1;
    }
    zbytek_do_konce = vektor_hodnot.size() - velikost_vektoru;
    double krok = velikost_vektoru/9;
    double kolikrat = velikost_vektoru/krok;
    for (double i = 0; i < kolikrat; i++)
    {
        double pom = (i+1)*krok;
        okna.push_back(pom);
        //okna[i] = pom;
    }
}

QVector<double> mediany_vektoru(QVector<double>& vektor_hodnot,
                                    QVector<double>& vektor_oken,
                                    double zbytek_do_konce)
{
    //std::ostream_iterator<double> out_it (std::cout," ");
    double krok = vektor_oken[0];
    QVector<double> mediany_oken_vektoru(vektor_oken.size(),0.0);
    double pocitadlo = 1.0;
    for (int i = 0; i < vektor_oken.size(); i++)
    {
        if (i == 0)
        {

            //QVector<double> vekpom(*vektor_hodnot.begin(),*(vektor_hodnot.begin()+krok));
            QVector<double> vekpom;vekpom = vektor_hodnot.mid(0,int(krok));
            //std::copy ( vekpom.begin(), vekpom.end(), out_it );
            //std::cout<<std::endl;
            double median_okna = median_vektoru_cisel(vekpom);
            mediany_oken_vektoru[i] = median_okna;
        }
        else
        {
            QVector<double> vekpom;//(vektor_hodnot.begin()+(pocitadlo*krok),vektor_hodnot.begin()+((pocitadlo+1)*krok+1));
            vekpom = vektor_hodnot.mid(int(0.0+(pocitadlo*krok)),int(0.0+((pocitadlo+1)*krok+1)));
            //std::copy ( vekpom.begin(), vekpom.end(), out_it );
            //std::cout<<std::endl;
            double median_okna = median_vektoru_cisel(vekpom);
            mediany_oken_vektoru[i] = median_okna;
            pocitadlo+=1;
        }
    }
    if (zbytek_do_konce != 0.0 && zbytek_do_konce >= 10.0)
    {
        QVector<double> vekpom;
        vekpom = vektor_hodnot.mid(int(vektor_hodnot.length()-zbytek_do_konce+1),vektor_hodnot.length());
        double median_okna = median_vektoru_cisel(vekpom);
        mediany_oken_vektoru.push_back(median_okna);
    }
    return mediany_oken_vektoru;
}

void analyza_prubehu_funkce(QVector<double>& vektor_hodnot,
                            QVector<double>& vektor_medianu,
                            QVector<double>& vektor_oken,
                            double& prepocitane_maximum,
                            QVector<double>& prahy,
                            double& tolerance,
                            int& dmin,
                            double& zbytek_do_konce,
                            QVector<int> &spatne_snimky,
                            QVector<double>& pro_provereni)
{
    if ((vektor_hodnot[0] < vektor_medianu[0]) || (vektor_hodnot[0] >= (prepocitane_maximum+prahy[1])))
    {
        spatne_snimky.push_back(0);
    }
    for (int i = 0; i <= vektor_oken.size()-1; i++)
    {
        int od_do[2] = {0};
        if (i == 0)
        {
            od_do[0] = 1;
            od_do[1] = int(vektor_oken[i]);
        }
        else if (i == (vektor_oken.size()-1))
        {
            od_do[0] = int(vektor_oken[i-1]);
            od_do[1] = int(vektor_oken[i]-2.0);
        }
        else
        {
            od_do[0] = int(vektor_oken[i-1]);
            od_do[1] = int(vektor_oken[i]);
        }
        //cout << od_do[0] <<" "<<od_do[1]<<" ";
        double aktualni_median = vektor_medianu[i];
        //cout<<"Aktualni median ke srovnani: "<<aktualni_median<<endl;
        for (int j = od_do[0]; j < od_do[1]; j++)
        {
            if (vektor_hodnot[j] < (aktualni_median-prahy[0]))
            {
                if ((vektor_hodnot[j-1])>=vektor_hodnot[j] || (vektor_hodnot[j]<=vektor_hodnot[j+1]))
                {
                    if (spatne_snimky.empty() == 1)
                    {
                        spatne_snimky.push_back(j);
                    }
                    else
                    {
                        if (std::abs(j-spatne_snimky.back())>=dmin)
                        {
                            spatne_snimky.push_back(j);
                        }
                    }
                }
            }
            if (vektor_hodnot[j] >= prepocitane_maximum+prahy[1])
            {
                spatne_snimky.push_back(j);
            }
            if (prahy.size() == 2)
            {
                if ((vektor_hodnot[j] > (aktualni_median-prahy[0]+tolerance))
                        && (vektor_hodnot[j] < prepocitane_maximum+prahy[1]))
                {
                    pro_provereni.push_back(j);
                }
            }
        }

    }
    if (zbytek_do_konce == 0.0)
    {
        if ((vektor_hodnot.back() < vektor_medianu.back()) || (vektor_hodnot.back() >= prepocitane_maximum + prahy[1]))
        {
            spatne_snimky.push_back(vektor_hodnot.size()-1);
        }
    }
}

int nalezeni_referencniho_snimku(double& prepocitane_maximum, QVector<double>& pro_provereni,
                                 QVector<double>& vektor_hodnot)
{
    double rozdil = 1000.0;
    int referencni_snimek = 0;
    for (int i = 0; i < pro_provereni.size(); i++)
    {
        if (std::abs(prepocitane_maximum - vektor_hodnot[int(pro_provereni[i])]) < rozdil)
        {
            referencni_snimek = int(pro_provereni[i]);
            rozdil = prepocitane_maximum - vektor_hodnot[int(pro_provereni[i])];
        }
    }
    return referencni_snimek;
}

/*void analyza_FWHM(cv::VideoCapture& capture,
                    int referencni_snimek_cislo,
                    int pocet_snimku_videa,
                    bool zmena_meritka,
                    double& vypocteneR,
                    double& vypocteneFWHM,
                    cv::Rect& vyrez_oblasti_standardni,
                    cv::Rect& vyrez_oblasti_navic,
                    QVector<double>& spatne_snimky_komplet)
{
    //std::ostream_iterator<double> out_it (std::cout," ");
    int velikost_spojeneho_vektoru = spatne_snimky_komplet.length();
    QVector<double> snimky_pro_sigma((pocet_snimku_videa-velikost_spojeneho_vektoru-10),0);
    QVector<double> cisla_pro_generator(pocet_snimku_videa,0);
    //std::iota(cisla_pro_generator.begin(),cisla_pro_generator.end(),0);
    std::generate(cisla_pro_generator.begin(), cisla_pro_generator.end(), [n = 0] () mutable { return n++; });

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, pocet_snimku_videa-1);
    QVector<double>::iterator it;
    for (int i = 0; i < snimky_pro_sigma.length(); i++)
    {
        int kontrola_ulozeni = 0;
        while (kontrola_ulozeni == 0)
        {
            int vygenerovane_cislo = distr(eng);
            while (cisla_pro_generator[vygenerovane_cislo] == 0.0)
            {
                vygenerovane_cislo = distr(eng);
            }
            it = std::find(spatne_snimky_komplet.begin(), spatne_snimky_komplet.end(), cisla_pro_generator[vygenerovane_cislo]);
            if (it != spatne_snimky_komplet.end())
                {kontrola_ulozeni = 0;}
            else
            {
                snimky_pro_sigma[i] = vygenerovane_cislo;
                cisla_pro_generator[vygenerovane_cislo] = 0;
                kontrola_ulozeni = 1;
            }
        }
    }
    //std::copy ( snimky_pro_sigma.begin(), snimky_pro_sigma.end(), out_it );
    //cout<<endl;
    if (capture.isOpened() == 0)
    {
        qWarning()<<"Video nelze pouzit pro analyzu entropie a tennengrada!";
    }
    cv::Mat referencni_snimek_temp,referencni_snimek,referencni_snimek32f,referencni_vyrez;
    capture.set(CV_CAP_PROP_POS_FRAMES,referencni_snimek_cislo);
    capture.read(referencni_snimek_temp);
    int rows = 0;
    int cols = 0;
    if (zmena_meritka == true)
    {
        referencni_snimek_temp(vyrez_oblasti_navic).copyTo(referencni_snimek);
        rows = referencni_snimek.rows;
        cols = referencni_snimek.cols;
        referencni_snimek(vyrez_oblasti_standardni).copyTo(referencni_vyrez);
        referencni_snimek_temp.release();
    }
    else
    {
        referencni_snimek_temp.copyTo(referencni_snimek);
        rows = referencni_snimek.rows;
        cols = referencni_snimek.cols;
        referencni_snimek(vyrez_oblasti_standardni).copyTo(referencni_vyrez);
        referencni_snimek_temp.release();
    }
    QVector<double> zaznamenane_FWHM(snimky_pro_sigma.length(),0.0);
    QVector<double> zaznamenane_R(snimky_pro_sigma.length(),0.0);
    kontrola_typu_snimku_32C1(referencni_snimek);
    //referencni_snimek.copyTo(referencni_snimek32f);
    //kontrola_typu_snimku_32(referencni_snimek32f);
    //cout << snimky_pro_sigma.size()<<" "<<zaznamenane_FWHM.size()<<endl;
    qDebug()<< "Analyza snimku pro urceni prumerneho korelacniho koeficientu a hodnoty FWHM";
    for (int j = 0; j < snimky_pro_sigma.length(); j++)
    {
        cv::Mat posunuty_temp,posunuty,posunuty_vyrez;
        double cisloSnimku = snimky_pro_sigma[j];
        capture.set(CV_CAP_PROP_POS_FRAMES,cisloSnimku);
        if (capture.read(posunuty_temp) != 1)
        {
            qWarning()<<"Snimek "<<j<<" nelze slicovat!";
            continue;
        }
        else
        {
            if (zmena_meritka == true)
            {
                posunuty_temp(vyrez_oblasti_navic).copyTo(posunuty);
                posunuty(vyrez_oblasti_standardni).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
                posunuty(vyrez_oblasti_standardni).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            //int procento = int((j*100.0)/snimky_pro_sigma.size());
            //cout << "\r" << procento << "%";
            cv::Point3d pt(0,0,0);
            kontrola_typu_snimku_32C1(posunuty);
            cv::Mat slicovany,slicovany_vyrez;
            double celkovyUhelRotace;
            int iter = 2;double obl = 10;double u = 0.1;
            if (zmena_meritka == true)
            {
                pt = fk_translace_hann(referencni_snimek,posunuty);
                if (std::abs(pt.x)>=290 || std::abs(pt.y)>=290)
                {
                    pt = fk_translace(referencni_snimek,posunuty);
                }
            }
            if (zmena_meritka == false)
            {
                kompletni_slicovani(capture,
                                    referencni_snimek,
                                    cisloSnimku,
                                    iter,obl,u,
                                    vyrez_oblasti_navic,
                                    vyrez_oblasti_standardni,
                                    zmena_meritka,slicovany,pt,
                                    celkovyUhelRotace);
            }            
            slicovany = translace_snimku(posunuty,pt,rows,cols);
            //cv::imshow("slicovany",slicovany);
            posunuty.release();
            slicovany(vyrez_oblasti_standardni).copyTo(slicovany_vyrez);
            double zSouradnice = pt.z;
            double sigma_gauss = 0.0;
            sigma_gauss = 1/(std::sqrt(2*CV_PI)*zSouradnice);
            double FWHM = 0.0;
            FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            zaznamenane_FWHM[j] = FWHM;
            double KK = vypocet_KK(referencni_snimek,slicovany,vyrez_oblasti_standardni);
            qDebug()<<"Snimek: "<<cisloSnimku<<"R: "<<KK<<" "<<"FWHM: "<<FWHM<<" "<<pt.x<<" "<<pt.y;
            slicovany.release();
            slicovany_vyrez.release();
            zaznamenane_R[j] = KK;
        }

    }
    //cout << endl;
    //std::copy ( zaznamenane_R.begin(), zaznamenane_R.end(), out_it );
    //cout<<endl;
    vypocteneFWHM = median_vektoru_cisel(zaznamenane_FWHM);
    vypocteneR = median_vektoru_cisel(zaznamenane_R);
}*/

void kontrola_celistvosti(QVector<int> &spatne_snimky)
{
    QVector<int> vektor_rozdilu(spatne_snimky.size(),0);
    //std::ostream_iterator<double> out_it (std::cout," ");
    for (int j = 0; j < spatne_snimky.size()-1; j++)
    {
        vektor_rozdilu[j] = spatne_snimky[j+1] - spatne_snimky[j];
    }
    //std::copy ( vektor_rozdilu.begin(), vektor_rozdilu.end(), out_it );
    //cout<<endl;
    int b = 0;
    while (b < (vektor_rozdilu.size()-1))
    {
        if (vektor_rozdilu[b]==2.0)
        {
            spatne_snimky.insert(spatne_snimky.begin()+b+1,1,spatne_snimky[b]+1);
            vektor_rozdilu.insert(vektor_rozdilu.begin()+b+1,1,1);
            b+=2;
            //std::copy ( vektor_rozdilu.begin(), vektor_rozdilu.end(), out_it );
            //cout<<endl;
            //std::copy ( spatne_snimky.begin(), spatne_snimky.end(), out_it );
            //cout<<endl;
        }
        else{b+=1;}
    }
}

int findReferenceFrame(QVector<int> vectorEvaluation)
{
    int foundReference = -1;
    for (int frame = 0; frame < vectorEvaluation.length(); frame++)
    {
        if (vectorEvaluation[frame] == 2){
            foundReference = frame;
            break;
        }
    }
    return foundReference;
}

QVector<QVector<int> > divideIntoPeaces(int totalLength, int threadCount)
{
    QVector<QVector<int>> output;
    if (threadCount == 1){
        output[0].push_back(-1);
        output[1].push_back(-1);
    }
    else if (threadCount > 1){
        if (totalLength%threadCount == 0){
            int pom = 1;
            int smallestPart = (totalLength/threadCount)-1;
            output[0].push_back(0);
            output[1].push_back(smallestPart);
            while (pom < threadCount){
                output[0].push_back(smallestPart*pom+pom);
                output[1].push_back(smallestPart*pom+pom+smallestPart);
                pom+=1;
            }
        }
        else{
            int rest = 1;
            totalLength-=1;
            while(totalLength%threadCount !=0){
                rest+=1;
                totalLength-=1;
            }
            int pom = 1;
            int smallestPart = (totalLength/threadCount)-1;
            output[0].push_back(0);
            output[1].push_back(smallestPart);
            while (pom < threadCount){
                output[0].push_back(smallestPart*pom+pom);
                output[1].push_back(smallestPart*pom+pom+smallestPart);
                pom+=1;
            }
            output[1][pom-1]+=rest;
        }
    }
    return output;
}
