#ifndef NEWVERSIONDIALOG_H
#define NEWVERSIONDIALOG_H

#include <QDialog>

namespace Ui {
class NewVersionDialog;
}

class NewVersionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewVersionDialog(QString currentVersion, QString newVersion, QString descriptionText,
                              QWidget *parent = nullptr);
    ~NewVersionDialog();

private slots:
    void on_confirmButton_clicked();

private:
    Ui::NewVersionDialog *ui;
};

#endif // NEWVERSIONDIALOG_H
