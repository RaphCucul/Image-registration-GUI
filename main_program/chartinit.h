#ifndef CHARTINIT_H
#define CHARTINIT_H

#include <QWidget>

namespace Ui {
class ChartInit;
}

/**
 * @class ChartInit
 * @brief The ChartInit class is a simple QWidgets class where a user can place *.dat or *.avi files and load their content.
 *
 * If a *.dat file is selected, provided and loaded, GraphET dialogue is displayed with complete information about E+T analysis.
 * If a *.avi file is selected, provided and loaded, RegistrationResult dialogue is displayed. Because registration results are
 * saved into a video with *.avi suffix, the program supports only *.avi suffix of video files.
 */
class ChartInit : public QWidget
{
    Q_OBJECT

public:
    explicit ChartInit(QWidget *parent = nullptr);
    ~ChartInit();
protected:
    /**
     * @brief Overriden drop event ensures files of selected type are accepted.
     * @param event
     */
    void dropEvent(QDropEvent *event) override;
    /**
     * @brief Overriden drag event checks the data has mime format text/uri-list.
     * @param event
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
private slots:
    /**
     * @brief Based on the selected data suffix, the function tries to display data from loaded files when the button is clicked.
     */
    void on_showDataPB_clicked();

private:
    /**
     * @brief Overriden QWidget keyPressEvent function. The reaction on Delete key press
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
