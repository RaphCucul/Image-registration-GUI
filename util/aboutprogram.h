#ifndef ABOUTPROGRAM_H
#define ABOUTPROGRAM_H

#include <QDialog>

namespace Ui {
class AboutProgram;
}

/**
 * @class AboutProgram
 * @brief The AboutProgram class is a simple QDialog implementation displaying release notes of the current version.
 */
class AboutProgram : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief The class construstructor loads data from release notes file and displays a dialogue with the text.
     * @param i_version - current program version
     * @param i_language - current app language
     * @param parent - parent widget of the AboutProgram class object
     */
    explicit AboutProgram(QString i_version, QString i_language, QWidget *parent = nullptr);
    ~AboutProgram();

private:
    Ui::AboutProgram *ui;
};

#endif // ABOUTPROGRAM_H
