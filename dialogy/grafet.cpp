#include "dialogy/grafet.h"
#include "analyza_obrazu/entropie.h"
#include "util/util_grafet.h"
#include "util/souborove_operace.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>
#include <QWidget>
#include <QHBoxLayout>

GrafET::GrafET(QVector<double> i_entropy,
               QVector<double> i_tennengrad,
               QVector<int> i_FirstEvalEntropy,
               QVector<int> i_FirstEvalTennengrad,
               QVector<int> i_FirstDecisionResults,
               QVector<int> i_SecondDecisionResults,
               QVector<int> i_CompleteEvaluation,
               QWidget *parent) : QDialog(parent),
    ui(new Ui::GrafET)
{
    ui->setupUi(this);

    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    entropy = i_entropy;
    tennengrad = i_tennengrad;
    framesEntropyFirstEvalComplete = i_FirstEvalEntropy;
    framesTennengradFirstEvalComplete = i_FirstEvalTennengrad;
    framesFirstEvalComplete = i_FirstDecisionResults;
    framesSecondEvalComplete = i_SecondDecisionResults;
    framesEvalComplete = i_CompleteEvaluation;

    ui->E_HPzobraz->setEnabled(false);
    ui->E_DPzobraz->setEnabled(false);
    ui->T_HPzobraz->setEnabled(false);
    ui->T_DPzobraz->setEnabled(false);

    if (framesEntropyFirstEvalComplete.length() == 0)
        ui->ohodnoceniEntropyCB->setEnabled(false);
    if (framesTennengradFirstEvalComplete.length()==0)
        ui->ohodnoceniTennenCB->setEnabled(false);
    if (framesFirstEvalComplete.length() == 0)
        ui->prvniRozhodCB->setEnabled(false);
    else
        ui->prvniRozhodCB->setEnabled(true);
    if (framesSecondEvalComplete.length() == 0)
        ui->druheRozhodCB->setEnabled(false);
    else
        ui->druheRozhodCB->setEnabled(true);

    maxEntropy = findExtremes(entropy,ExtremeType::MAX);
    minEntropy = findExtremes(entropy,ExtremeType::MIN);
    maxTennengrad = findExtremes(tennengrad,ExtremeType::MAX);
    minTennengrad = findExtremes(tennengrad,ExtremeType::MIN);

    upperThreshold_entropy = createThreshold(maxEntropy,
                                         ThresholdType::UPPER,
                                         ValueType::ENTROPY);
    lowerThreshold_entropy = createThreshold(minEntropy,ThresholdType::LOWER,ValueType::ENTROPY);
    upperThreshold_tennengrad = createThreshold(maxTennengrad,
                                           ThresholdType::UPPER,
                                           ValueType::TENNENGRAD);
    lowerThreshold_tennengrad = createThreshold(minTennengrad,ThresholdType::LOWER,ValueType::TENNENGRAD);

    valueStandardization(entropy,entropyStandard,maxEntropy,minEntropy);
    valueStandardization(tennengrad,tennengradStandard,maxTennengrad,minTennengrad);
    upperThreshold_entropyStandardized = valueStandardization(upperThreshold_entropy,maxEntropy,minEntropy);
    lowerThreshold_entropyStandardized = valueStandardization(lowerThreshold_entropy,maxEntropy,minEntropy);
    upperThreshold_tennengradStandardized = valueStandardization(upperThreshold_tennengrad,maxTennengrad,minTennengrad);
    lowerThreshold_tennengradStandardized = valueStandardization(lowerThreshold_tennengrad,maxTennengrad,minTennengrad);
    qDebug()<<"Standardizovano.";

    // generate data for vector 0->frameCount
    /*std::vector<double> snimky(frameCount);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    valueRange = QVector<double>::fromStdVector(snimky);*/

    QWidget *WSCustomPlot = new QWidget();
    QCustomPlot* GrafickyObjekt = new QCustomPlot(WSCustomPlot);
    ui->graphLayout->addWidget(GrafickyObjekt);

    QObject::connect(ui->zobrazGrafE,SIGNAL(stateChanged(int)),this,SLOT(showEntropy()));
    QObject::connect(ui->zobrazGrafT,SIGNAL(stateChanged(int)),this,SLOT(showTennengrad()));
    QObject::connect(ui->E_HPzobraz,SIGNAL(stateChanged(int)),this,SLOT(showEntropyUpperThreshold()));
    QObject::connect(ui->E_DPzobraz,SIGNAL(stateChanged(int)),this,SLOT(showEntropyLowerThreshold()));
    QObject::connect(ui->T_HPzobraz,SIGNAL(stateChanged(int)),this,SLOT(showTennengradUpperThreshold()));
    QObject::connect(ui->T_DPzobraz,SIGNAL(stateChanged(int)),this,SLOT(showTennengradLowerThreshold()));
    QObject::connect(ui->E_HP,SIGNAL(valueChanged(double)),this,SLOT(showEntropyUpperThreshold()));
    QObject::connect(ui->E_DP,SIGNAL(valueChanged(double)),this,SLOT(showEntropyLowerThreshold()));
    QObject::connect(ui->T_HP,SIGNAL(valueChanged(double)),this,SLOT(showTennengradUpperThreshold()));
    QObject::connect(ui->T_DP,SIGNAL(valueChanged(double)),this,SLOT(showTennengradLowerThreshold()));
    QObject::connect(ui->ohodnoceniEntropyCB,SIGNAL(stateChanged(int)),this,SLOT(firstEvaluationEntropy_f()));
    QObject::connect(ui->ohodnoceniTennenCB,SIGNAL(stateChanged(int)),this,SLOT(firstEvaluationTennengrad_f()));
    QObject::connect(ui->prvniRozhodCB,SIGNAL(stateChanged(int)),this,SLOT(firstEvaluation_f()));
    QObject::connect(ui->druheRozhodCB,SIGNAL(stateChanged(int)),this,SLOT(secondEvaluation_f()));

    frameCount = entropy.length();

    UT_entropy = thresholdLine(upperThreshold_entropy,frameCount);
    LT_entropy = thresholdLine(lowerThreshold_entropy,frameCount);
    HP_tennengrad = thresholdLine(upperThreshold_tennengrad,frameCount);
    DP_tennengrad = thresholdLine(lowerThreshold_tennengrad,frameCount);

    QVector<double> snimky(frameCount);
    std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
    valueRange = snimky;

    inicializujGrafickyObjekt(GrafickyObjekt,entropy,tennengrad,
                              entropyStandard,tennengradStandard,
                              UT_entropy,LT_entropy,HP_tennengrad,DP_tennengrad,valueRange);
    ActualGraphicalObject = GrafickyObjekt;
    ui->zobrazGrafE->setChecked(true);
    ui->E_HP->setValue(upperThreshold_entropy);
    ui->E_DP->setValue(lowerThreshold_entropy);
    ui->T_HP->setValue(upperThreshold_tennengrad);
    ui->T_DP->setValue(lowerThreshold_tennengrad);
}

GrafET::~GrafET()
{
    delete ui;
}

double GrafET::findExtremes(QVector<double> &i_analysedVector, ExtremeType extreme)
{
    if (extreme == ExtremeType::MAX)
    {
        QVector<double>::iterator maximumIterator = std::max_element(i_analysedVector.begin(),i_analysedVector.end());
        int maximalPosition = std::distance(i_analysedVector.begin(),maximumIterator);
        qDebug()<<"Maximum: "<<i_analysedVector[maximalPosition];
        return i_analysedVector[maximalPosition];
    }
    else
    {
        QVector<double>::iterator minimumIterator = std::min_element(i_analysedVector.begin(),i_analysedVector.end());
        int minimalPosition = std::distance(i_analysedVector.begin(),minimumIterator);
        qDebug()<<"Minimum: "<<i_analysedVector[minimalPosition];
        return i_analysedVector[minimalPosition];
    }
}
double GrafET::createThreshold(double i_extrem, ThresholdType threshold, ValueType type)
{
    double thresholdValue = 0.0;
    if (type == ValueType::ENTROPY)
    {
        if (threshold == ThresholdType::UPPER)
            thresholdValue = (i_extrem-0.01);
        if (threshold == ThresholdType::LOWER)
            thresholdValue = (i_extrem+0.01);
    }
    if (type == ValueType::TENNENGRAD)
    {
        if (threshold == ThresholdType::UPPER)
            thresholdValue = (i_extrem-5);
        if (threshold == ThresholdType::LOWER)
            thresholdValue = (i_extrem+5);
    }
    return thresholdValue;
}
void GrafET::valueStandardization(QVector<double>& i_inputVector, QVector<double>& i_standardizedVector,
                                  double i_max, double i_min)
{
    int frameCount = i_inputVector.length();
    QVector<double> pom;
    pom.fill(0.0,frameCount);
    i_standardizedVector = pom;
    for (int b = 0; b < frameCount; b++)
        i_standardizedVector[b] = (i_inputVector[b]-i_min)/(i_max-i_min);
}

double GrafET::valueStandardization(double i_originalValue, double i_max, double i_min){
    return ((i_originalValue-i_min)/(i_max-i_min));
}

QVector<double> GrafET::thresholdLine(double i_originalValueForVector, int i_frameCount)
{
    QVector<double> pom;
    pom.fill(i_originalValueForVector,i_frameCount);
    return pom;
}

/*void GrafET::zmenaTabu(int indexTabu)
{
    if (pocetVidei > 1)
    {
        ui->zobrazGrafE->setChecked(false);
        ui->grafyTBW->setCurrentIndex(indexTabu);
        qDebug()<<ui->grafyTBW->currentIndex();
        frameCount = entropy[indexTabu].length();
        //std::vector<double> snimky(frameCount);
        QVector<double> snimky(frameCount);
        std::generate(snimky.begin(),snimky.end(),[n = 0] () mutable { return n++; });
        //valueRange = QVector<double>::fromStdVector(snimky);
        valueRange = snimky;
        if (indexTabu > UT_entropy.size())
        {
            thresholdLine(upperThreshold_entropy,UT_entropy,indexTabu,frameCount);
            thresholdLine(lowerThreshold_entropy,LT_entropy,indexTabu,frameCount);
            thresholdLine(upperThreshold_tennengrad,HP_tennengrad,indexTabu,frameCount);
            thresholdLine(lowerThreshold_tennengrad,DP_tennengrad,indexTabu,frameCount);
            qDebug()<<"zmena velikosti vektoru prahu";
        }
        //qDebug()<<"nebylo nutne menit velikost";
        QWidget* w = ui->grafyTBW->currentWidget();
        QCustomPlot* GrafickyObjekt = qobject_cast<QCustomPlot*>(w);
        ui->grafyTBW->setCurrentWidget(GrafickyObjekt);
        inicializujGrafickyObjekt(GrafickyObjekt,entropy[indexTabu],tennengrad[indexTabu],
                                  entropyStandard[indexTabu],tennengradStandard[indexTabu],
                                  UT_entropy[indexTabu],LT_entropy[indexTabu],
                                  HP_tennengrad[indexTabu],DP_tennengrad[indexTabu],valueRange);
        ActualGraphicalObject = GrafickyObjekt;

        ui->zobrazGrafE->setChecked(true);
        ui->E_HP->setValue(upperThreshold_entropy[indexTabu]);
        ui->E_DP->setValue(lowerThreshold_entropy[indexTabu]);
        ui->T_HP->setValue(upperThreshold_tennengrad[indexTabu]);
        ui->T_DP->setValue(lowerThreshold_tennengrad[indexTabu]);
        aktualniIndex = indexTabu;
        //qDebug()<<ui->grafyTBW->currentIndex();
    }
}*/

void GrafET::showEntropy()
{
    if(ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
    {
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(minEntropy-0.5,maxEntropy+0.5);
        ActualGraphicalObject->graph(0)->setVisible(true);

        ActualGraphicalObject->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(false);ui->ohodnoceniTennenCB->setChecked(false);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        ActualGraphicalObject->graph(0)->setVisible(false);
        ActualGraphicalObject->graph(2)->setVisible(false);
        ActualGraphicalObject->graph(3)->setVisible(false);
        ui->E_HPzobraz->setChecked(false);
        ui->E_DPzobraz->setChecked(false);

        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(minTennengrad-1,maxTennengrad+1);
        ActualGraphicalObject->graph(1)->setVisible(true);

        HP_tennengrad.fill(0.0,frameCount);
        HP_tennengrad.fill(upperThreshold_tennengrad,frameCount);
        ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
        DP_tennengrad.fill(0.0,frameCount);
        DP_tennengrad.fill(lowerThreshold_tennengrad,frameCount);
        ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);

        ActualGraphicalObject->replot();
        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(false);
        ui->ohodnoceniTennenCB->setEnabled(true);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        ActualGraphicalObject->graph(0)->setVisible(false);
        ActualGraphicalObject->graph(1)->setVisible(false);

        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(0,1);
        ActualGraphicalObject->graph(2)->setVisible(true);
        ActualGraphicalObject->graph(3)->setVisible(true);

        if (ui->T_HPzobraz->isChecked())
        {
            HP_tennengrad.fill(0.0,frameCount);
            HP_tennengrad.fill(upperThreshold_tennengradStandardized,frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
        }
        if (ui->T_DPzobraz->isChecked())
        {
            DP_tennengrad.fill(0.0,frameCount);
            DP_tennengrad.fill(lowerThreshold_tennengradStandardized,frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
        }
        ActualGraphicalObject->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(true);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else
    {
        hideAll();
    }
}

void GrafET::showTennengrad()
{
    if(ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
    {
        ActualGraphicalObject->graph(2)->setVisible(false);
        ActualGraphicalObject->graph(3)->setVisible(false);
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(minEntropy-0.5,maxEntropy+0.5);
        ActualGraphicalObject->graph(0)->setVisible(true);

        ui->ohodnoceniTennenCB->setEnabled(false);
        ActualGraphicalObject->replot();

        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(false);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(minTennengrad-1,maxTennengrad+1);
        ActualGraphicalObject->graph(1)->setVisible(true);

        ActualGraphicalObject->replot();

        ui->T_HPzobraz->setEnabled(true);ui->ohodnoceniTennenCB->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(false);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(0,1);
        ActualGraphicalObject->graph(2)->setVisible(true);
        ActualGraphicalObject->graph(3)->setVisible(true);
        ActualGraphicalObject->graph(0)->setVisible(false);
        if (ui->E_HPzobraz->isChecked() == true)
        {
            UT_entropy.fill(0.0,frameCount);
            UT_entropy.fill(upperThreshold_entropyStandardized,frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
        }
        if (ui->E_DPzobraz->isChecked() == true)
        {
            LT_entropy.fill(0.0,frameCount);
            LT_entropy.fill(lowerThreshold_entropyStandardized,frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
        }

        ActualGraphicalObject->replot();

        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(true);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else
    {
        hideAll();
    }
}

void GrafET::showEntropyUpperThreshold()
{
    double aktualHodnotaEHP = ui->E_HP->value();
    if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked() &&
            ui->E_HPzobraz->isChecked())
    {
        //qDebug()<<"Zobrazuji graf horního prahu entropy.";
        if (std::abs(aktualHodnotaEHP-upperThreshold_entropy)>0.005)
        {
            UT_entropy.fill(0.0,frameCount);
            upperThreshold_entropy = aktualHodnotaEHP;
            UT_entropy.fill(upperThreshold_entropy,frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            ActualGraphicalObject->graph(4)->setVisible(true);
            ActualGraphicalObject->replot();
        }

    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
             ui->E_HPzobraz->isChecked())
    {
        //qDebug()<<"Zobrazuji graf horního prahu entropy.";
        if (std::abs(aktualHodnotaEHP-upperThreshold_entropy)>0.005)
        {

            UT_entropy.fill(0.0,frameCount);
            upperThreshold_entropyStandardized = (aktualHodnotaEHP-minEntropy)/(maxEntropy-minEntropy);
            //qDebug()<<"EHP Standardized: "<<upperThreshold_entropyStandardized;
            UT_entropy.fill(upperThreshold_entropyStandardized,frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            UT_entropy.fill(0.0,frameCount);
            UT_entropy.fill(upperThreshold_entropyStandardized,frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->E_HPzobraz->isChecked())
    {
        ActualGraphicalObject->graph(4)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::showEntropyLowerThreshold()
{
    double aktualniHodnotaEDP = ui->E_DP->value();
    if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked() &&
            ui->E_DPzobraz->isChecked())
    {
        //qDebug()<<"Zobrazuji graf dolního prahu tennengradu.";
        if (std::abs(aktualniHodnotaEDP-lowerThreshold_entropy)>0.005)
        {
            LT_entropy.fill(0.0,frameCount);
            lowerThreshold_entropy = aktualniHodnotaEDP;
            LT_entropy.fill(lowerThreshold_entropy,frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            ActualGraphicalObject->graph(5)->setVisible(true);
            ActualGraphicalObject->replot();
        }

    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
             ui->E_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaEDP-lowerThreshold_entropy)>0.005)
        {
            LT_entropy.fill(0.0,frameCount);
            lowerThreshold_entropyStandardized = (aktualniHodnotaEDP-minEntropy)/(maxEntropy-minEntropy);
            LT_entropy.fill(lowerThreshold_entropyStandardized,frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            LT_entropy.fill(0.0,frameCount);
            LT_entropy.fill(lowerThreshold_entropyStandardized,frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if(!ui->E_DPzobraz->isChecked())
    {
        ActualGraphicalObject->graph(5)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::showTennengradUpperThreshold()
{
    double aktualHodnotaTHP = ui->T_HP->value();
    //qDebug()<<"Aktualni hodnota THP: "<<aktualHodnotaTHP;
    if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
            ui->T_HPzobraz->isChecked())
    {
        //qDebug()<<"Zobrazuji graf horního prahu tennengradu.";
        if (std::abs(aktualHodnotaTHP-upperThreshold_tennengrad)>0.005)
        {

            HP_tennengrad.fill(0.0,frameCount);
            upperThreshold_tennengradStandardized = (aktualHodnotaTHP-minTennengrad)/(maxTennengrad-minTennengrad);
            //qDebug()<<"THP Standardized: "<<upperThreshold_tennengradStandardized;
            HP_tennengrad.fill(upperThreshold_tennengradStandardized,frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            HP_tennengrad.fill(0.0,frameCount);
            HP_tennengrad.fill(upperThreshold_tennengradStandardized,frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
             ui->T_HPzobraz->isChecked())
    {
        if (std::abs(aktualHodnotaTHP-upperThreshold_tennengrad)>0.005)
        {
            HP_tennengrad.fill(0.0,frameCount);
            upperThreshold_tennengrad = aktualHodnotaTHP;
            HP_tennengrad.fill(upperThreshold_tennengrad,frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->T_HPzobraz->isChecked())
    {
        ActualGraphicalObject->graph(6)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::showTennengradLowerThreshold()
{
    double aktualniHodnotaTDP = ui->T_DP->value();
    if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
            ui->T_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaTDP-lowerThreshold_tennengrad)>0.005)
        {
            DP_tennengrad.fill(0.0,frameCount);
            lowerThreshold_tennengradStandardized = (aktualniHodnotaTDP-minTennengrad)/(maxTennengrad-minTennengrad);
            DP_tennengrad.fill(lowerThreshold_tennengradStandardized,frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            DP_tennengrad.fill(0.0,frameCount);
            DP_tennengrad.fill(lowerThreshold_tennengradStandardized,frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
             ui->T_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaTDP-lowerThreshold_tennengrad)>0.005)
        {
            DP_tennengrad.fill(0.0,frameCount);
            lowerThreshold_tennengrad = aktualniHodnotaTDP;
            DP_tennengrad.fill(lowerThreshold_tennengrad,frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
        }
        else
        {
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if(!ui->T_DPzobraz->isChecked())
    {
        ActualGraphicalObject->graph(7)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::firstEvaluationEntropy_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->ohodnoceniEntropyCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEntropyFirstEvalComplete,
                                                 entropyStandard,0);
            souradniceDouble = transformInt2Double(framesEntropyFirstEvalComplete,0);
            ActualGraphicalObject->graph(8)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(8)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(8,framesEntropyFirstEvalComplete,entropyStandard,0);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEntropyFirstEvalComplete,
                                                 entropy,0);
            souradniceDouble = transformInt2Double(framesEntropyFirstEvalComplete,0);
            ActualGraphicalObject->graph(8)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(8)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(8,framesEntropyFirstEvalComplete,entropy,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(8)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}
void GrafET::firstEvaluationTennengrad_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->ohodnoceniTennenCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesTennengradFirstEvalComplete,
                                                 tennengradStandard,0);
            souradniceDouble = transformInt2Double(framesTennengradFirstEvalComplete,0);
            ActualGraphicalObject->graph(9)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(9)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(9,framesTennengradFirstEvalComplete,tennengradStandard,0);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesTennengradFirstEvalComplete,
                                                 tennengrad,0);
            souradniceDouble = transformInt2Double(framesTennengradFirstEvalComplete,0);
            ActualGraphicalObject->graph(9)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(9)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(9,framesTennengradFirstEvalComplete,tennengrad,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(9)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}
void GrafET::firstEvaluation_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->prvniRozhodCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesFirstEvalComplete,
                                                 entropyStandard,0);
            souradniceDouble = transformInt2Double(framesFirstEvalComplete,0);
            ActualGraphicalObject->graph(10)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(10)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(10,framesFirstEvalComplete,entropyStandard,0);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesFirstEvalComplete,
                                                 entropy,0);
            souradniceDouble = transformInt2Double(framesFirstEvalComplete,0);
            ActualGraphicalObject->graph(10)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(10)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(10,framesFirstEvalComplete,entropy,0);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesFirstEvalComplete,
                                                 tennengrad,0);
            souradniceDouble = transformInt2Double(framesFirstEvalComplete,0);
            ActualGraphicalObject->graph(10)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(10)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(10,framesFirstEvalComplete,tennengrad,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(10)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}
void GrafET::secondEvaluation_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->druheRozhodCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesSecondEvalComplete,
                                                 entropyStandard,0);
            souradniceDouble = transformInt2Double(framesSecondEvalComplete,0);
            ActualGraphicalObject->graph(11)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(11)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(11,framesSecondEvalComplete,entropyStandard,0);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesSecondEvalComplete,
                                                 entropy,0);
            souradniceDouble = transformInt2Double(framesSecondEvalComplete,0);
            ActualGraphicalObject->graph(11)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(11)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(11,framesSecondEvalComplete,entropy,0);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesSecondEvalComplete,
                                                 tennengrad,0);
            souradniceDouble = transformInt2Double(framesSecondEvalComplete,0);
            ActualGraphicalObject->graph(11)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(11)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(11,framesSecondEvalComplete,tennengrad,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(11)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_ohodnocKomplet_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                             entropyStandard,1);
        souradniceDouble = transformInt2Double(framesEvalComplete,1);
        ActualGraphicalObject->graph(12)->setData(souradniceDouble,vybraneHodnotyY);
        ActualGraphicalObject->graph(12)->setVisible(true);
        ActualGraphicalObject->replot();*/
            populateGraph(12,framesEvalComplete,entropyStandard,1);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                                 entropy,1);
            souradniceDouble = transformInt2Double(framesEvalComplete,1);
            ActualGraphicalObject->graph(12)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(12)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(12,framesEvalComplete,entropy,1);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                                 tennengrad,0);
            souradniceDouble = transformInt2Double(framesEvalComplete,0);
            ActualGraphicalObject->graph(12)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(12)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(12,framesEvalComplete,tennengrad,1);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(12)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_IV1_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                             entropyStandard,2);
        souradniceDouble = transformInt2Double(framesEvalComplete,2);
        ActualGraphicalObject->graph(13)->setData(souradniceDouble,vybraneHodnotyY);
        ActualGraphicalObject->graph(13)->setVisible(true);
        ActualGraphicalObject->replot();*/
            populateGraph(13,framesEvalComplete,entropyStandard,2);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                                 entropy,2);
            souradniceDouble = transformInt2Double(framesEvalComplete,2);
            ActualGraphicalObject->graph(13)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(13)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(13,framesEvalComplete,entropy,2);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(13,framesEvalComplete,tennengrad,2);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(13)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_IV4_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                             entropyStandard,3);
        souradniceDouble = transformInt2Double(framesEvalComplete,3);
        ActualGraphicalObject->graph(14)->setData(souradniceDouble,vybraneHodnotyY);
        ActualGraphicalObject->graph(14)->setVisible(true);
        ActualGraphicalObject->replot();*/
            populateGraph(14,framesEvalComplete,tennengrad,3);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                                 entropy,3);
            souradniceDouble = transformInt2Double(framesEvalComplete,3);
            ActualGraphicalObject->graph(14)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(14)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(14,framesEvalComplete,entropy,3);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(14,framesEvalComplete,tennengrad,3);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(14)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_IV5_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                             entropyStandard,4);
        souradniceDouble = transformInt2Double(framesEvalComplete,4);
        ActualGraphicalObject->graph(15)->setData(souradniceDouble,vybraneHodnotyY);
        ActualGraphicalObject->graph(15)->setVisible(true);
        ActualGraphicalObject->replot();*/
            populateGraph(15,framesEvalComplete,tennengrad,4);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            /*vybraneHodnotyY = vyberHodnotySnimku(framesEvalComplete,
                                                 entropy,4);
            souradniceDouble = transformInt2Double(framesEvalComplete,4);
            ActualGraphicalObject->graph(15)->setData(souradniceDouble,vybraneHodnotyY);
            ActualGraphicalObject->graph(15)->setVisible(true);
            ActualGraphicalObject->replot();*/
            populateGraph(15,framesEvalComplete,entropy,4);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(15,framesEvalComplete,tennengrad,4);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(15)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::populateGraph(int i_graphLine,
                           QVector<int> &i_y_coords, QVector<double> &i_x_coords,
                           int i_evalType){
    QVector<double> chosenYcoordinates,coordinatesInDouble;
    chosenYcoordinates = vyberHodnotySnimku(i_y_coords,i_x_coords,i_evalType);
    coordinatesInDouble = transformInt2Double(i_y_coords,i_evalType);
    ActualGraphicalObject->graph(i_graphLine)->setData(coordinatesInDouble,chosenYcoordinates);
    ActualGraphicalObject->graph(i_graphLine)->setVisible(true);
    ActualGraphicalObject->replot();
}

void GrafET::hideAll(){
    ActualGraphicalObject->graph(0)->setVisible(false);
    ActualGraphicalObject->graph(1)->setVisible(false);
    ActualGraphicalObject->graph(2)->setVisible(false);
    ActualGraphicalObject->graph(3)->setVisible(false);
    ActualGraphicalObject->graph(4)->setVisible(false);
    ActualGraphicalObject->graph(5)->setVisible(false);
    ActualGraphicalObject->graph(6)->setVisible(false);
    ActualGraphicalObject->graph(7)->setVisible(false);
    ActualGraphicalObject->replot();

    ui->E_HPzobraz->setEnabled(false);ui->E_DPzobraz->setEnabled(false);
    ui->T_HPzobraz->setEnabled(false);ui->T_DPzobraz->setEnabled(false);
    ui->E_DPzobraz->setChecked(false);ui->E_HPzobraz->setChecked(false);
    ui->T_HPzobraz->setChecked(false);ui->T_DPzobraz->setChecked(false);
    ui->ohodnoceniEntropyCB->setEnabled(false);ui->ohodnoceniEntropyCB->setChecked(false);
    ui->ohodnoceniTennenCB->setEnabled(false);ui->ohodnoceniTennenCB->setChecked(false);
    ui->prvniRozhodCB->setEnabled(false);ui->prvniRozhodCB->setChecked(false);
    ui->druheRozhodCB->setEnabled(false);ui->druheRozhodCB->setChecked(false);
    ui->IV1->setEnabled(false);ui->IV1->setChecked(false);
    ui->IV4->setEnabled(false);ui->IV4->setChecked(false);
    ui->IV5->setEnabled(false);ui->IV5->setChecked(false);
}
