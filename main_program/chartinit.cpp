#include "chartinit.h"
#include "ui_chartinit.h"
#include "main_program/chartinit.h"
#include "dialogs/graphet_parent.h"
#include "util/files_folders_operations.h"

#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QMimeDatabase>
#include <QJsonObject>
#include <QDebug>

ChartInit::ChartInit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartInit)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    installEventFilter(this);

    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);
}

ChartInit::~ChartInit()
{
    delete ui;
}

void ChartInit::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData->hasUrls()) {
        return;
    }
    QList<QUrl> urls = mimeData->urls();
    foreach (QUrl url,urls){
        QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
        if (mime.inherits("text/plain")) {
            fileList.append(url.toLocalFile());
        }
    }

    qDebug()<<"Update video list: "<<fileList;
    ui->selectedFiles->addItems(fileList);
}

void ChartInit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void ChartInit::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
        case Qt::Key_Delete:
            deleteSelectedFiles();
            break;
         default:
            QWidget::keyPressEvent(event);
    }
}

void ChartInit::deleteSelectedFiles(){
    QList<QListWidgetItem*> items = ui->selectedFiles->selectedItems();
    foreach(QListWidgetItem * item, items){
        delete ui->selectedFiles->takeItem(ui->selectedFiles->row(item));
    }
}

void ChartInit::on_pushButton_clicked()
{
    GraphET_parent* _parent = new GraphET_parent(fileList,this);
    _parent->setModal(true);
    _parent->show();
}
