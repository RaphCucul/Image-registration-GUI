#ifndef ERRORS_H
#define ERRORS_H
#include <QJsonArray>
#include <QString>

QJsonArray errorList = {"No *.ini file found. Can't load program settings.", //0
                        "No *.json file with folders found. Can't load paths.", //1
                        "Add paths into all categories. Save them to create new *.json file.", //2
                        "Selected folder contains no *.avi files.", //3
                        "Can't save frangi parameters - it must be non zero numbers.", //4
                        "Can't use frangi filter with zero or missing paramers.", //5
                        "Video can't be opened. Stopping calculations...", //6
                        "","","","","","",""};
#endif // ERRORS_H
