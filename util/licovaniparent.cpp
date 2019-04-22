#include "util/licovaniparent.h"

LicovaniParent::LicovaniParent(QWidget *parent) : QWidget(parent)
{

}

void LicovaniParent::onFinishThread(int threadIndex){
    if (threadPool[threadIndex]->isRunning()){
        threadPool[threadIndex]->terminate();
        threadPool[threadIndex]->wait(10);
        threadPool[threadIndex]->deleteLater();
    }
    else
        threadPool[threadIndex]->deleteLater();
}

void LicovaniParent::cancelAllCalculations(){
    if (!threadPool.isEmpty()){
        for (int threadIndex = 1; threadIndex <= threadPool.count(); threadIndex++){
            if (threadPool[threadIndex]->isRunning()){
                threadPool[threadIndex]->terminate();
                threadPool[threadIndex]->wait(10);
                threadPool[threadIndex]->dataObtained();
            }
        }
    }
    canProceed = false;
    initMaps();
    emit calculationStopped();
}

void LicovaniParent::initMaps(){
    QVector<double> pomD;
    QVector<int> pomI;
    for (int index = 0; index < videoParameters.count(); index++){
        if (index < 8){
            videoParametersDouble[videoParameters.at(index)] = pomD;
        }
        else if (index >= 8 && index < 13){
            videoParametersInt[videoParameters.at(index)] = pomI;
        }
        else if (index >= 13 && index <= 14){
            videoAnomalies[videoParameters.at(index)] = pomI;
        }
    }
}
