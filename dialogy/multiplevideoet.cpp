#include "dialogy/multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/upravy_obrazu.h"
//#include "hlavni_program/t_b_ho.h"
#include "hlavni_program/frangi_detektor.h"
#include "dialogy/grafet.h"
#include "dialogy/clickimageevent.h"
#include "util/souborove_operace.h"
#include "multithreadET/qThreadFirstPart.h"
#include "util/prace_s_vektory.h"
#include "licovani/rozhodovaci_algoritmy.h"
#include "licovani/multiPOC_Ai1.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

extern QString videaKanalyzeAktual;
extern QString ulozeniVideiAktual;
extern QString TXTnacteniAktual;
extern QString TXTulozeniAktual;
extern QString paramFrangi;

MultipleVideoET::MultipleVideoET(QWidget *parent) :
    ETanalysisParent (parent),
    ui(new Ui::MultipleVideoET)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->showResultsPB->setEnabled(false);
    ui->showResultsPB->setText(tr("Show results"));
    ui->savePB->setEnabled(false);
    ui->savePB->setText(tr("Save computed parameters"));
    ui->horizontalAnomaly->setEnabled(false);
    ui->horizontalAnomaly->setText(tr("Top/bottom anomaly"));
    ui->verticalAnomaly->setEnabled(false);
    ui->verticalAnomaly->setText(tr("Left/right anomaly"));
    ui->areaMaximum->setPlaceholderText("0-20");
    ui->rotationAngle->setPlaceholderText("0 - 0.5");
    ui->iterationCount->setPlaceholderText("1-Inf; -1~auto");
    ui->areaSizelabel->setText(tr("Size of calculation area"));
    ui->angleTolerationlabel->setText(tr("Maximal tolerated rotation angle"));
    ui->numberIterationlabel->setText(tr("Number of iterations of algorithm"));
    ui->afewVideosPB->setText(tr("Choose few files"));
    ui->wholeFolderPB->setText(tr("Choose whole folder"));
    ui->deleteChosenFromListPB->setText(tr("Delete selected"));
    ui->analyzeVideosPB->setText(tr("Analyse videos"));

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

    //QObject::connect()
}

MultipleVideoET::~MultipleVideoET()
{
    delete ui;
}

void MultipleVideoET::checkPaths(){
    if (paramFrangi != ""){
        QFile soubor;
        soubor.setFileName(paramFrangi+"/frangiParameters.json");
        FrangiParametersFile = readJson(soubor);
        QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
        for (int a = 0; a < 6; a++)
        {
            //inicializace_frangi_opt(FrangiParametersFile,parametry.at(a),FrangiParametersVector,a);
        }
    }
}
void MultipleVideoET::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
       if (!mimeData->hasUrls()) {
           return;
       }
       QList<QUrl> urls = mimeData->urls();
       QStringList seznamVidei;
       foreach (QUrl url,urls){
           QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
           if (mime.inherits("video/x-msvideo")) {
                videoList.append(url.toLocalFile());
                seznamVidei.append(url.toLocalFile());
              }
       }
       //videoList = seznamVidei;
       qDebug()<<"Aktualizace seznamu videi: "<<videoList;
       ui->vybranaVidea->addItems(seznamVidei);
}

void MultipleVideoET::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MultipleVideoET::on_nekolikVideiPB_clicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),videaKanalyzeAktual,tr("Video files (*.avi);;;") );
    if( !filenames.isEmpty() )
    {
        for (int i =0;i<filenames.count();i++)
        {
            ui->vybranaVidea->addItem(filenames.at(i));
            videoList.append(filenames.at(i));
        }
    }
}

void MultipleVideoET::on_celaSlozkaPB_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),videaKanalyzeAktual,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir chosenDirectory(dir);
    //qDebug()<<"Chosen directory:"<<dir;
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.avi" << "*.AVI",QDir::Files);
    //qDebug()<<"contains "<<videosInDirectory.length();
    if (!videosInDirectory.isEmpty())
    {
        for (int a = 0; a < videosInDirectory.count();a++)
        {
            videoList.append(videosInDirectory.at(a));
        }
        ui->vybranaVidea->addItems(videosInDirectory);
    }
    qDebug()<<"videoList contains "<<videoList.count()<<" videos.";
}

void MultipleVideoET::on_ETanalyzaVideiPB_clicked()
{
    QString pom = videoList.at(0);
    cv::VideoCapture cap = cv::VideoCapture(pom.toLocal8Bit().constData());
    if (!cap.isOpened())
        qWarning()<<"Error opening video";
    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
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
    qDebug()<<"videoList contains "<<videoList.count()<<" videos.";
    TFirstP = new qThreadFirstPart(videoList,obtainedVerticalAnomaly,
                                          obtainedHorizontalAnomaly,FrangiParametersVector,
                                          volbaSvetAnomETSingle,volbaCasZnackyETSingle);
    connect(TFirstP,SIGNAL(percentageCompleted(int)),ui->computationProgress,SLOT(setValue(int)));
    connect(TFirstP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
    connect(TFirstP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
    connect(TFirstP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
    TFirstP->start();
}

void MultipleVideoET::on_zobrazVysledkyPB_clicked()
{
    QStringList vektorKzapisu;
    for (int a=0; a<videoList.count(); a++)
    {
        QString fullPath = videoList.at(a);
        QString slozka,jmeno,koncovka;
        zpracujJmeno(fullPath,slozka,jmeno,koncovka);
        vektorKzapisu.append(jmeno);
    }
    GrafET* graf_ET = new GrafET(entropy,tennengrad,framesFirstFullCompleteEntropyEvaluation,
                                 framesFirstFullCompleteTennengradEvaluation,framesFirstFullCompleteDecision,
                                 framesSecondFullCompleteDecision,framesFinalCompleteDecision,vektorKzapisu,this);
    graf_ET->setModal(true);
    graf_ET->show();
}

void MultipleVideoET::on_vymazatZVyberuPB_clicked()
{
    QList<QListWidgetItem*> selectedVideos = ui->vybranaVidea->selectedItems();
    //qDebug()<<"Selected videos will be deleted: "<<selectedVideos;
    foreach (QListWidgetItem* item,selectedVideos)
    {
        int index = ui->vybranaVidea->row(item);
        videoList.removeAt(index);
        delete ui->vybranaVidea->takeItem(ui->vybranaVidea->row(item));
    }
    //qDebug()<<"Number of videos after deletion: "<<ui->vybranaVidea->count();
    //qDebug()<<"Number of videos in the video list: "<<videoList.count();
}

void MultipleVideoET::on_ulozeni_clicked()
{

    for (int a = 0; a < videoList.count(); a++)
    {
        QJsonDocument document;
        QJsonObject object;
        QVector<double> pomVecE = entropy[0];
        QVector<double> pomVecT = tennengrad[0];
        QVector<int> pomVecPOE = framesFirstFullCompleteEntropyEvaluation[0];
        QVector<int> pomVecPOT = framesFirstFullCompleteTennengradEvaluation[0];
        QVector<int> pomVecPR = framesFirstFullCompleteDecision[0];
        QVector<int> pomVecDR = framesSecondFullCompleteDecision[0];
        QVector<int> pomVecO = framesFinalCompleteDecision[0];
        QVector<double> pomVecFX = framesFrangiX[0];
        QVector<double> pomVecFY = framesFrangiY[0];
        QVector<double> pomVecFE = framesFrangiEuklid[0];
        QVector<double> pomVecPX = framesPOCX[0];
        QVector<double> pomVecPY = framesPOCY[0];
        QVector<double> pomVecU = framesAngle[0];
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
        QString slozka,jmeno,koncovka;
        QString fullPath = videoList.at(a);
        zpracujJmeno(fullPath,slozka,jmeno,koncovka);
        QString aktualJmeno = jmeno;
        QString cesta = TXTulozeniAktual+"/"+aktualJmeno+".dat";
        object["entropie"] = poleE;
        object["tennengrad"] = poleT;
        object["Ohodnoceni"] = poleO;
        object["PrvotOhodEntropie"] = polePOE;
        object["PrvotOhodTennengrad"] = polePOT;
        object["PrvniRozhod"] = polePR;
        object["DruheRozhod"] = poleDR;
        object["FrangiX"] = poleFX;
        object["FrangiY"] = poleFY;
        object["FrangiEuklid"] = poleFE;
        object["POCX"] = polePX;
        object["POCY"] = polePY;
        object["Uhel"] = poleU;
        document.setObject(object);
        QString documentString = document.toJson();
        QFile zapis;
        zapis.setFileName(cesta);
        zapis.open(QIODevice::WriteOnly);
        zapis.write(documentString.toLocal8Bit());
        zapis.close();
    }
}

void MultipleVideoET::zpracovano(int dokonceno)
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
        TSecondP = new qThreadSecondPart(videoList,obtainedCutoffStandard,obtainedCutoffExtra,
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
        TThirdP = new qThreadThirdPart(videoList,badFramesComplete,framesFinalCompleteDecision,
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
        TFourthP = new qThreadFourthPart(videoList,framesFirstFullCompleteDecision,framesFinalCompleteDecision,
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
        TFifthP = new qThreadFifthPart(videoList,obtainedCutoffStandard,obtainedCutoffExtra,
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
        ui->showResultsPB->setEnabled(true);
        ui->savePB->setEnabled(true);
    }
}

void MultipleVideoET::newVideoProcessed(int index)
{
    ui->actualVideo_label->setText("Analysing: "+videoList.at(index)+" ("+QString::number(index+1)+"/"+QString::number(videoList.size())+")");
}

void MultipleVideoET::movedToMethod(int metoda)
{
    if (metoda == 0)
        ui->actualMethod_label->setText("1/5 Entropy and tennengrad computation");
    if (metoda == 1)
        ui->actualMethod_label->setText("2/5 Average correlation and FWHM");
    if (metoda == 2)
        ui->actualMethod_label->setText("3/5 First decision algorithm started");
    if (metoda == 3)
        ui->actualMethod_label->setText("4/5 Second decision algorithm started");
    if (metoda == 4)
        ui->actualMethod_label->setText("5/5 Third decision algorithm started");
}

void MultipleVideoET::terminatedByError(int where)
{

}

void MultipleVideoET::on_oblastMaxima_textChanged(const QString &arg1)
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

void MultipleVideoET::on_uhelRotace_textChanged(const QString &arg1)
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

void MultipleVideoET::on_pocetIteraci_textChanged(const QString &arg1)
{
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 && zadane_cislo != -1)
    {
        ui->iterationCount->setStyleSheet("color: #FF0000");
        iteration = -1;
    }
    if (zadane_cislo == -1 || zadane_cislo > 1)
    {
        ui->iterationCount->setStyleSheet("color: #339900");
        if (zadane_cislo == -1)
            iteration = -1;
        else
            iteration = zadane_cislo;
    }
}
