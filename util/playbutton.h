#ifndef PLAYBUTTON_H
#define PLAYBUTTON_H

#include <QWidget>
#include <QPushButton>

#include "main_program/registrationresult.h"

class PlayButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PlayButton(QString i_videoPath,QWidget *parent = nullptr);

    enum struct ButtonMode{INACTIVE,ACTIVE};

signals:

private:
    ButtonMode mode = ButtonMode::INACTIVE;
    QString videoPath = "";
    RegistrationResult* result = nullptr;
};

#endif // PLAYBUTTON_H
