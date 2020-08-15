#include "playbutton.h"

#include <QIcon>

PlayButton::PlayButton(QString i_videoPath,QWidget *parent) : QPushButton(parent),videoPath(i_videoPath)
{
    this->setIcon(QIcon(":/images/play.png"));
    this->setEnabled(true);
    this->setFocusPolicy(Qt::NoFocus);

    result = new RegistrationResult(videoPath);
    connect(this,&QPushButton::clicked,[=](){
        result->callVideo();
        result->setModal(true);
        result->show();
    });
}
