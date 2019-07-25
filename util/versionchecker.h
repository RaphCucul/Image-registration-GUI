#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class VersionChecker : public QObject
{
    Q_OBJECT
public:
    explicit VersionChecker(QUrl versionInfoURL, QObject *parent = nullptr);
    virtual ~VersionChecker();
    QByteArray downloadedVersionInfo() const;
    void startDownload();

signals:
    void downloaded();
    void notDownloaded();
public slots:

    void fileDownloaded(QNetworkReply* pReply);

private:
    QNetworkAccessManager webCtrl;
    QNetworkReply* reply;
    QByteArray downloadedData;
    QUrl selectedAddress;
};

#endif // VERSIONCHECKER_H
