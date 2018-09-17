#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialogy/t_b_ho.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setupUsagePlots();
private slots:
    void updateWidget();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
