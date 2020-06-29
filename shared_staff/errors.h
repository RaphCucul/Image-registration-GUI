#ifndef ERRORS_H
#define ERRORS_H
#include <QJsonArray>
#include <QString>
#include <QObject>

/**
 * @file errors.h
 * The file contains all error, warning, info and whatToDo messages. These messages are used by errorDialog functions.
 */

static QJsonArray hardErrorList = {
    QObject::tr("No *.ini file found. Can't load program settings."), //0
    QObject::tr("No *.json file with folders found. Can't load paths."), //1
    QObject::tr("Empty folder path cannot be added."), //2
    QObject::tr("Selected folder contains no *.avi files."), //3
    QObject::tr("Can't save frangi parameters - the input must be non zero numbers."), //4
    QObject::tr("Can't use frangi filter with zero or missing paramers."), //5
    QObject::tr("Video could not be opened."), //6
    QObject::tr("No file with Frangi parameters in this directory"), //7
    QObject::tr("Error occured when calculating ET of referencial image"), //8
    QObject::tr("Error occured when calculating ET of translated image"), //9
    QObject::tr("Error occured when preprocessing frames before full registration"), //10
    QObject::tr("Frames registration failed"), //11
    QObject::tr("Video writer could not be loaded."), //12
    QObject::tr("Referencial frame could not be read"), //13
    QObject::tr("Frangi anaylsis failed"), //14
};

static QJsonArray softErrorList = {
    QObject::tr("*.ini file loaded successfully, but file folder path is empty."),//0
    QObject::tr("*.ini file path must be similar with the application path."), //1
    QObject::tr("An error occured when loading frangi json from this directory."), //2
    QObject::tr("All processed videos are unprocessable."), //3
};

static QJsonArray infoList = {
    QObject::tr("No *.dat file found for some chosen video. Use entropy and tennengrad analysis to add info about this video"), //0
    QObject::tr("More than one *.json file found. Can't decide, which one is correct."), //1
    QObject::tr("Check the Frangi filter parameters if the values are suitable for the video(s)."), //2
    QObject::tr("No *.dat file found for this video."), //3
    QObject::tr("This frame is not official referential frame"), //4
};

static QJsonArray whatToDoList = {
    QObject::tr("Problem with HDD counter occured. Please use the Settings option \"Add counter name\" to enable HDD usage plot.\n"
                "HDD monitoring can be turned off completely."), //0
    QObject::tr("Please write a referential frame number here."), //1
    QObject::tr("Choose video and type referential frame number"), //2
};
#endif // ERRORS_H
