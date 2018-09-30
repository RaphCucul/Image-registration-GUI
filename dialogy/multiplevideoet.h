#ifndef MULTIPLEVIDEOET_H
#define MULTIPLEVIDEOET_H

#include <QWidget>

namespace Ui {
class MultipleVideoET;
}

class MultipleVideoET : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleVideoET(QWidget *parent = nullptr);
    ~MultipleVideoET();

private:
    Ui::MultipleVideoET *ui;
};

#endif // MULTIPLEVIDEOET_H
