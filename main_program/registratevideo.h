#ifndef REGISTRATEVIDEO_H
#define REGISTRATEVIDEO_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QString>
#include <QProgressBar>
#include <QShowEvent>

namespace Ui {
class RegistrateVideo;
}

/**
 * @brief The RegistrateVideo class is included in tabs widget maintained by tabs class.
 * It provides a space for entropy+tennengrad video registration dialogs.
 */
class RegistrateVideo : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrateVideo(QWidget *parent = nullptr);
    ~RegistrateVideo() override;

    /**
     * @brief Calls checkPath() functions for single ET analysis and single registration.
     */
    void checkPathinitialization();
signals:
    void calculationStarted();
    void calculationStopped();
    void checkFilePaths();
private slots:
    void enableTabs();
    void disableTabs();
private:
    void showEvent(QShowEvent* e) override;
    Ui::RegistrateVideo *ui;
};

#endif // REGISTRATEVIDEO_H
