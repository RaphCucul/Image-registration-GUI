#ifndef NEWVERSIONDIALOG_H
#define NEWVERSIONDIALOG_H

#include <QDialog>

namespace Ui {
class NewVersionDialog;
}

/**
 * @class NewVersionDialog
 * @brief The NewVersionDialog class is derived from the QDialog class.
 *
 * The dialog appears when the program detects there is
 * a new version available. An object of the NewVersionDialog class provides information about the actual version and the new
 * version of the program. If it is possible, release notes should be included too.
 */
class NewVersionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewVersionDialog(QString currentVersion, QString newVersion, QString descriptionText,
                              QWidget *parent = nullptr);
    ~NewVersionDialog();

private slots:
    /**
     * @brief Closes the dialog.
     */
    void on_confirmButton_clicked();

private:
    Ui::NewVersionDialog *ui;
};

#endif // NEWVERSIONDIALOG_H
