#ifndef ERRORS_H
#define ERRORS_H
#include <QJsonArray>
#include <QString>

static QJsonArray hardErrorList = {
    "No *.ini file found. Can't load program settings.", //0
    "No *.json file with folders found. Can't load paths.", //1
    "Empty folder path cannot be added.", //2
    "Selected folder contains no *.avi files.", //3
    "Can't save frangi parameters - the input must be non zero numbers.", //4
    "Can't use frangi filter with zero or missing paramers.", //5
    "Video could not be opened.", //6
    "No file with Frangi parameters in this directory", //7
    "Error occured when calculating ET of referencial image", //8
    "Error occured when calculating ET of translated image", //9
    "Error occured when preprocessing frames before full registration", //10
    "Frames registration failed", //11
    "Video writer could not be loaded.", //12
    "Referencial frame could not be read", //13
    "","","","","","","",""
};

static QJsonArray softErrorList = {
    "*.ini file loaded successfully, but file folder path is empty.",//0
    "*.ini file path must be similar with the application path.", //1
    "An error occured when loading frangi json from this directory.", //2
    "All processed videos are unprocessable.", //3
};

static QJsonArray infoList = {
    "No *.dat file found for some chosen video. Use entropy and tennengrad analysis to add info about this video", //0
    "More than one *.json file found. Can't decide, which one is correct.", //1
    "Check the Frangi filter parameters if the values are suitable for the video(s).", //2
    "No *.dat file found for this video.", //3
};

static QJsonArray whatToDoList = {
    "Problem with HDD counter occured. Please use the Settings option \"Add counter name\" to enable HDD usage plot", //0
};
#endif // ERRORS_H
