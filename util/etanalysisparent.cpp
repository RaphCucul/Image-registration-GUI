#include "etanalysisparent.h"

#include <QDebug>

ETanalysisParent::ETanalysisParent(QWidget *parent) : QWidget(parent)
{

}

void ETanalysisParent::checkInputNumber(double input, double lower, double upper, QLineEdit *editWidget, double &finalValue, bool &evaluation){
    if (input < lower || (input > upper && upper != 0.0)){
        editWidget->setStyleSheet("color: #FF0000");
        editWidget->setText("");
        finalValue = -99;
        evaluation = false;
    }
    else if (input < lower || (upper == 0.0 && input == 0.0)){
        editWidget->setStyleSheet("color: #FF0000");
        editWidget->setText("");
        finalValue = -99;
        evaluation = false;
    }
    else{
        editWidget->setStyleSheet("color: #33aa00");
        evaluation = true;
        finalValue = input;
    }
}

void ETanalysisParent::initMaps(){
    QVector<QVector<double>> pomD;
    QVector<QVector<int>> pomI;
    QVector<int> pomI_;
    for (int index = 0; index < videoParameters.count(); index++){
        if (index < 8){
            mapDouble[videoParameters.at(index)] = pomD;
        }
        else if (index >= 8 && index < 13){
            mapInt[videoParameters.at(index)] = pomI;
        }
        else if (index >= 13 && index <= 14){
            mapAnomalies[videoParameters.at(index)] = pomI_;
        }
    }
}

void ETanalysisParent::cancelAllCalculations(){
    if (First[1]->isRunning()){
        First[1]->terminate();
        First[1]->wait(10);
        emit dataObtained_first();        
    }
    if (!Second.isEmpty()){
        if (Second[2]->isRunning()){
            Second[2]->terminate();
            Second[2]->wait(10);
            emit dataObtained_second();            
        }
    }
    if (!Third.isEmpty()){
        if(Third[3]->isRunning()){
            Third[3]->terminate();
            Third[3]->wait(10);
            emit dataObtained_third();            
        }
    }
    if (!Fourth.isEmpty()){
        if (Fourth[4]->isRunning()){
            Fourth[4]->terminate();
            Fourth[4]->wait(10);
            emit dataObtained_fourth();            
        }
    }
    if (!Fifth.isEmpty()){
        if (Fifth[5]->isRunning()){
            Fifth[5]->terminate();
            Fifth[5]->wait(10);
            emit dataObtained_fifth();            
        }
    }

    canProceed = false;
    initMaps();
    emit calculationStopped();
}

void ETanalysisParent::done(int done)
{
    if (done == 1)
    {
        mapDouble["entropie"] = First[1]->computedEntropy();
        mapDouble["tennengrad"] = First[1]->computedTennengrad();
        mapInt["PrvotOhodEntropie"] = First[1]->computedFirstEntropyEvaluation();
        mapInt["PrvotOhodTennengrad"] = First[1]->computedFirstTennengradEvalueation();
        obtainedCutoffStandard = First[1]->computedCOstandard();
        obtainedCutoffExtra = First[1]->computedCOextra();
        mapInt["Ohodnoceni"] = First[1]->computedCompleteEvaluation();
        framesReferencial = First[1]->estimatedReferencialFrames();
        badFramesComplete = First[1]->computedBadFrames();
        emit dataObtained_first();
    }
    if (done == 2)
    {
        averageCCcomplete = Second[2]->computedCC();
        averageFWHMcomplete = Second[2]->computedFWHM();
        emit dataObtained_second();
    }
    if (done == 3)
    {
        mapInt["Ohodnoceni"] = Third[3]->framesUpdateEvaluation();
        mapInt["PrvniRozhod"] = Third[3]->framesFirstEvaluationComplete();
        CC_problematicFrames = Third[3]->framesProblematic_CC();
        FWHM_problematicFrames = Third[3]->framesProblematic_FWHM();
        mapDouble["FrangiX"] = Third[3]->framesFrangiXestimated();
        mapDouble["FrangiY"] = Third[3]->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = Third[3]->framesFrangiEuklidestimated();
        mapDouble["POCX"] = Third[3]->framesPOCXestimated();
        mapDouble["POCY"] = Third[3]->framesPOCYestimated();
        mapDouble["Uhel"] = Third[3]->framesUhelestimated();
        emit dataObtained_third();
    }
    if (done == 4)
    {
        mapInt["Ohodnoceni"] = Fourth[4]->framesUpdateEvaluationComplete();
        mapInt["DruheRozhod"] = Fourth[4]->framesSecondEvaluation();
        mapDouble["FrangiX"] = Fourth[4]->framesFrangiXestimated();
        mapDouble["FrangiY"] = Fourth[4]->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = Fourth[4]->framesFrangiEuklidestimated();
        mapDouble["POCX"] = Fourth[4]->framesPOCXestimated();
        mapDouble["POCY"] = Fourth[4]->framesPOCYestimated();
        mapDouble["Uhel"] = Fourth[4]->framesAngleestimated();
        emit dataObtained_fourth();
    }
    if (done == 5)
    {
        mapDouble["FrangiX"] = Fifth[5]->framesFrangiXestimated();
        mapDouble["FrangiY"] = Fifth[5]->framesFrangiYestimated();
        mapDouble["FrangiEuklid"] = Fifth[5]->framesFrangiEuklidestimated();
        mapDouble["POCX"] = Fifth[5]->framesPOCXestimated();
        mapDouble["POCY"] = Fifth[5]->framesPOCYestimated();
        mapDouble["Uhel"] = Fifth[5]->framesAngleestimated();
        mapInt["Ohodnoceni"] = Fifth[5]->framesUpdateEvaluationComplete();
        emit dataObtained_fifth();
    }
}
