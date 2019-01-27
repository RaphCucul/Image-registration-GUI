#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QJsonObject>
#include <opencv2/opencv.hpp>

#include "multithreadET/qThreadFirstPart.h"
#include "multithreadET/qthreadsecondpart.h"
#include "multithreadET/qthreadthirdpart.h"
#include "multithreadET/qthreadfourthpart.h"
#include "multithreadET/qthreadfifthpart.h"
#include "dialogy/errordialog.h"
#include "util/etanalysisparent.h"

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public ETanalysisParent
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    ~MultipleVideoET();
    void aktualizujProgBar(int procento);
    void checkPaths();
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_nekolikVideiPB_clicked();
    void on_celaSlozkaPB_clicked();
    void on_ETanalyzaVideiPB_clicked();
    void on_zobrazVysledkyPB_clicked();
    void on_vymazatZVyberuPB_clicked();
    void zpracovano(int dokonceno);
    void newVideoProcessed(int index);
    void movedToMethod(int metoda);
    void terminatedByError(int where);
    void on_ulozeni_clicked();
    void on_oblastMaxima_textChanged(const QString &arg1);
    void on_uhelRotace_textChanged(const QString &arg1);
    void on_pocetIteraci_textChanged(const QString &arg1);

private:
    Ui::MultipleVideoET *ui;
    QStringList videoList;
};

#endif // MULTIPLEVIDEOET_H
