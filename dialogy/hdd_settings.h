#ifndef HDD_SETTINGS_H
#define HDD_SETTINGS_H

#include <QDialog>

namespace Ui {
class HDD_Settings;
}

class HDD_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit HDD_Settings(QWidget *parent = nullptr);
    ~HDD_Settings();

private:
    Ui::HDD_Settings *ui;
};

#endif // HDD_SETTINGS_H
