#ifndef ERRORS_H
#define ERRORS_H
#include <QJsonArray>
#include <QString>

static QJsonArray errorList = {
                        "No *.ini file found. Can't load program settings.", //0
                        "No *.json file with folders found. Can't load paths.", //1
                        "Empty folder path cannot be added.", //2
                        "Selected folder contains no *.avi files.", //3
                        "Can't save frangi parameters - it must be non zero numbers.", //4
                        "Can't use frangi filter with zero or missing paramers.", //5
                        "Video can't be opened. Stopping calculations...", //6
                        "No file with Frangi parameters in this directory", //7
                        "Error occured when calculating ET of referencial image", //8
                        "Error occured when calculating ET of translated image", //9
                        "Error occured when preprocessing frames before full registration", //10
                        "Frames registration failed", //11
                        "",""
                        };
#endif // ERRORS_H
