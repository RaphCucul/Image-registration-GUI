#include "versionchecker.h"

VersionChecker::VersionChecker(QUrl versionInfoURL, QObject *parent) : QObject(parent)
{
    selectedAddress = versionInfoURL;
    connect(&webCtrl, SIGNAL (finished(QNetworkReply*)),this, SLOT (fileDownloaded(QNetworkReply*)));
}

void VersionChecker::startDownload(){
    QNetworkRequest request(selectedAddress);
    webCtrl.get(request);
}

VersionChecker::~VersionChecker(){

}

void VersionChecker::fileDownloaded(QNetworkReply* pReply) {    
    if (pReply->bytesAvailable() && pReply->error() == QNetworkReply::NoError){
        downloadedData = pReply->readAll();
        pReply->deleteLater();
        emit downloaded();
    }
    else{
        //qDebug()<<"no data recieved";
        pReply->deleteLater();
        emit notDownloaded();
    }
}

QByteArray VersionChecker::downloadedVersionInfo() const {
 return downloadedData;
}
