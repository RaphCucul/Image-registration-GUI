#include "dialogy/singlevideoet.h"
#include "ui_singlevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "hlavni_program/frangi_detektor.h"
#include "dialogy/grafet.h"
#include "dialogy/clickimageevent.h"
#include "multithreadET/qThreadFirstPart.h"
#include "util/prace_s_vektory.h"
#include "util/souborove_operace.h"
#include "licovani/rozhodovaci_algoritmy.h"
#include "licovani/multiPOC_Ai1.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <limits>

#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <QLabel>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

extern QString videaKanalyzeAktual;
extern QString ulozeniVideiAktual;
extern QString TXTnacteniAktual;
extern QString TXTulozeniAktual;
extern QString paramFrangi;

SingleVideoET::SingleVideoET(QWidget *parent) :
    ETanalysisParent (parent),
    ui(new Ui::SingleVideoET)
{
    ui->setupUi(this);
    ui->horizontalAnomalyCB->setEnabled(false);
    ui->verticalAnomalyCB->setEnabled(false);
    ui->calculateET->setEnabled(false);
    ui->showGraphET->setEnabled(false);
    ui->savePB->setEnabled(false);
    ui->areaMaximum->setPlaceholderText("0 - 20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationNumber->setPlaceholderText("1 - Inf; -1~automatic settings");

    ui->chooseVideoPB->setText(tr("Choose video"));
    ui->referencialNumberLE->setPlaceholderText(tr("Write number of referrence frame"));
    ui->horizontalAnomalyCB->setText(tr("Top/bottom anomaly"));
    ui->verticalAnomalyCB->setText(tr("Left/right anomaly"));
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerancelabel->setText(tr("Maximal tolerated rotation angle"));
    ui->iterationNumberlabel->setText(tr("Number of iterations of algorithm"));
    ui->calculateET->setText(tr("Estimate entropy and tennengrade"));
    ui->showGraphET->setText(tr("Show computed results"));
    ui->savePB->setText(tr("Save results"));
    mapDouble["entropie"]=entropy;
    mapDouble["tennengrad"]=tennengrad;
    mapDouble["FrangiX"]=framesFrangiX;
    mapDouble["FrangiX"]=framesFrangiY;
    mapDouble["FrangiEuklid"]=framesFrangiEuklid;
    mapDouble["POCX"]=framesPOCX;
    mapDouble["POCY"]=framesPOCY;
    mapDouble["Uhel"]=framesAngle;
    mapInt["Ohodnoceni"]=framesFinalCompleteDecision;
    mapInt["PrvotOhodEntropie"]=framesFirstFullCompleteEntropyEvaluation;
    mapInt["PrvotOhodTennengrad"]=framesFirstFullCompleteTennengradEvaluation;
    mapInt["PrvniRozhod"]=framesFirstFullCompleteDecision;
    mapInt["DruheRozhod"]=framesSecondFullCompleteDecision;
    mapAnomalies["VerticalAnomaly"]=horizontalAnomalyPresent;
    mapAnomalies["HorizontalAnomaly"]=verticalAnomalyPresent;

    /*velikost_frangi_opt(6,FrangiParametersVector);
    if (paramFrangi != ""){
        QFile soubor;
        soubor.setFileName(paramFrangi+"/frangiParameters.json");
        FrangiParametersFile = readJson(soubor);
        QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
        for (int a = 0; a < 6; a++)
        {
            inicializace_frangi_opt(FrangiParametersFile,parametry.at(a),FrangiParametersVector,a);
        }
    }*/

    if (videaKanalyzeAktual == "")
        ui->chosenVideoLE->setPlaceholderText("Vybrane video");
    else
    {
        QString slozka,jmeno,koncovka;
        QStringList nalezeneSoubory;
        int pocetNalezenych;
        analyzuj_jmena_souboru_avi(videaKanalyzeAktual,nalezeneSoubory,pocetNalezenych,"avi");
        if (pocetNalezenych != 0)
        {
            QString celeJmeno = videaKanalyzeAktual+"/"+nalezeneSoubory.at(0);
            zpracujJmeno(celeJmeno,slozka,jmeno,koncovka);
            if (vybraneVideoETSingle.length() == 0)
            {
                vybraneVideoETSingle.push_back(slozka);
                vybraneVideoETSingle.push_back(jmeno);
                vybraneVideoETSingle.push_back(koncovka);
            }
            else
            {
                vybraneVideoETSingle.clear();
                vybraneVideoETSingle.push_back(slozka);
                vybraneVideoETSingle.push_back(jmeno);
                vybraneVideoETSingle.push_back(koncovka);
            }
            ui->chosenVideoLE->setText(jmeno);
            ui->calculateET->setEnabled(true);
        }
    }    
}

SingleVideoET::~SingleVideoET()
{
    delete ui;
}

void SingleVideoET::checkPaths(){
    if (videaKanalyzeAktual == "")
        ui->chosenVideoLE->setPlaceholderText("Vybrane video");
    else
    {
        QString slozka,jmeno,koncovka;
        QStringList nalezeneSoubory;
        int pocetNalezenych;
        analyzuj_jmena_souboru_avi(videaKanalyzeAktual,nalezeneSoubory,pocetNalezenych,"avi");
        if (pocetNalezenych != 0)
        {
            QString celeJmeno = videaKanalyzeAktual+"/"+nalezeneSoubory.at(0);
            zpracujJmeno(celeJmeno,slozka,jmeno,koncovka);
            if (vybraneVideoETSingle.length() == 0)
            {
                vybraneVideoETSingle.push_back(slozka);
                vybraneVideoETSingle.push_back(jmeno);
                vybraneVideoETSingle.push_back(koncovka);
            }
            else
            {
                vybraneVideoETSingle.clear();
                vybraneVideoETSingle.push_back(slozka);
                vybraneVideoETSingle.push_back(jmeno);
                vybraneVideoETSingle.push_back(koncovka);
            }
            ui->chosenVideoLE->setText(jmeno);
            ui->calculateET->setEnabled(true);
        }
    }

    /*if (paramFrangi != ""){
        QFile soubor;
        soubor.setFileName(paramFrangi+"/frangiParameters.json");
        FrangiParametersFile = readJson(soubor);
        QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
        for (int a = 0; a < 6; a++)
        {
            inicializace_frangi_opt(FrangiParametersFile,parametry.at(a),FrangiParametersVector,a);
        }
    }*/
}
void SingleVideoET::on_vyberVidea_clicked()
{
    QString referencniObrazek = QFileDialog::getOpenFileName(this,
         "Vyberte referenční obrázek", videaKanalyzeAktual,"*.avi;;Všechny soubory (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    zpracujJmeno(referencniObrazek,vybrana_slozka,vybrany_soubor,koncovka);
    /*int lastindexSlash = referencniObrazek.lastIndexOf("/");
    int lastIndexComma = referencniObrazek.length() - referencniObrazek.lastIndexOf(".");
    QString vybrana_slozka = referencniObrazek.left(lastindexSlash);
    QString vybrany_soubor = referencniObrazek.mid(lastindexSlash+1,
         (referencniObrazek.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = referencniObrazek.right(lastIndexComma-1);*/
    if (vybraneVideoETSingle.length() == 0)
    {
        vybraneVideoETSingle.push_back(vybrana_slozka);
        vybraneVideoETSingle.push_back(vybrany_soubor);
        vybraneVideoETSingle.push_back(koncovka);
    }
    else
    {
        vybraneVideoETSingle.clear();
        vybraneVideoETSingle.push_back(vybrana_slozka);
        vybraneVideoETSingle.push_back(vybrany_soubor);
        vybraneVideoETSingle.push_back(koncovka);
    }
    ui->chosenVideoLE->setText(vybraneVideoETSingle[1]);
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        ui->chosenVideoLE->setStyleSheet("color: #339900");
        spravnostVideaETSingle = true;
        ui->calculateET->setEnabled(true);
        int pocet_snimku = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        actualEntropy.fill(0.0,pocet_snimku);
        actualTennengrad.fill(0.0,pocet_snimku);
    }
}

void SingleVideoET::on_vybraneVideo_textChanged(const QString &arg1)
{
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+arg1+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->chosenVideoLE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
        spravnostVideaETSingle = false;
    }
    else
    {
        ui->chosenVideoLE->setStyleSheet("color: #339900");
        spravnostVideaETSingle = true;
        vybraneVideoETSingle[1] = arg1;
    }
}

void SingleVideoET::on_casovaZnacka_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,referencialNumber,2);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
    }
}

void SingleVideoET::on_svetelnaAnomalie_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,referencialNumber,1);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
    }

}

void SingleVideoET::on_vypocetET_clicked()
{
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    //pocetSnimkuVidea = cap.get(CV_CAP_PROP_FRAME_COUNT);*/
    /*int uspech_analyzy = entropie_tennengrad_videa(cap,ui->actualEntropy,aktualniTennengrad,ui->computationProgress);
    if (uspech_analyzy == 0)
        qDebug()<<"Výpočty skončily chybou.";
    else
    {
        entropie.push_back(ui->actualEntropy);
        tennengrad.push_back(aktualniTennengrad);
        ui->showGraphET->setEnabled(true);
    }*/
    if (oznacena_hranice_svetelne_anomalie.x >0.0f && oznacena_hranice_svetelne_anomalie.x < float(sirka))
    {
        obtainedVerticalAnomaly.x = oznacena_hranice_svetelne_anomalie.x;
        obtainedVerticalAnomaly.y = oznacena_hranice_svetelne_anomalie.y;
    }
    else
    {
        obtainedVerticalAnomaly.x = 0.0f;
        obtainedVerticalAnomaly.y = 0.0f;
    }
    if (oznacena_hranice_casove_znacky.y > 0.0f && oznacena_hranice_casove_znacky.y < float(vyska))
    {
        obtainedHorizontalAnomaly.y = oznacena_hranice_casove_znacky.y;
        obtainedHorizontalAnomaly.x = oznacena_hranice_casove_znacky.x;
    }
    else
    {
        obtainedHorizontalAnomaly.y = 0.0f;
        obtainedHorizontalAnomaly.x = 0.0f;
    }
    analysedVideos.append(kompletni_cesta);
    //temp(pom);
    TFirstP = new qThreadFirstPart(analysedVideos,obtainedVerticalAnomaly,
                                          obtainedHorizontalAnomaly,FrangiParametersVector,
                                          volbaSvetAnomETSingle,volbaCasZnackyETSingle);
    connect(TFirstP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
    connect(TFirstP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
    connect(TFirstP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
    connect(TFirstP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
    TFirstP->start();
}

void SingleVideoET::on_zobrazGrafET_clicked()
{
    QStringList vektorKzapisu;
    vektorKzapisu.append(vybraneVideoETSingle[1]);
    GrafET* graf_ET = new GrafET(entropy,tennengrad,framesFirstFullCompleteEntropyEvaluation,
                                 framesFirstFullCompleteTennengradEvaluation,framesFirstFullCompleteDecision,
                                 framesSecondFullCompleteDecision,framesFinalCompleteDecision,vektorKzapisu,this);
    graf_ET->setModal(true);
    graf_ET->show();
}

void SingleVideoET::on_ulozeni_clicked()
{   
    QJsonDocument document;
    QJsonObject object;
    QString aktualJmeno = vybraneVideoETSingle[1];
    QString cesta = TXTulozeniAktual+"/"+aktualJmeno+".dat";
    for (int indexVideo=0; indexVideo<entropy.length(); indexVideo++){
        for (int parameter = 0; parameter < videoParameters.count(); parameter++){
            if (parameter < 8){
                QVector<double> pomDouble = mapDouble[videoParameters.at(parameter)][indexVideo];
                QJsonArray pomArray = vector2array(pomDouble);
                object[videoParameters.at(parameter)] = pomArray;
            }
            if (parameter >= 8 && parameter < 14){
                QVector<int> pomInt = mapInt[videoParameters.at(parameter)][indexVideo];
                if (parameter == 13)
                    pomInt[framesReferencial[indexVideo]]=2;

                QJsonArray pomArray = vector2array(pomInt);
                object[videoParameters.at(parameter)] = pomArray;
            }
            else{
                if (videoParameters.at(parameter) == "VerticalAnomaly")
                    object[videoParameters.at(parameter)] = double(obtainedHorizontalAnomaly.y);
                else
                    object[videoParameters.at(parameter)] = double(obtainedVerticalAnomaly.x);
            }
        }

        /*QVector<double> pomVecE = entropie[indexVideo];
        QVector<double> pomVecT = tennengrad[indexVideo];
        QVector<int> pomVecPOE = framesFirstFullCompleteEntropyEvaluation[indexVideo];
        QVector<int> pomVecPOT = framesFirstFullCompleteTennengradEvaluation[indexVideo];
        QVector<int> pomVecPR = framesFirstFullCompleteDecision[indexVideo];
        QVector<int> pomVecDR = framesSecondFullCompleteDecision[indexVideo];
        QVector<int> pomVecO = framesFinalCompleteDecision[indexVideo];
        pomVecO[framesReferencial[indexVideo]] = 2;
        QVector<double> pomVecFX = framesFrangiX[indexVideo];
        QVector<double> pomVecFY = framesFrangiY[indexVideo];
        QVector<double> pomVecFE = framesFrangiEuklid[indexVideo];
        QVector<double> pomVecPX = framesPOCX[indexVideo];
        QVector<double> pomVecPY = framesPOCY[indexVideo];
        QVector<double> pomVecU = framesAngle[indexVideo];*/
    }
    /*
    QJsonArray poleE = vector2array(pomVecE);
    QJsonArray poleT = vector2array(pomVecT);
    QJsonArray polePOE = vector2array(pomVecPOE);
    QJsonArray polePOT = vector2array(pomVecPOT);
    QJsonArray poleO = vector2array(pomVecO);
    QJsonArray polePR = vector2array(pomVecPR);
    QJsonArray poleDR = vector2array(pomVecDR);
    QJsonArray poleFX = vector2array(pomVecFX);
    QJsonArray poleFY = vector2array(pomVecFY);
    QJsonArray poleFE = vector2array(pomVecFE);
    QJsonArray polePX = vector2array(pomVecPX);
    QJsonArray polePY = vector2array(pomVecPY);
    QJsonArray poleU = vector2array(pomVecU);
    QString aktualJmeno = vybraneVideoETSingle[1];
    QString cesta = TXTulozeniAktual+"/"+aktualJmeno+".dat";
    object[] = poleE;
    object[] = poleT;
    object[] = poleO;
    object[] = polePOE;
    object[] = polePOT;
    object[] = polePR;
    object[] = poleDR;
    object[] = poleFX;
    object[] = poleFY;
    object[] = poleFE;
    object[] = polePX;
    object[] = polePY;
    object[] = poleU;*/
    document.setObject(object);
    QString documentString = document.toJson();
    QFile zapis;
    zapis.setFileName(cesta);
    zapis.open(QIODevice::WriteOnly);
    zapis.write(documentString.toLocal8Bit());
    zapis.close();
}

void SingleVideoET::zpracovano(int dokonceno)
{
    if (dokonceno == 1)
    {
        entropy = TFirstP->vypocitanaEntropie();
        tennengrad = TFirstP->vypocitanyTennengrad();
        framesFirstFullCompleteEntropyEvaluation = TFirstP->vypocitanePrvotniOhodnoceniEntropie();
        framesFirstFullCompleteTennengradEvaluation = TFirstP->vypocitanePrvotniOhodnoceniTennengrad();
        obtainedCutoffStandard = TFirstP->vypocitanyVKstandard();
        obtainedCutoffExtra = TFirstP->vypocitanyVKextra();
        framesFinalCompleteDecision = TFirstP->vypocitaneKompletniOhodnoceni();
        framesReferencial = TFirstP->urceneReferenceVidei();
        badFramesComplete = TFirstP->vypocitaneSpatneSnimkyKomplet();
        TFirstP->quit();
        TSecondP = new qThreadSecondPart(analysedVideos,obtainedCutoffStandard,obtainedCutoffExtra,
                                         badFramesComplete,framesReferencial,false);
        connect(TSecondP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TSecondP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TSecondP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TSecondP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TSecondP->start();
    }
    if (dokonceno == 2)
    {
        averageCCcomplete = TSecondP->vypoctenyR();
        averageFWHMcomplete = TSecondP->vypocteneFWHM();
        TSecondP->quit();
        TThirdP = new qThreadThirdPart(analysedVideos,badFramesComplete,framesFinalCompleteDecision,
                                       framesReferencial,averageCCcomplete,averageFWHMcomplete,
                                       obtainedCutoffStandard,obtainedCutoffExtra,false);
        connect(TThirdP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TThirdP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TThirdP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TThirdP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TThirdP->start();
    }
    if (dokonceno == 3)
    {
        framesFinalCompleteDecision = TThirdP->snimkyUpdateOhodnoceni();
        framesFirstFullCompleteDecision = TThirdP->snimkyProvereniPrvniKompletestimated();
        QVector<QVector<double>> KKproblematickychSnimku = TThirdP->snimkyProblematicke_R();
        QVector<QVector<double>> FWHMproblematickychSnimku = TThirdP->snimkyProblematicke_FWHM();
        framesFrangiX = TThirdP->snimkyFrangiXestimated();
        framesFrangiY = TThirdP->snimkyFrangiYestimated();
        framesFrangiEuklid = TThirdP->snimkyFrangiEuklidestimated();
        framesPOCX = TThirdP->snimkyPOCXestimated();
        framesPOCY = TThirdP->snimkyPOCYestimated();
        framesAngle = TThirdP->snimkyUhelestimated();
        TThirdP->quit();
        TFourthP = new qThreadFourthPart(analysedVideos,framesFirstFullCompleteDecision,framesFinalCompleteDecision,
                                         KKproblematickychSnimku,FWHMproblematickychSnimku,framesPOCX,
                                         framesPOCY,framesAngle,framesFrangiX,framesFrangiY,framesFrangiEuklid,
                                         averageCCcomplete,averageFWHMcomplete);
        connect(TFourthP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TFourthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFourthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFourthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TFourthP->start();
    }
    if (dokonceno == 4)
    {
        framesFinalCompleteDecision = TFourthP->snimkyUpdateOhodnoceniKomplet();
        framesSecondFullCompleteDecision = TFourthP->snimkyRozhodovaniDruheKomplet();
        framesFrangiX = TFourthP->snimkyFrangiXestimated();
        framesFrangiY = TFourthP->snimkyFrangiYestimated();
        framesFrangiEuklid = TFourthP->snimkyFrangiEuklidestimated();
        framesPOCX = TFourthP->snimkyPOCXestimated();
        framesPOCY = TFourthP->snimkyPOCYestimated();
        framesAngle = TFourthP->snimkyUhelestimated();
        TFourthP->quit();
        TFifthP = new qThreadFifthPart(analysedVideos,obtainedCutoffStandard,obtainedCutoffExtra,
                                       framesPOCX,framesPOCY,framesAngle,framesFrangiX,framesFrangiY,
                                       framesFrangiEuklid,false,false,framesFinalCompleteDecision,framesSecondFullCompleteDecision,
                                       framesReferencial,FrangiParametersVector);
        connect(TFifthP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TFifthP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
        connect(TFifthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFifthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TFifthP->start();
    }
    if (dokonceno == 5)
    {
        framesFrangiX = TFifthP->snimkyFrangiXestimated();
        framesFrangiY = TFifthP->snimkyFrangiYestimated();
        framesFrangiEuklid = TFifthP->snimkyFrangiEuklidestimated();
        framesPOCX = TFifthP->snimkyPOCXestimated();
        framesPOCY = TFifthP->snimkyPOCYestimated();
        framesAngle = TFifthP->snimkyUhelestimated();
        framesFinalCompleteDecision = TFifthP->snimkyUpdateOhodnoceniKomplet();
        TFifthP->quit();
        ui->showGraphET->setEnabled(true);
        ui->savePB->setEnabled(true);
    }
}

void SingleVideoET::newVideoProcessed(int index)
{
    ui->analysedVideo_label->setText("Analysing: "+analysedVideos.at(index)+" ("+QString::number((index+1))+"/"+QString::number(analysedVideos.size())+")");
}

void SingleVideoET::movedToMethod(int metoda)
{
    if (metoda == 0)
        ui->actualAlgorithmPart_label->setText("1/5 Entropy and tennengrad computation");
    if (metoda == 1)
        ui->actualAlgorithmPart_label->setText("2/5 Average correlation and FWHM");
    if (metoda == 2)
        ui->actualAlgorithmPart_label->setText("3/5 First decision algorithm started");
    if (metoda == 3)
        ui->actualAlgorithmPart_label->setText("4/5 Second decision algorithm started");
    if (metoda == 4)
        ui->actualAlgorithmPart_label->setText("5/5 Third decision algorithm started");
}

void SingleVideoET::on_oblastMaxima_textChanged(const QString &arg1)
{
    double oblast_maxima_minimum = 0.0;
    double oblast_maxima_maximum = 20.0;
    double zadane_cislo = arg1.toDouble();
    if (zadane_cislo < oblast_maxima_minimum || zadane_cislo > oblast_maxima_maximum)
    {
        ui->areaMaximum->setStyleSheet("color: #FF0000");
        areaMaximum = -1;
    }
    else
    {
        ui->areaMaximum->setStyleSheet("color: #339900");
        areaMaximum = zadane_cislo;
    }
}

void SingleVideoET::on_uhelRotace_textChanged(const QString &arg1)
{
    double oblast_maxima_minimum = 0.0;
    double oblast_maxima_maximum = 0.5;
    double zadane_cislo = arg1.toDouble();
    if (zadane_cislo < oblast_maxima_minimum || zadane_cislo > oblast_maxima_maximum)
    {
        ui->rotationAngle->setStyleSheet("color: #FF0000");
        angle = 0.1;
    }
    else
    {
        ui->rotationAngle->setStyleSheet("color: #339900");
        angle = zadane_cislo;
    }
}

void SingleVideoET::on_pocetIteraci_textChanged(const QString &arg1)
{
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 && zadane_cislo != -1)
    {
        ui->iterationNumber->setStyleSheet("color: #FF0000");
        iteration = -1;
    }
    if (zadane_cislo == -1 || zadane_cislo > 1)
    {
        ui->iterationNumber->setStyleSheet("color: #339900");
        if (zadane_cislo == -1)
            iteration = -1;
        else
            iteration = zadane_cislo;
    }
}

void SingleVideoET::on_cisloReferencnihosnimku_textChanged(const QString &arg1)
{
    QString kompletni_cesta = vybraneVideoETSingle[0]+"/"+vybraneVideoETSingle[1]+"."+vybraneVideoETSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    int pocet_snimku_videa = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 || zadane_cislo > pocet_snimku_videa)
    {
        ui->referencialNumberLE->setStyleSheet("color: #FF0000");
        //qDebug()<<"Referencni snimek nelze ve videu dohledat";
        referencialNumber = -1;
    }
    else
    {
        ui->referencialNumberLE->setStyleSheet("color: #339900");
        referencialNumber = zadane_cislo;
        ui->horizontalAnomalyCB->setEnabled(true);
        ui->verticalAnomalyCB->setEnabled(true);
    }
}

/*void SingleVideoET::temp(QStringList pomList)
{

    /// Firstly, entropy and tennengrad are computed for each frame of each video
    /// This task is indicated by emiting 0 in typeOfMethod
    int kolikateVideo = 0;
    //for (int kolikateVideo = 0; kolikateVideo < pom.count(); kolikateVideo++)
    //{
    QString fullPath = pomList.at(kolikateVideo);
    qDebug()<<"Processing: "<<fullPath;
    cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
    int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    QVector<double> pom(frameCount,0);

    QVector<double> frangi_x,frangi_y,frangi_euklid,POC_x,POC_y,uhel;
    frangi_x = pom;frangi_y=pom;frangi_euklid=pom;POC_x=pom;POC_y=pom;uhel=pom;

    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    cv::Rect vyrez_anomalie(0,0,0,0);
    cv::Point2f hranice_anomalie;
    cv::Point2f hranice_CasZnac;
    if (ziskane_hranice_anomalie.x >0.0f && ziskane_hranice_anomalie.x < float(sirka))
    {
        hranice_anomalie.x = ziskane_hranice_anomalie.x;
        hranice_anomalie.y = ziskane_hranice_anomalie.y;
    }
    else
    {
        hranice_anomalie.x = 0.0f;
        hranice_anomalie.y = 0.0f;
    }
    if (ziskane_hranice_anomalie.y > 0.0f && ziskane_hranice_anomalie.y < float(vyska))
    {
        hranice_CasZnac.y = ziskane_hranice_anomalie.y;
        hranice_CasZnac.x = ziskane_hranice_anomalie.x;
    }
    else
    {
        hranice_CasZnac.x = 0.0f;
        hranice_CasZnac.y = 0.0f;
    }
    if (hranice_anomalie.x != 0.0f)
    {
        if (hranice_anomalie.x < float(sirka/2))
        {
            vyrez_anomalie.x = 0;
            vyrez_anomalie.y = int(hranice_anomalie.y);
            vyrez_anomalie.width = int(sirka)-int(hranice_anomalie.x)-1;
            vyrez_anomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
        }
        if (hranice_anomalie.x > float(sirka/2))
        {
            vyrez_anomalie.x = 0;
            vyrez_anomalie.y = 0;
            vyrez_anomalie.width = int(hranice_anomalie.x);
            vyrez_anomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
        }
    }
    QVector<double> entropyActual,tennengradActual;
    entropyActual.fill(0.0,frameCount);
    tennengradActual.fill(0.0,frameCount);
    int uspech_analyzy = 0;
    if (cap.isOpened() == 0)
        qWarning()<<"Nelze nacist";
    else
    {
        double pocet_snimku_videa = (cap.get(CV_CAP_PROP_FRAME_COUNT));
        qDebug()<< "Analyza videa: "<<kolikateVideo;
        for (double a = 0; a < pocet_snimku_videa; a++)
        {
            //qDebug()<<(kolikateVideo/pocetVidei)*100;//+((a/pocet_snimku_videa)*100.0)/pocetVidei;
            //emit percentageCompleted(qRound((kolikateVideo/pocetVidei)*100+((a/pocet_snimku_videa)*100.0)/pocetVidei));
            cv::Mat snimek;
            double hodnota_entropie;
            cv::Scalar hodnota_tennengrad;
            cap.set(CV_CAP_PROP_POS_FRAMES,(a));
            if (!cap.read(snimek))
                continue;
            else
            {
                kontrola_typu_snimku_8C3(snimek);
                vypocet_entropie(snimek,hodnota_entropie,hodnota_tennengrad); /// výpočty proběhnou v pořádku
                double pomocna = hodnota_tennengrad[0];
                //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
                entropyActual[int(a)] = (hodnota_entropie);
                tennengradActual[int(a)] = (pomocna);
                snimek.release();
            }
        }
        uspech_analyzy = 1;
        //procento = qRound(100.0/kolikateVideo+2);
    }
    entropie.push_back(entropyActual);
    tennengrad.push_back(tennengradActual);
    qDebug()<<entropie.size()<<tennengrad.size();

    /// Secondly, it is necessary to select proper maximal value for later selection of bad images
    /// after this procedure, windows for detail analysis of entropy and tennengrad vectors are computed
    /// medians of values in the windows are computed
    double correctEntropyMax = kontrola_maxima(entropie[kolikateVideo]);
    double correctTennengradMax = kontrola_maxima(tennengrad[kolikateVideo]);
    QVector<double> windows_tennengrad,windows_entropy,windowsEntropy_medians,windowsTennengrad_medians;
    double restEntropy = 0.0,restTennengrad = 0.0;
    okna_vektoru(entropie[kolikateVideo],windows_entropy,restEntropy);
    okna_vektoru(tennengrad[kolikateVideo],windows_tennengrad,restTennengrad);
    windowsEntropy_medians = mediany_vektoru(entropie[kolikateVideo],windows_entropy,restEntropy);
    windowsTennengrad_medians = mediany_vektoru(tennengrad[kolikateVideo],windows_tennengrad,restTennengrad);
    qDebug()<<windowsEntropy_medians;
    qDebug()<<windowsTennengrad_medians;

    /// Thirdly, values of entropy and tennengrad are evaluated and frames get mark good/bad, if they are
    /// or they are not suitable for image registration
    QVector<double> thresholdsEntropy(2,0);
    QVector<double> thresholdsTennengrad(2,0);
    thresholdsEntropy[0] = 0.01;thresholdsEntropy[1] = 0.01;
    thresholdsTennengrad[0] = 10;thresholdsTennengrad[1] = 10;
    double toleranceEntropy = 0.001;
    double toleranceTennengrad = 0.1;
    int dmin = 1;
    QVector<double> badFramesEntropy, badFramesTennengrad;
    QVector<double> nextAnalysisEntropy, nextAnalysisTennengrad;
    analyza_prubehu_funkce(entropie[kolikateVideo],windowsEntropy_medians,windows_entropy,
                           correctEntropyMax,thresholdsEntropy,toleranceEntropy,dmin,restEntropy,
                           badFramesEntropy,nextAnalysisEntropy);
    analyza_prubehu_funkce(tennengrad[kolikateVideo],windowsTennengrad_medians,windows_tennengrad,
                           correctTennengradMax,thresholdsTennengrad,toleranceTennengrad,dmin,restTennengrad,
                           badFramesTennengrad,nextAnalysisTennengrad);
    int referencni_snimek = nalezeni_referencniho_snimku(correctEntropyMax,nextAnalysisEntropy,
                                                         entropie[kolikateVideo]);
    /// Fourth part - frangi filter is applied on the frame marked as the reference
    cv::Mat reference, reference_vyrez;
    cap.set(CV_CAP_PROP_POS_FRAMES,referencni_snimek);
    cap.read(reference);

    cv::Rect vyrez_korelace_extra(0,0,0,0);
    cv::Rect vyrez_korelace_standard(0,0,0,0);
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Mat obraz;
    cv::Point3d frangi_bod(0,0,0);
    bool zmenaMeritka = false;
    predzpracovaniKompletnihoLicovani(reference,
                                      obraz,
                                      FrangiParametersVector,
                                      hranice_anomalie,
                                      hranice_CasZnac,
                                      frangi_bod,
                                      vyrez_anomalie,
                                      vyrez_korelace_extra,
                                      vyrez_korelace_standard,
                                      cap,
                                      volbaSvetAnomETSingle,
                                      volbaCasZnackyETSingle,
                                      zmenaMeritka);
    /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
    /// for decision algorithms
    double prumerny_korelacni_koeficient = 0.0;
    double prumerne_FWHM = 0.0;
    QVector<double> spatne_snimky_komplet = spojeni_vektoru(badFramesEntropy,badFramesTennengrad);
    kontrola_celistvosti(spatne_snimky_komplet);
    analyza_FWHM(cap,referencni_snimek,frameCount,zmenaMeritka,prumerny_korelacni_koeficient,prumerne_FWHM,
                                        vyrez_korelace_standard,vyrez_korelace_extra,spatne_snimky_komplet);
    QVector<double> hodnoceniSnimku;
    hodnoceniSnimku = pom;
    for (int a = 0; a < spatne_snimky_komplet.length(); a++)
    {
        hodnoceniSnimku[int(spatne_snimky_komplet[a])] = 1;
    }
    if (restEntropy == 1.0)
    {
        spatne_snimky_komplet.push_back(frameCount-1);
    }
    int do_zbytku = int(restEntropy-1.0);
    if (restEntropy > 1)
    {
        while (do_zbytku >= 0)
        {
            spatne_snimky_komplet.push_back(frameCount-1-do_zbytku);
            do_zbytku -= 1;
        }
        spatne_snimky_komplet.push_back(frameCount-1);
    }
    /// Sixth part - first decision algorithm
    QVector<double> vypoctena_R,vypoctena_FWHM,snimkyKprovereniPrvni;
    rozhodovani_prvni(spatne_snimky_komplet,
                      hodnoceniSnimku,
                      POC_x,
                      POC_y,
                      uhel,
                      frangi_x,
                      frangi_y,
                      frangi_euklid,
                      prumerny_korelacni_koeficient,
                      prumerne_FWHM,
                      cap,
                      reference,
                      vyrez_korelace_standard,
                      vyrez_korelace_extra,
                      zmenaMeritka,
                      snimkyKprovereniPrvni,
                      vypoctena_R,
                      vypoctena_FWHM);
    if (snimkyKprovereniPrvni.empty() != true)
    {
        QVector<double> snimkyKprovereniDruhy;
        /// Seventh part - second decision algorithm
        rozhodovani_druhe(snimkyKprovereniPrvni,
                          hodnoceniSnimku,
                          vypoctena_R,
                          vypoctena_FWHM,
                          POC_x,
                          POC_y,
                          uhel,
                          frangi_x,
                          frangi_y,
                          frangi_euklid,
                          prumerny_korelacni_koeficient,
                          prumerne_FWHM,
                          snimkyKprovereniDruhy);
        if (snimkyKprovereniDruhy.empty() != true)
        {
            /// Eigth part - third decision algorithm
            rozhodovani_treti(obraz,
                              vyrez_korelace_extra,
                              vyrez_korelace_standard,
                              frangi_x,
                              frangi_y,
                              frangi_euklid,
                              POC_x,
                              POC_y,
                              uhel,
                              zmenaMeritka,
                              volbaCasZnackyETSingle,
                              cap,
                              hodnoceniSnimku,
                              snimkyKprovereniDruhy,
                              FrangiParametersVector);
        }
        else
            qWarning()<<"Continuing";
    }
    else
        qWarning()<<"Continuing";
}
//}*/
