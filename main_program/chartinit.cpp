#include "chartinit.h"
#include "ui_chartinit.h"
#include "main_program/chartinit.h"
#include "dialogs/graphet_parent.h"
#include "util/files_folders_operations.h"
#include "main_program/registrationresult.h"

#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QMimeDatabase>
#include <QJsonObject>
#include <QDebug>

#include <opencv2/opencv.hpp>

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
    ui->pushButton->setEnabled(false);
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
        if (ui->datFiles->isChecked() && mime.inherits("text/plain")) {
            fileList.append(url.toLocalFile());
        }
        else if (ui->videoFiles->isChecked() && mime.inherits("video/x-msvideo")) {
            fileList.append(url.toLocalFile());
        }
    }

    qDebug()<<"Update video list: "<<fileList;
    ui->selectedFiles->addItems(fileList);
    if (!fileList.isEmpty()) {
        ui->pushButton->setEnabled(true);
    }
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
        int index = fileList.indexOf(item->text());
        delete ui->selectedFiles->takeItem(ui->selectedFiles->row(item));
        fileList.removeAt(index);
    }
    if (fileList.isEmpty())
        ui->pushButton->setEnabled(false);
}

void ChartInit::on_pushButton_clicked()
{
    if (ui->datFiles->isChecked()) {
        GraphET_parent* _parent = new GraphET_parent(fileList,this);
        _parent->setModal(true);
        _parent->show();
    }
    else if (ui->videoFiles->isChecked()) {
        foreach (QString video, fileList) {
            cv::VideoCapture _cap(video.toLocal8Bit().constData());
            if (_cap.isOpened()) {
                RegistrationResult* _result = new RegistrationResult(video);
                _result->callVideo();
                _result->setModal(true);
                _result->show();
            }
            _cap.release();
        }
    }
}
