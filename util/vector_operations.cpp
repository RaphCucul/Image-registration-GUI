#include "util/vector_operations.h"
#include "registration/phase_correlation_function.h"
#include "image_analysis/image_processing.h"
#include "image_analysis/correlation_coefficient.h"
#include "util/files_folders_operations.h"
#include "registration/multiPOC_Ai1.h"

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

double median_VectorDouble(QVector<double> i_input)
{
    int size = i_input.length();

    if (size == 0)
    {
        return 0;
    }
    else if (size == 1)
    {
        return i_input[0];
    }
    else
    {
        std::sort(i_input.begin(), i_input.end());
        if (size % 2 == 0)
        {
            return (i_input[size / 2 - 1] + i_input[size / 2]) / 2;
        }
        else
        {
            return i_input[size / 2];
        }
    }
}

double checkMaximum(QVector<double> &i_input)
{
    QVector<double>::iterator result;
    result = std::max_element(i_input.begin(), i_input.end());
    int pom = std::distance(i_input.begin(), result);

    double maximum = i_input[pom];
    double pom2 = maximum;
    int controlAmount = 0;
    while (controlAmount < 20)
    {
        controlAmount = 0;
        pom2 -= 0.001;
        for (int i = 0; i < i_input.size(); i++)
        {
            if(i_input[i] > pom2)
            {
                controlAmount+=1;
            }
        }       
    }
    return pom2;
}

QVector<int> mergeVectors(QVector<int> &i_input1, QVector<int> &i_input2)
{
    int totalSize = i_input1.length()+i_input2.length();
    QVector<int> mergedVector(totalSize,0.0);

    for (int a = 0; a < 2; a++)
    {
        if (a == 0)
        {
            for (int b = 0; b < i_input1.length(); b++)
            {
                mergedVector[b] = int(i_input1[b]);
            }
        }
        if (a == 1)
        {
            int c=0;
            for (int b = i_input1.length(); b < mergedVector.length(); b++)
            {
                mergedVector[b] = int(i_input2[c]);
                c+=1;
            }
        }
    }
    /*mergedVector.insert(mergedVector.end(),*vektor1.begin(),*vektor1.end());
    mergedVector.insert(mergedVector.end(),*vektor2.begin(),*vektor2.end());*/
    std::sort( mergedVector.begin(), mergedVector.end() );
    auto last = std::unique(mergedVector.begin(), mergedVector.end());
    mergedVector.erase(last, mergedVector.end());
    return mergedVector;
}

void vectorWindows(QVector<double>& i_inputValues, QVector<double>& i_windows, double &i_restToEnd)
{

    int vectorSize = i_inputValues.size();
    while (vectorSize%9 != 0)
    {
        vectorSize -= 1;
    }
    i_restToEnd = i_inputValues.size() - vectorSize;
    double step = vectorSize/9;
    double howManyTimes = vectorSize/step;
    for (double i = 0; i < howManyTimes; i++)
    {
        double pom = (i+1)*step;
        i_windows.push_back(pom);
    }
}

QVector<double> mediansOfVector(QVector<double>& i_inputValues,
                                    QVector<double>& i_inputWindows,
                                    double i_restToEnd)
{
    double step = i_inputWindows[0];
    QVector<double> mediansVectorWindows(i_inputWindows.size(),0.0);
    double counter = 1.0;
    for (int i = 0; i < i_inputWindows.size(); i++)
    {
        if (i == 0)
        {
            QVector<double> vekpom;vekpom = i_inputValues.mid(0,int(step));
            double windowMedian = median_VectorDouble(vekpom);
            mediansVectorWindows[i] = windowMedian;
        }
        else
        {
            QVector<double> vekpom;
            vekpom = i_inputValues.mid(int(0.0+(counter*step)),int(0.0+((counter+1)*step+1)));
            double windowMedian = median_VectorDouble(vekpom);
            mediansVectorWindows[i] = windowMedian;
            counter+=1;
        }
    }
    if (i_restToEnd != 0.0 && i_restToEnd >= 10.0)
    {
        QVector<double> vekpom;
        vekpom = i_inputValues.mid(int(i_inputValues.length()-i_restToEnd+1),i_inputValues.length());
        double windowMedian = median_VectorDouble(vekpom);
        mediansVectorWindows.push_back(windowMedian);
    }
    return mediansVectorWindows;
}

bool analysisFunctionValues(QVector<double>& i_inputValues,
                            QVector<double>& i_medianVector,
                            QVector<double>& i_windowsVector,
                            double& i_recalculatedMaximum,
                            QVector<double>& i_thresholds,
                            double& i_tolerance,
                            int& i_dmin,
                            double& i_restToEnd,
                            QVector<int> &i_badFrames,
                            QVector<double>& i_forEvaluation,
                            bool explicitThresholds)
{
    double absoluteMinimum = 100000;
    double maximum_criterion = 0, minimum_criterion = 0;
    if (explicitThresholds) {
        maximum_criterion = i_thresholds[1];
    }
    else {
        maximum_criterion = i_recalculatedMaximum+i_thresholds[1];
    }

    try { // 0 - lower; 1 - upper
        if ((i_inputValues[0] < i_medianVector[0]) || (i_inputValues[0] >= (maximum_criterion)))
        {
            i_badFrames.push_back(0);
        }
        for (int i = 0; i <= i_windowsVector.size()-1; i++)
        {
            int from_to[2] = {0};
            if (i == 0)
            {
                from_to[0] = 1;
                from_to[1] = int(i_windowsVector[i]);
            }
            else if (i == (i_windowsVector.size()-1))
            {
                from_to[0] = int(i_windowsVector[i-1]);
                from_to[1] = int(i_windowsVector[i]-2.0);
            }
            else
            {
                from_to[0] = int(i_windowsVector[i-1]);
                from_to[1] = int(i_windowsVector[i]);
            }

            double actualMedian = i_medianVector[i];
            // trying to identify absolute minimum which will be stored in the *.dat file
            if (actualMedian-i_thresholds[0] < absoluteMinimum)
                absoluteMinimum = actualMedian-i_thresholds[0];

            for (int j = from_to[0]; j < from_to[1]; j++)
            {
                if (explicitThresholds) {
                    minimum_criterion = i_thresholds[0];
                }
                else {
                    minimum_criterion = actualMedian-i_thresholds[0];
                }

                if (i_inputValues[j] < (minimum_criterion))
                {
                    if ((i_inputValues[j-1])>=i_inputValues[j] || (i_inputValues[j]<=i_inputValues[j+1]))
                    {
                        if (i_badFrames.empty() == 1)
                        {
                            i_badFrames.push_back(j);
                        }
                        else
                        {
                            if (std::abs(j-i_badFrames.back())>=i_dmin)
                            {
                                i_badFrames.push_back(j);
                            }
                        }
                    }
                }
                if (i_inputValues[j] >= maximum_criterion)
                {
                    i_badFrames.push_back(j);
                }
                if (i_thresholds.size() == 2)
                {
                    if (!explicitThresholds)
                        minimum_criterion+=i_tolerance;

                    if ((i_inputValues[j] > minimum_criterion)
                            && (i_inputValues[j] < maximum_criterion))
                    {
                        i_forEvaluation.push_back(j);
                    }
                }
            }

        }
        if (i_restToEnd == 0.0)
        {
            if ((i_inputValues.back() < i_medianVector.back()) || (i_inputValues.back() >= maximum_criterion))
            {
                i_badFrames.push_back(i_inputValues.size()-1);
            }
        }
        if (!explicitThresholds) {
            i_thresholds[1]+= i_recalculatedMaximum;
            i_thresholds[0] = absoluteMinimum;
        }
        return true;
    } catch (std::exception e) {
        return false;
    }
}

int findReferentialNumber(double& i_recalculatedMaximum, QVector<double>& i_forEvaluation,
                                 QVector<double>& i_inputValues)
{
    double difference = 1000.0;
    int referentialNumber = 0;
    for (int i = 0; i < i_forEvaluation.size(); i++)
    {
        if (std::abs(i_recalculatedMaximum - i_inputValues[int(i_forEvaluation[i])]) < difference)
        {
            referentialNumber = int(i_forEvaluation[i]);
            difference = i_recalculatedMaximum - i_inputValues[int(i_forEvaluation[i])];
        }
    }
    return referentialNumber;
}

void integrityCheck(QVector<int> &i_badFrames)
{
    QVector<int> vectorOfDifferences(i_badFrames.size(),0);
    for (int j = 0; j < i_badFrames.size()-1; j++)
    {
        vectorOfDifferences[j] = i_badFrames[j+1] - i_badFrames[j];
    }
    int b = 0;
    while (b < (vectorOfDifferences.size()-1))
    {
        if (vectorOfDifferences[b]==2.0)
        {
            i_badFrames.insert(i_badFrames.begin()+b+1,1,i_badFrames[b]+1);
            vectorOfDifferences.insert(vectorOfDifferences.begin()+b+1,1,1);
            b+=2;
        }
        else{b+=1;}
    }
}

int findReferenceFrame(QVector<int> i_vectorEvaluation)
{
    int foundReference = -1;
    for (int frame = 0; frame < i_vectorEvaluation.length(); frame++)
    {
        if (i_vectorEvaluation[frame] == 2){
            foundReference = frame;
            break;
        }
    }
    return foundReference;
}

QVector<QVector<int> > divideIntoPeaces(int i_totalLength, int i_threadCount)
{
    QVector<int> lowerBoundary;
    QVector<int> upperBoundary;
    QVector<QVector<int>> output;
    if (i_threadCount == 1){
        lowerBoundary.push_back(0);
        upperBoundary.push_back(i_totalLength-1);

    }
    else if (i_threadCount > 1){
        if (i_totalLength%i_threadCount == 0){
            int pom = 1;
            int smallestPart = (i_totalLength/i_threadCount)-1;
            lowerBoundary.push_back(0);
            upperBoundary.push_back(smallestPart);
            while (pom < i_threadCount){
                lowerBoundary.push_back(smallestPart*pom+pom);
                upperBoundary.push_back(smallestPart*pom+pom+smallestPart);
                pom+=1;
            }
        }
        else{
            int rest = 1;
            i_totalLength-=1;
            while(i_totalLength%i_threadCount !=0){
                rest+=1;
                i_totalLength-=1;
            }
            int pom = 1;
            int smallestPart = (i_totalLength/i_threadCount)-1;
            lowerBoundary.push_back(0);
            upperBoundary.push_back(smallestPart);
            while (pom < i_threadCount){
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

void analyseAndSaveFirst(QString i_analysedFolder, QMap<QString,QString> &i_whereToSave){
    QString folder,filename,suffix;
    QStringList filesFound;
    int foundCount = 0;
    analyseFileNames(i_analysedFolder,filesFound,foundCount,"avi");
    if (foundCount != 0){
        QString fullName = i_analysedFolder+"/"+filesFound.at(0);
        processFilePath(fullName,folder,filename,suffix);
        i_whereToSave["folder"] = folder;
        i_whereToSave["filename"] = filename;
        i_whereToSave["suffix"] = suffix;
    }
}
