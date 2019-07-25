#include "util/versioncheckerparent.h"
#include "shared_staff/globalsettings.h"
#include "shared_staff/version.h"
#include "dialogs/newversiondialog.h"

#include <QMessageBox>

#include <sstream>
#include <iostream>
#include <string>

std::string actualVersion = "1.0.0";

VersionCheckerParent::VersionCheckerParent(QObject *parent) : QObject(parent)
{
}

void VersionCheckerParent::initiateDownload(){
    QUrl url("https://raw.githubusercontent.com/RaphCucul/Frames-registration/master/versionInfo/version.txt");
    versionController = new VersionChecker(url);
    versionController->startDownload();
    QObject::connect(versionController,SIGNAL(downloaded()),this,SLOT(checkVersion()));
    QObject::connect(versionController,SIGNAL(notDownloaded()),this,SLOT(noData()));
}

VersionCheckerParent::~VersionCheckerParent(){

}

void VersionCheckerParent::noData(){
    //qDebug()<<"processing noData()";
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("An error occured when connecting to internet. Program version not checked.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    emit analysed();
}

void VersionCheckerParent::checkVersion(){
    downloadedDataForAnalysis = versionController->downloadedVersionInfo();
    const char* data = downloadedDataForAnalysis.data();
    std::stringstream ss;
    ss << data;
    std::string stringFull;
    std::string dotSearch(": ");
    std::string commaSearch(",");
    stringFull = ss.str();
    std::size_t foundVersion = stringFull.find(dotSearch);
    std::size_t foundComma = stringFull.find(commaSearch);
    std::size_t foundDescription = stringFull.find(dotSearch,foundComma+3);
    std::string version = stringFull.substr(foundVersion+2,foundComma-foundVersion-2);
    std::string description = stringFull.substr(foundDescription+2);
    if (Version(actualVersion) < Version(version)){
        NewVersionDialog* newVersionDialog = new NewVersionDialog(QString(actualVersion.c_str()),
                                                              QString(version.c_str()),
                                                              QString(description.c_str()));
        newVersionDialog->setModal(true);
        newVersionDialog->open();
        emit versionChecked(true);
    }
    else
        emit versionChecked(false);

    std::cout << version<<" "<<description<<"\n";
    emit analysed();
}
