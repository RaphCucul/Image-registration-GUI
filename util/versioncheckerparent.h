#ifndef VERSIONCHECKERPARENT_H
#define VERSIONCHECKERPARENT_H

#include <QObject>
#include <QByteArray>

#include "util/versionchecker.h"

/**
 * @class VersionCheckerParent
 * @brief The VersionCheckerParent class downloads specific data from the Github repository of this program
 * to determine if the current version is the latest. The class uses several helper classes/functions to achieve
 * the designated purpose. Each time a new version is released, corresponding file at Github repo should be changed.
 */
class VersionCheckerParent : public QObject
{
    Q_OBJECT
public:
    explicit VersionCheckerParent(QObject *parent = nullptr);
    virtual ~VersionCheckerParent();
    std::string getActualVersion();
signals:
    void analysed();
    void versionChecked(bool status);
public slots:
    /**
     * @brief It processes downloaded data. Information must have the same structure. When downloaded information is processed,
     * the progam version is compared to the downloaded data. If a new version is available, a dialog with information about this
     * new version is created.
     * Downloaded data is supposed to have a brief information about new version - fixes, bugs etc.
     * @sa NewVersionDialog
     */
    void checkVersion();
    /**
     * @brief Slot function is called when the download operation fails for some reason. Function creates QMessageBox instance,
     * type "Warning", informing a user about the problem.
     */
    void noData();
    /**
     * @brief It creates an instance of the VersionChecker class and starts the data download. According to the
     * download operation result, corresponding slot functions are called.
     * @sa checkVersion()
     * @sa noData()
     */
    void initiateDownload();
private:
    VersionChecker* versionController = nullptr;
    QByteArray downloadedDataForAnalysis;
};

#endif // VERSIONCHECKERPARENT_H
