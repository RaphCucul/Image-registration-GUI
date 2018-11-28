#include "dialogy/multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "hlavni_program/t_b_ho.h"
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

MultipleVideoET::MultipleVideoET(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultipleVideoET)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->zobrazVysledkyPB->setEnabled(false);
    ui->ulozeni->setEnabled(false);
    ui->casovaZnacka->setEnabled(false);
    ui->svetAnomal->setEnabled(false);
    ui->zobrazVysledkyPB->setEnabled(false);
    ui->ulozeni->setEnabled(false);
    ui->oblastMaxima->setText("0-20");
    oblastMaxima = 10.0;
    ui->uhelRotace->setText("0 - 0.5");
    uhel = 0.1;
    ui->pocetIteraci->setText("-1");
    iterace = -1;

    velikost_frangi_opt(6,parametry_frangi);
    QFile soubor;
    soubor.setFileName(paramFrangi+"/frangiParameters.json");
    parametryFrangiJson = readJson(soubor);
    QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    for (int a = 0; a < 6; a++)
    {
        inicializace_frangi_opt(parametryFrangiJson,parametry.at(a),parametry_frangi,a);
    }
}

MultipleVideoET::~MultipleVideoET()
{
    delete ui;
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
                sezVid.append(url.toLocalFile());
                seznamVidei.append(url.toLocalFile());
              }
       }
       //sezVid = seznamVidei;
       qDebug()<<"Aktualizace seznamu videi: "<<sezVid;
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
            sezVid.append(filenames.at(i));
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
            sezVid.append(videosInDirectory.at(a));
        }
        ui->vybranaVidea->addItems(videosInDirectory);
    }
    qDebug()<<"sezVid contains "<<sezVid.count()<<" videos.";
}

void MultipleVideoET::on_ETanalyzaVideiPB_clicked()
{
    /*vlaknoET = new VicevlaknoveZpracovani(sezVid);
    connect(vlaknoET,SIGNAL(percentageCompleted(int)),ui->progBar,SLOT(setValue(int)));
    connect(vlaknoET,SIGNAL(hotovo()),this,SLOT(zpracovano()));
    vlaknoET->start();*/
    QString pom = sezVid.at(0);
    cv::VideoCapture cap = cv::VideoCapture(pom.toLocal8Bit().constData());
    if (!cap.isOpened())
        qWarning()<<"Error opening video";
    double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    if (oznacena_hranice_svetelne_anomalie.x >0.0f && oznacena_hranice_svetelne_anomalie.x < float(sirka))
    {
        ziskane_hranice_anomalie.x = oznacena_hranice_svetelne_anomalie.x;
        ziskane_hranice_anomalie.y = oznacena_hranice_svetelne_anomalie.y;
    }
    else
    {
        ziskane_hranice_anomalie.x = 0.0f;
        ziskane_hranice_anomalie.y = 0.0f;
    }
    if (oznacena_hranice_casove_znacky.y > 0.0f && oznacena_hranice_casove_znacky.y < float(vyska))
    {
        ziskane_hranice_casZnac.y = oznacena_hranice_casove_znacky.y;
        ziskane_hranice_casZnac.x = oznacena_hranice_casove_znacky.x;
    }
    else
    {
        ziskane_hranice_casZnac.y = 0.0f;
        ziskane_hranice_casZnac.x = 0.0f;
    }
    qDebug()<<"sezVid contains "<<sezVid.count()<<" videos.";
    TFirstP = new qThreadFirstPart(sezVid,ziskane_hranice_anomalie,
                                          ziskane_hranice_casZnac,parametry_frangi,
                                          volbaSvetAnomETSingle,volbaCasZnackyETSingle);
    connect(TFirstP,SIGNAL(percentageCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
    connect(TFirstP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
    connect(TFirstP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
    connect(TFirstP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
    TFirstP->start();
}

void MultipleVideoET::on_zobrazVysledkyPB_clicked()
{
    QStringList vektorKzapisu;
    for (int a=0; a<sezVid.count(); a++)
    {
        QString fullPath = sezVid.at(a);
        QString slozka,jmeno,koncovka;
        zpracujJmeno(fullPath,slozka,jmeno,koncovka);
        vektorKzapisu.append(jmeno);
    }
    GrafET* graf_ET = new GrafET(entropie,tennengrad,snimkyPrvotniOhodnoceniEntropieKomplet,
                                 snimkyPrvotniOhodnoceniTennengradKomplet,snimkyPrvniRozhodovaniKomplet,
                                 snimkyDruheRozhodovaniKomplet,snimkyOhodnoceniKomplet,vektorKzapisu,this);
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
        sezVid.removeAt(index);
        delete ui->vybranaVidea->takeItem(ui->vybranaVidea->row(item));
    }
    //qDebug()<<"Number of videos after deletion: "<<ui->vybranaVidea->count();
    //qDebug()<<"Number of videos in the video list: "<<sezVid.count();
}

void MultipleVideoET::on_ulozeni_clicked()
{

    for (int a = 0; a < sezVid.count(); a++)
    {
        QJsonDocument document;
        QJsonObject object;
        QVector<double> pomVecE = entropie[0];
        QVector<double> pomVecT = tennengrad[0];
        QVector<int> pomVecPOE = snimkyPrvotniOhodnoceniEntropieKomplet[0];
        QVector<int> pomVecPOT = snimkyPrvotniOhodnoceniTennengradKomplet[0];
        QVector<int> pomVecPR = snimkyPrvniRozhodovaniKomplet[0];
        QVector<int> pomVecDR = snimkyDruheRozhodovaniKomplet[0];
        QVector<int> pomVecO = snimkyOhodnoceniKomplet[0];
        QVector<double> pomVecFX = snimkyFrangiX[0];
        QVector<double> pomVecFY = snimkyFrangiY[0];
        QVector<double> pomVecFE = snimkyFrangiEuklid[0];
        QVector<double> pomVecPX = snimkyPOCX[0];
        QVector<double> pomVecPY = snimkyPOCY[0];
        QVector<double> pomVecU = snimkyUhel[0];
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
        QString fullPath = sezVid.at(a);
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
        entropie = TFirstP->vypocitanaEntropie();
        tennengrad = TFirstP->vypocitanyTennengrad();
        snimkyPrvotniOhodnoceniEntropieKomplet = TFirstP->vypocitanePrvotniOhodnoceniEntropie();
        snimkyPrvotniOhodnoceniTennengradKomplet = TFirstP->vypocitanePrvotniOhodnoceniTennengrad();
        ziskany_VK_standard = TFirstP->vypocitanyVKstandard();
        ziskany_VK_extra = TFirstP->vypocitanyVKextra();
        snimkyOhodnoceniKomplet = TFirstP->vypocitaneKompletniOhodnoceni();
        snimkyReferencni = TFirstP->urceneReferenceVidei();
        spatneSnimkyKomplet = TFirstP->vypocitaneSpatneSnimkyKomplet();
        TFirstP->quit();
        TSecondP = new qThreadSecondPart(sezVid,ziskany_VK_standard,ziskany_VK_extra,
                                         spatneSnimkyKomplet,snimkyReferencni,false);
        connect(TSecondP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TSecondP,SIGNAL(percentageCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
        connect(TSecondP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TSecondP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TSecondP->start();
    }
    if (dokonceno == 2)
    {
        prumerneRKomplet = TSecondP->vypoctenyR();
        prumerneFWHMKomplet = TSecondP->vypocteneFWHM();
        TSecondP->quit();
        TThirdP = new qThreadThirdPart(sezVid,spatneSnimkyKomplet,snimkyOhodnoceniKomplet,
                                       snimkyReferencni,prumerneRKomplet,prumerneFWHMKomplet,
                                       ziskany_VK_standard,ziskany_VK_extra,false);
        connect(TThirdP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TThirdP,SIGNAL(percentageCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
        connect(TThirdP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TThirdP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TThirdP->start();
    }
    if (dokonceno == 3)
    {
        snimkyOhodnoceniKomplet = TThirdP->snimkyUpdateOhodnoceni();
        snimkyPrvniRozhodovaniKomplet = TThirdP->snimkyProvereniPrvniKompletestimated();
        QVector<QVector<double>> KKproblematickychSnimku = TThirdP->snimkyProblematicke_R();
        QVector<QVector<double>> FWHMproblematickychSnimku = TThirdP->snimkyProblematicke_FWHM();
        snimkyFrangiX = TThirdP->snimkyFrangiXestimated();
        snimkyFrangiY = TThirdP->snimkyFrangiYestimated();
        snimkyFrangiEuklid = TThirdP->snimkyFrangiEuklidestimated();
        snimkyPOCX = TThirdP->snimkyPOCXestimated();
        snimkyPOCY = TThirdP->snimkyPOCYestimated();
        snimkyUhel = TThirdP->snimkyUhelestimated();
        TThirdP->quit();
        TFourthP = new qThreadFourthPart(sezVid,snimkyPrvniRozhodovaniKomplet,snimkyOhodnoceniKomplet,
                                         KKproblematickychSnimku,FWHMproblematickychSnimku,snimkyPOCX,
                                         snimkyPOCY,snimkyUhel,snimkyFrangiX,snimkyFrangiY,snimkyFrangiEuklid,
                                         prumerneRKomplet,prumerneFWHMKomplet);
        connect(TFourthP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TFourthP,SIGNAL(percentageCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
        connect(TFourthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFourthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TFourthP->start();
    }
    if (dokonceno == 4)
    {
        snimkyOhodnoceniKomplet = TFourthP->snimkyUpdateOhodnoceniKomplet();
        snimkyDruheRozhodovaniKomplet = TFourthP->snimkyRozhodovaniDruheKomplet();
        snimkyFrangiX = TFourthP->snimkyFrangiXestimated();
        snimkyFrangiY = TFourthP->snimkyFrangiYestimated();
        snimkyFrangiEuklid = TFourthP->snimkyFrangiEuklidestimated();
        snimkyPOCX = TFourthP->snimkyPOCXestimated();
        snimkyPOCY = TFourthP->snimkyPOCYestimated();
        snimkyUhel = TFourthP->snimkyUhelestimated();
        TFourthP->quit();
        TFifthP = new qThreadFifthPart(sezVid,ziskany_VK_standard,ziskany_VK_extra,
                                       snimkyPOCX,snimkyPOCY,snimkyUhel,snimkyFrangiX,snimkyFrangiY,
                                       snimkyFrangiEuklid,false,false,snimkyOhodnoceniKomplet,snimkyDruheRozhodovaniKomplet,
                                       snimkyReferencni,parametry_frangi);
        connect(TFifthP,SIGNAL(hotovo(int)),this,SLOT(zpracovano(int)));
        connect(TFifthP,SIGNAL(percentageCompleted(int)),ui->prubehVypoctu,SLOT(setValue(int)));
        connect(TFifthP,SIGNAL(typeOfMethod(int)),this,SLOT(movedToMethod(int)));
        connect(TFifthP,SIGNAL(actualVideo(int)),this,SLOT(newVideoProcessed(int)));
        TFifthP->start();
    }
    if (dokonceno == 5)
    {
        snimkyFrangiX = TFifthP->snimkyFrangiXestimated();
        snimkyFrangiY = TFifthP->snimkyFrangiYestimated();
        snimkyFrangiEuklid = TFifthP->snimkyFrangiEuklidestimated();
        snimkyPOCX = TFifthP->snimkyPOCXestimated();
        snimkyPOCY = TFifthP->snimkyPOCYestimated();
        snimkyUhel = TFifthP->snimkyUhelestimated();
        snimkyOhodnoceniKomplet = TFifthP->snimkyUpdateOhodnoceniKomplet();
        TFifthP->quit();
        ui->zobrazVysledkyPB->setEnabled(true);
        ui->ulozeni->setEnabled(true);
    }
}

void MultipleVideoET::newVideoProcessed(int index)
{
    ui->aktualniVideo->setText("Analysing: "+sezVid.at(index)+" ("+QString::number(index+1)+"/"+QString::number(sezVid.size())+")");
}

void MultipleVideoET::movedToMethod(int metoda)
{
    if (metoda == 0)
        ui->aktualniMetoda->setText("1/5 Entropy and tennengrad computation");
    if (metoda == 1)
        ui->aktualniMetoda->setText("2/5 Average correlation and FWHM");
    if (metoda == 2)
        ui->aktualniMetoda->setText("3/5 First decision algorithm started");
    if (metoda == 3)
        ui->aktualniMetoda->setText("4/5 Second decision algorithm started");
    if (metoda == 4)
        ui->aktualniMetoda->setText("5/5 Third decision algorithm started");
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
        ui->oblastMaxima->setStyleSheet("color: #FF0000");
        oblastMaxima = -1;
    }
    else
    {
        ui->oblastMaxima->setStyleSheet("color: #339900");
        oblastMaxima = zadane_cislo;
    }
}

void MultipleVideoET::on_uhelRotace_textChanged(const QString &arg1)
{
    double oblast_maxima_minimum = 0.0;
    double oblast_maxima_maximum = 0.5;
    double zadane_cislo = arg1.toDouble();
    if (zadane_cislo < oblast_maxima_minimum || zadane_cislo > oblast_maxima_maximum)
    {
        ui->uhelRotace->setStyleSheet("color: #FF0000");
        uhel = 0.1;
    }
    else
    {
        ui->uhelRotace->setStyleSheet("color: #339900");
        uhel = zadane_cislo;
    }
}

void MultipleVideoET::on_pocetIteraci_textChanged(const QString &arg1)
{
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 && zadane_cislo != -1)
    {
        ui->pocetIteraci->setStyleSheet("color: #FF0000");
        iterace = -1;
    }
    if (zadane_cislo == -1 || zadane_cislo > 1)
    {
        ui->pocetIteraci->setStyleSheet("color: #339900");
        if (zadane_cislo == -1)
            iterace = -1;
        else
            iterace = zadane_cislo;
    }
}
