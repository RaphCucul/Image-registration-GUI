#ifndef VERSIONCHECKERPARENT_H
#define VERSIONCHECKERPARENT_H

#include <QObject>
#include <QByteArray>

#include "util/versionchecker.h"

class VersionCheckerParent : public QObject
{
    Q_OBJECT
public:
    explicit VersionCheckerParent(QObject *parent = nullptr);
    virtual ~VersionCheckerParent();

signals:
    void analysed();
    void versionChecked(bool status);
public slots:
    void checkVersion();
    void noData();
    void initiateDownload();
private:
    VersionChecker* versionController = nullptr;
    QByteArray downloadedDataForAnalysis;
};

#endif // VERSIONCHECKERPARENT_H
