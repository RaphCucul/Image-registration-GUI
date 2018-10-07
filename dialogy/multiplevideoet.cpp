#include "multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "util/souborove_operace.h"
#include "util/entropie.h"
#include "dialogy/grafet.h"

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
        int AnalysisSuccess = entropie_tennengrad_videa(cap,entropyActual,tennengradActual,ui->progBar);
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
