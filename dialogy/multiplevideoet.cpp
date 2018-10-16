#include "multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "util/souborove_operace.h"
#include "analyza_obrazu/entropie.h"
#include "dialogy/grafet.h"
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
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Choose avi files"),QDir::currentPath(),tr("Video files (*.avi);;;") );
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
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/home",
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
        vlaknoET = new VicevlaknoveZpracovani(cap,entropyActual,tennengradActual,1);
        connect(vlaknoET,SIGNAL(percentageCompleted(int)),ui->progBar,SLOT(setValue(int)));
        connect(vlaknoET,SIGNAL(hotovo()),this,SLOT(zpracovano()));
        vlaknoET->start();
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
        }*/
    }
}

void MultipleVideoET::on_zobrazVysledkyPB_clicked()
{
    GrafET* graf_ET = new GrafET(entropie,tennengrad,videoNames,this);
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

void MultipleVideoET::aktualizujProgBar(int procento)
{
    ui->progBar->setValue(procento);
}

void MultipleVideoET::on_pushButton_clicked()
{

    for (int a = 0; a < videoNames.length(); a++)
    {
        QJsonObject object;
        QVector<double> pomVecE = entropie[a];
        QVector<double> pomVecT = tennengrad[a];
        QJsonArray poleE = vector2array(pomVecE);
        QJsonArray poleT = vector2array(pomVecT);
        QString aktualJmeno = videoNames[a];
        QString cesta = TXTulozeniAktual+"/"+aktualJmeno+".dat";
        object["entropie"] = poleE;
        object["tennengrad"] = poleT;
        QJsonDocument document;
        document.setObject(object);
        QString documentString = document.toJson();
        QFile zapis;
        zapis.setFileName(cesta);
        zapis.open(QIODevice::WriteOnly);
        zapis.write(documentString.toLocal8Bit());
        zapis.close();
    }
}

void MultipleVideoET::zpracovano()
{
    QVector<double> pomE = vlaknoET->vypocitanaEntropie();
    QVector<double> pomT = vlaknoET->vypocitanyTennengrad();
    entropie.push_back(pomE);
    tennengrad.push_back(pomT);
    ui->zobrazVysledkyPB->setEnabled(true);
}
