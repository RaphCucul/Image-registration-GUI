#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/**
 * @class VersionChecker class
 * @brief The VersionChecker class downloads data from the given URL and stores it for processing.
 */
class VersionChecker : public QObject
{
    Q_OBJECT
public:
    explicit VersionChecker(QUrl versionInfoURL, QObject *parent = nullptr);
    virtual ~VersionChecker();
    /**
     * @brief Returns information downloaded from the given URL.
     */
    QByteArray downloadedVersionInfo() const;
    /**
     * @brief Sends a QNetworkRequest object to given URL.
     */
    void startDownload();

signals:
    void downloaded();
    void notDownloaded();
public slots:
    /**
     * @brief Slot function called when the QNetworkAccessManager obtains a reply from the URL.
     * Processes the QNetworkReply object.
     * @param[in] pReply
     */
    void fileDownloaded(QNetworkReply* pReply);

private:
    QNetworkAccessManager webCtrl;
    QByteArray downloadedData;
    QUrl selectedAddress;
};

#endif // VERSIONCHECKER_H
