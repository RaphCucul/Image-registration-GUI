#ifndef REGISTRATEVIDEO_H
#define REGISTRATEVIDEO_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QString>
#include <QProgressBar>
#include <QResizeEvent>

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
    /**
     * @brief An operation started and the program has to block all tabs until it is finished.
     * @sa disableTabs()
     */
    void calculationStarted();
    /**
     * @brief An operation finished and the program has to unblock all tabs.
     * @sa enableTabs()
     */
    void calculationStopped();
    /**
     * @brief When this tab is clicked, the program emits this signal to inform all necessary objects that it is necessary
     * to check actual path to a video folder.
     *
     * SingleVideoET, MultipleVideoET, SingleVideoRegistration, MultiVideoRegistration objects are accessing the folder with
     * original videos and if the path is not available, the program has to react correctly. The signal invokes this folder control.
     */
    void checkFilePaths();
private slots:
    /**
     * @brief Enables tabs of all classes connected with this class.
     */
    void enableTabs();
    /**
     * @brief Disables tabs of all classes connected with this class.
     */
    void disableTabs();
private:
    void resizeEvent(QResizeEvent* e) override;
    Ui::RegistrateVideo *ui;
};

#endif // REGISTRATEVIDEO_H
