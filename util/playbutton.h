#ifndef PLAYBUTTON_H
#define PLAYBUTTON_H

#include <QWidget>
#include <QPushButton>

#include "main_program/registrationresult.h"

/**
 * @class PlayButton
 * @brief The PlayButton class creates QPushButton class object with default icon.
 *
 * Objects of this class are used in the MultiVideoRegistration class to allow a user see the registration results
 * immediatelly when the video is successfully written. Each created object of this class is connected with a RegistrationResult
 * class object which can access a specific video when the button is clicked.
 * Push button cannot be activated until the video is written.
 */
class PlayButton : public QPushButton
{
    Q_OBJECT
public:
    /**
     * @brief Class constructor sets 32x32 icon for the button and connects the button with RegistrationResult class object using
     * given input path.
     * @param i_videoPath - full path to the video
     * @param parent
     */
    explicit PlayButton(QString i_videoPath,QWidget *parent = nullptr);

    enum struct ButtonMode{INACTIVE,ACTIVE};

signals:

private:
    ButtonMode mode = ButtonMode::INACTIVE;
    QString videoPath = "";
    RegistrationResult* result = nullptr;
};

#endif // PLAYBUTTON_H
