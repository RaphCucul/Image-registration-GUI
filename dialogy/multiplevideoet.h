#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    ~MultipleVideoET();
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private:
    Ui::MultipleVideoET *ui;
    QStringList sezVid;
};

#endif // MULTIPLEVIDEOET_H
