#include "prace_s_vektory.h"
#include "licovani/fazova_korelace_funkce.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "util/souborove_operace.h"
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
#include <exception>

double median_VectorDouble(QVector<double> inputValues)
{
    int size = inputValues.length();

    if (size == 0)
    {
        return 0;
    }
    else if (size == 1)
    {
        return inputValues[0];
    }
    else
    {
        std::sort(inputValues.begin(), inputValues.end());
        if (size % 2 == 0)
        {
            return (inputValues[size / 2 - 1] + inputValues[size / 2]) / 2;
        }
        else
        {
            return inputValues[size / 2];
        }
    }
}

double checkMaximum(QVector<double> &inputValues)
{
    QVector<double>::iterator result;
    result = std::max_element(inputValues.begin(), inputValues.end());
    int pom = std::distance(inputValues.begin(), result);

    double maximum = inputValues[pom];
    double pom2 = maximum;
    int controlAmount = 0;
    while (controlAmount < 20)
    {
        controlAmount = 0;
        pom2 -= 0.001;
        for (int i = 0; i < inputValues.size(); i++)
        {
            if(inputValues[i] > pom2)
            {
                controlAmount+=1;
            }
        }       
    }
    return pom2;
}

QVector<int> mergeVectors(QVector<int> &input1, QVector<int> &input2)
{
    //std::ostream_iterator<double> out_it (std::cout," ");
    int totalSize = input1.length()+input2.length();
    QVector<int> mergedVector(totalSize,0.0);

    for (int a = 0; a < 2; a++)
    {
        if (a == 0)
        {
            for (int b = 0; b < input1.length(); b++)
            {
                mergedVector[b] = int(input1[b]);
            }
        }
        if (a == 1)
        {
            int c=0;
            for (int b = input1.length(); b < mergedVector.length(); b++)
            {
                mergedVector[b] = int(input2[c]);
                c+=1;
            }
        }
    }
    /*mergedVector.insert(mergedVector.end(),*vektor1.begin(),*vektor1.end());
    mergedVector.insert(mergedVector.end(),*vektor2.begin(),*vektor2.end());*/
    std::sort( mergedVector.begin(), mergedVector.end() );
    auto last = std::unique(mergedVector.begin(), mergedVector.end());
    mergedVector.erase(last, mergedVector.end());
    //std::copy ( mergedVector.begin(), mergedVector.end(), out_it );
    //cout<<endl;
    return mergedVector;
}

void vectorWindows(QVector<double>& inputValues, QVector<double>& windows, double &restToEnd)
{

    int vectorSize = inputValues.size();
    while (vectorSize%9 != 0)
    {
        vectorSize -= 1;
    }
    restToEnd = inputValues.size() - vectorSize;
    double step = vectorSize/9;
    double howManyTimes = vectorSize/step;
    for (double i = 0; i < howManyTimes; i++)
    {
        double pom = (i+1)*step;
        windows.push_back(pom);
        //okna[i] = pom;
    }
}

QVector<double> mediansOfVector(QVector<double>& inputValues,
                                    QVector<double>& inputWindows,
                                    double restToEnd)
{
    //std::ostream_iterator<double> out_it (std::cout," ");
    double step = inputWindows[0];
    QVector<double> mediansVectorWindows(inputWindows.size(),0.0);
    double counter = 1.0;
    for (int i = 0; i < inputWindows.size(); i++)
    {
        if (i == 0)
        {
            //QVector<double> vekpom(*vektor_hodnot.begin(),*(vektor_hodnot.begin()+krok));
            QVector<double> vekpom;vekpom = inputValues.mid(0,int(step));
            //std::copy ( vekpom.begin(), vekpom.end(), out_it );
            //std::cout<<std::endl;
            double windowMedian = median_VectorDouble(vekpom);
            mediansVectorWindows[i] = windowMedian;
        }
        else
        {
            QVector<double> vekpom;//(vektor_hodnot.begin()+(pocitadlo*krok),vektor_hodnot.begin()+((pocitadlo+1)*krok+1));
            vekpom = inputValues.mid(int(0.0+(counter*step)),int(0.0+((counter+1)*step+1)));
            //std::copy ( vekpom.begin(), vekpom.end(), out_it );
            //std::cout<<std::endl;
            double windowMedian = median_VectorDouble(vekpom);
            mediansVectorWindows[i] = windowMedian;
            counter+=1;
        }
    }
    if (restToEnd != 0.0 && restToEnd >= 10.0)
    {
        QVector<double> vekpom;
        vekpom = inputValues.mid(int(inputValues.length()-restToEnd+1),inputValues.length());
        double windowMedian = median_VectorDouble(vekpom);
        mediansVectorWindows.push_back(windowMedian);
    }
    return mediansVectorWindows;
}

bool analysisFunctionValues(QVector<double>& inputValues,
                            QVector<double>& medianVector,
                            QVector<double>& windowsVector,
                            double& recalculatedMaximum,
                            QVector<double>& thresholds,
                            double& tolerance,
                            int& dmin,
                            double& restToEnd,
                            QVector<int> &badFrames,
                            QVector<double>& forEvaluation)
{
    try {
        if ((inputValues[0] < medianVector[0]) || (inputValues[0] >= (recalculatedMaximum+thresholds[1])))
        {
            badFrames.push_back(0);
        }
        for (int i = 0; i <= windowsVector.size()-1; i++)
        {
            int od_do[2] = {0};
            if (i == 0)
            {
                od_do[0] = 1;
                od_do[1] = int(windowsVector[i]);
            }
            else if (i == (windowsVector.size()-1))
            {
                od_do[0] = int(windowsVector[i-1]);
                od_do[1] = int(windowsVector[i]-2.0);
            }
            else
            {
                od_do[0] = int(windowsVector[i-1]);
                od_do[1] = int(windowsVector[i]);
            }
            //cout << od_do[0] <<" "<<od_do[1]<<" ";
            double actualMedian = medianVector[i];
            //cout<<"Aktualni median ke srovnani: "<<actualMedian<<endl;
            for (int j = od_do[0]; j < od_do[1]; j++)
            {
                if (inputValues[j] < (actualMedian-thresholds[0]))
                {
                    if ((inputValues[j-1])>=inputValues[j] || (inputValues[j]<=inputValues[j+1]))
                    {
                        if (badFrames.empty() == 1)
                        {
                            badFrames.push_back(j);
                        }
                        else
                        {
                            if (std::abs(j-badFrames.back())>=dmin)
                            {
                                badFrames.push_back(j);
                            }
                        }
                    }
                }
                if (inputValues[j] >= recalculatedMaximum+thresholds[1])
                {
                    badFrames.push_back(j);
                }
                if (thresholds.size() == 2)
                {
                    if ((inputValues[j] > (actualMedian-thresholds[0]+tolerance))
                            && (inputValues[j] < recalculatedMaximum+thresholds[1]))
                    {
                        forEvaluation.push_back(j);
                    }
                }
            }

        }
        if (restToEnd == 0.0)
        {
            if ((inputValues.back() < medianVector.back()) || (inputValues.back() >= recalculatedMaximum + thresholds[1]))
            {
                badFrames.push_back(inputValues.size()-1);
            }
        }
        return true;
    } catch (std::exception e) {
        return false;
    }
}

int findReferencialNumber(double& prepocitane_maximum, QVector<double>& forEvaluation,
                                 QVector<double>& vektor_hodnot)
{
    double difference = 1000.0;
    int referencialNumber = 0;
    for (int i = 0; i < forEvaluation.size(); i++)
    {
        if (std::abs(prepocitane_maximum - vektor_hodnot[int(forEvaluation[i])]) < difference)
        {
            referencialNumber = int(forEvaluation[i]);
            difference = prepocitane_maximum - vektor_hodnot[int(forEvaluation[i])];
        }
    }
    return referencialNumber;
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
                completeRegistration(capture,
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
    vypocteneFWHM = median_VectorDouble(zaznamenane_FWHM);
    vypocteneR = median_VectorDouble(zaznamenane_R);
}*/

void integrityCheck(QVector<int> &badFrames)
{
    QVector<int> vektor_rozdilu(badFrames.size(),0);
    //std::ostream_iterator<double> out_it (std::cout," ");
    for (int j = 0; j < badFrames.size()-1; j++)
    {
        vektor_rozdilu[j] = badFrames[j+1] - badFrames[j];
    }
    //std::copy ( vektor_rozdilu.begin(), vektor_rozdilu.end(), out_it );
    //cout<<endl;
    int b = 0;
    while (b < (vektor_rozdilu.size()-1))
    {
        if (vektor_rozdilu[b]==2.0)
        {
            badFrames.insert(badFrames.begin()+b+1,1,badFrames[b]+1);
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
    QVector<int> lowerBoundary;
    QVector<int> upperBoundary;
    QVector<QVector<int>> output;
    if (threadCount == 1){
        lowerBoundary.push_back(0);
        upperBoundary.push_back(totalLength-1);

    }
    else if (threadCount > 1){
        if (totalLength%threadCount == 0){
            int pom = 1;
            int smallestPart = (totalLength/threadCount)-1;
            lowerBoundary.push_back(0);
            upperBoundary.push_back(smallestPart);
            while (pom < threadCount){
                lowerBoundary.push_back(smallestPart*pom+pom);
                upperBoundary.push_back(smallestPart*pom+pom+smallestPart);
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
            lowerBoundary.push_back(0);
            upperBoundary.push_back(smallestPart);
            while (pom < threadCount){
                lowerBoundary.push_back(smallestPart*pom+pom);
                upperBoundary.push_back(smallestPart*pom+pom+smallestPart);
                pom+=1;
            }
            upperBoundary[pom-1]+=rest;
        }
    }
    output.append(lowerBoundary);
    output.append(upperBoundary);
    return output;
}

void analyseAndSaveFirst(QString analysedFolder,QVector<QString> &whereToSave){
    QString folder,filename,suffix;
    QStringList filesFound;
    int foundCount = 0;
    analyseFileNames(analysedFolder,filesFound,foundCount,"avi");
    if (foundCount != 0){
        QString fullName = analysedFolder+"/"+filesFound.at(0);
        processFilePath(fullName,folder,filename,suffix);
        if (whereToSave.length() == 0){
            whereToSave.push_back(folder);
            whereToSave.push_back(filename);
            whereToSave.push_back(suffix);
        }
        else{
            whereToSave.clear();
            whereToSave.push_back(folder);
            whereToSave.push_back(filename);
            whereToSave.push_back(suffix);
        }
    }
}

int vectorSum(QVector<int> input)
{
    int output=0;
    for (int vectorElement = 0; vectorElement < input.length(); vectorElement++)
        output += input[vectorElement];

    return output;
}

double vectorSum(QVector<double> input)
{
    int output=0;
    for (int vectorElement = 0; vectorElement < input.length(); vectorElement++)
        output += input[vectorElement];

    return output;
}
