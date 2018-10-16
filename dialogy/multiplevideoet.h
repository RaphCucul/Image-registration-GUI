#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include "util/vicevlaknovezpracovani.h"

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    ~MultipleVideoET();
    void aktualizujProgBar(int procento);
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_nekolikVideiPB_clicked();

    void on_celaSlozkaPB_clicked();

    void on_ETanalyzaVideiPB_clicked();

    void on_zobrazVysledkyPB_clicked();

    void on_vymazatZVyberuPB_clicked();
    void zpracovano();
    void on_pushButton_clicked();

private:
    Ui::MultipleVideoET *ui;
    QStringList sezVid;
    QVector<QVector<double>> entropie;
    QVector<QVector<double>> tennengrad;
    QVector<QString> videoNames;
    VicevlaknoveZpracovani* vlaknoET;
};

#endif // MULTIPLEVIDEOET_H
