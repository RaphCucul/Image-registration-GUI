#include "multiplevideoet.h"
#include "ui_multiplevideoet.h"
#include "util/souborove_operace.h"

#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDebug>

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
                seznamVidei.append(url.toLocalFile());

              }
       }
       sezVid = seznamVidei;
       qDebug()<<seznamVidei;
       ui->vybranaVidea->addItems(seznamVidei);
}

void MultipleVideoET::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
