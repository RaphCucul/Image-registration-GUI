#ifndef SINGLEVIDEOLICOVANI_H
#define SINGLEVIDEOLICOVANI_H

#include "util/licovaniparent.h"
#include <QWidget>

namespace Ui {
class SingleVideoLicovani;
}

class SingleVideoLicovani : public LicovaniParent
{
    Q_OBJECT

public:
    explicit SingleVideoLicovani(QWidget *parent = nullptr);
    ~SingleVideoLicovani();

private slots:
    void on_vybraneVideo_textChanged(const QString &arg1);

    void on_vyberVidea_clicked();

    void on_slozkaParametruLE_textChanged(const QString &arg1);

    void on_slozkaParametruPB_clicked();

    void on_slicovatPB_clicked();

    void on_zobrazVysledekLicovaniPB_clicked();

    void on_grafickeHodnoceniPB_clicked();

    void on_ulozitPB_clicked();

private:
    Ui::SingleVideoLicovani *ui;
    QVector<QString> vybraneVideoLicovaniSingle;
    QVector<QString> vybranyJSONLicovaniSingle;
};

#endif // SINGLEVIDEOLICOVANI_H
