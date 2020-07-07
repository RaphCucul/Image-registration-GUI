#ifndef HDD_SETTINGS_H
#define HDD_SETTINGS_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class HDD_Settings;
}

/**
 * @class HDD_Settings
 * @brief The HDD_Settings class provides user interface for HDD counter and parameter processing. If a user wants to add
 * HDD counter and parameter names translated to the system language, it is necessary to run Powershell script. This class calls
 * the necessary script when correct button is clicked.
 */
class HDD_Settings : public QDialog
{
    Q_OBJECT

public:
    /**
     * @enum IconType
     * @brief The IconType enum helps to identify which icon should be displayed in the dialogue.
     */
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
    /**
     * @brief Calls powershell script to identify HDD counter and parameter names. Dialogue icon is updated according to the
     * actual progress.
     */
    void on_pushButton_clicked();
    /**
     * @brief Scans the program folder where the ouput of the powershell script is created when the process ends successfully.
     * The interval of scanning is 1 second.
     */
    void scanFolder();

private:
    Ui::HDD_Settings *ui;
    /**
     * @brief Loads the icon of the corresponding icon type to the dialogue.
     * @param i_type
     */
    void setLabelIcon(IconType i_type);
    /**
     * @brief Loads the text describing the progress of the algortihm to the dialogue.
     * @param i_text
     */
    void setLabelText(QString i_text);

    QTimer* timer = nullptr;

    QStringList scriptResults;
};

#endif // HDD_SETTINGS_H
