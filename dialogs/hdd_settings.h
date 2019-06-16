#ifndef HDD_SETTINGS_H
#define HDD_SETTINGS_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class HDD_Settings;
}

class HDD_Settings : public QDialog
{
    Q_OBJECT

public:
    enum class IconType{
        GREY,
        ORANGE,
        GREEN,
        RED
    };
    explicit HDD_Settings(QWidget *parent = nullptr);
    ~HDD_Settings();
private slots:
    void slot_accepted();
    void slot_rejected();
    void on_pushButton_clicked();
    void scanFolder();

private:
    Ui::HDD_Settings *ui;

    void setLabelIcon(IconType i_type);
    void setLabelText(QString i_text);

    QTimer* timer = nullptr;

    QStringList scriptResults;
};

#endif // HDD_SETTINGS_H
