#ifndef CHARTINIT_H
#define CHARTINIT_H

#include <QWidget>

namespace Ui {
class ChartInit;
}

class ChartInit : public QWidget
{
    Q_OBJECT

public:
    explicit ChartInit(QWidget *parent = nullptr);
    ~ChartInit();
protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    void on_pushButton_clicked();

private:
    /**
     * @brief Function is overridden QWidget keyPressEvent function. The reaction on Delete key press
     * is handled here.
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief Helper function for keyPressEvent function handles the deletion of selected QListWidget
     * items.
     */
    void deleteSelectedFiles();
    Ui::ChartInit *ui;
    QStringList fileList;

};

#endif // CHARTINIT_H
