#include "multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/entropie.h"
#include "dialogy/grafet.h"
#include "dialogy/clickimageevent.h"
#include "hlavni_program/frangi_detektor.h"
#include "hlavni_program/t_b_ho.h"

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
    for (int a = 0; a < sezVid.count(); a++)
    {
        QString fullPath = sezVid.at(a);
        qDebug()<<"Processing: "<<fullPath;
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> entropyActual,tennengradActual;
        entropyActual.fill(0.0,frameCount);
        tennengradActual.fill(0.0,frameCount);
        /*vlaknoET = new VicevlaknoveZpracovani(sezVid,ziskane_hranice_anomalie,
                                              ziskane_hranice_casZnac,parametry_frangi,
                                              volbaSvetAnomETSingle,volbaCasZnackyETSingle);
        connect(vlaknoET,SIGNAL(percentageCompleted(int)),ui->progBar,SLOT(setValue(int)));
        connect(vlaknoET,SIGNAL(hotovo()),this,SLOT(zpracovano()));
        vlaknoET->start();*/
        /*QFuture<int> future = QtConcurrent::run(entropie_tennengrad_videa,this,cap,entropyActual,tennengradActual,
                                                ui->progBar);
        int AnalysisSuccess = future.result();*/
        /*int AnalysisSuccess = entropie_tennengrad_videa(cap,entropyActual,tennengradActual,ui->progBar);
        if (AnalysisSuccess == 0)
            qDebug()<<"Error occured when calculating entropy and tennengrad!";
        else
        {
            entropie.push_back(entropyActual);
            tennengrad.push_back(tennengradActual);
            QString onlyFolder,onlyVideoName,onlySuffix;
            zpracujJmeno(fullPath,onlyFolder,onlyVideoName,onlySuffix);
            videoNames.push_back(onlyVideoName);
        }
    }*/
  }
}

void MultipleVideoET::on_zobrazVysledkyPB_clicked()
{
    /*GrafET* graf_ET = new GrafET(entropie,tennengrad,snimkyPrvotniOhodnoceniEntropieKomplet,
                                 snimkyPrvotniOhodnoceniTennengradKomplet,snimkyPrvniRozhodovaniKomplet,
                                 snimkyDruheRozhodovaniKomplet,sezVid,this);
    graf_ET->setModal(true);
    graf_ET->show();*/
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

void MultipleVideoET::aktualizujProgBar(int procento)
{
    ui->progBar->setValue(procento);
}

void MultipleVideoET::on_ulozeni_clicked()
{

    for (int a = 0; a < sezVid.count(); a++)
    {
        QJsonDocument document;
        QJsonObject object;
        QVector<double> pomVecE = entropie[a];
        QVector<double> pomVecT = tennengrad[a];
        QVector<double> pomVecPOE = snimkyPrvotniOhodnoceniEntropieKomplet[a];
        QVector<double> pomVecPOT = snimkyPrvotniOhodnoceniTennengradKomplet[a];
        QVector<double> pomVecPR = snimkyPrvniRozhodovaniKomplet[a];
        QVector<double> pomVecDR = snimkyDruheRozhodovaniKomplet[a];
        QVector<double> pomVecO = snimkyOhodnoceniKomplet[a];
        QVector<double> pomVecFX = snimkyFrangiX[a];
        QVector<double> pomVecFY = snimkyFrangiY[a];
        QVector<double> pomVecFE = snimkyFrangiEuklid[a];
        QVector<double> pomVecPX = snimkyPOCX[a];
        QVector<double> pomVecPY = snimkyPOCY[a];
        QVector<double> pomVecU = snimkyUhel[a];
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
        QString cesta,jmeno,koncovka;
        QString kompletniCesta = sezVid.at(a);
        zpracujJmeno(kompletniCesta,cesta,jmeno,koncovka);
        QString cestaUlozeni = TXTulozeniAktual+"/"+jmeno+".dat";
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
        zapis.setFileName(cestaUlozeni);
        zapis.open(QIODevice::WriteOnly);
        zapis.write(documentString.toLocal8Bit());
        zapis.close();
    }
}

void MultipleVideoET::zpracovano()
{
    /*entropie = vlaknoET->vypocitanaEntropie();
    tennengrad = vlaknoET->vypocitanyTennengrad();
    snimkyPrvotniOhodnoceniEntropieKomplet = vlaknoET->vypocitanePrvotniOhodnoceniEntropie();
    snimkyPrvotniOhodnoceniTennengradKomplet = vlaknoET->vypocitanePrvotniOhodnoceniTennengrad();
    snimkyPrvniRozhodovaniKomplet = vlaknoET->vypocitanePrvniRozhodnuti();
    snimkyDruheRozhodovaniKomplet = vlaknoET->vypocitaneDruheRozhodnuti();
    snimkyOhodnoceniKomplet = vlaknoET->vypocitaneOhodnoceni();
    snimkyFrangiX = vlaknoET->vypocitaneFrangiX();
    snimkyFrangiY = vlaknoET->vypocitaneFrangiY();
    snimkyFrangiEuklid = vlaknoET->vypocitaneFrangiEuklid();
    snimkyPOCX = vlaknoET->vypocitanePOCX();
    snimkyPOCY = vlaknoET->vypocitanePOCY();
    snimkyUhel = vlaknoET->vypocitanyUhel();
    vlaknoET->quit();
    ui->zobrazVysledkyPB->setEnabled(true);
    ui->ulozeni->setEnabled(true);*/
}
