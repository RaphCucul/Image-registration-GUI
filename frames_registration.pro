#-------------------------------------------------
#
# Project created by QtCreator 2018-06-14T19:48:21
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport network

TARGET = frames_registration
TEMPLATE = app

CONFIG += c++14
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    image_analysis/entropy.cpp \
    image_analysis/frangi.cpp \
    image_analysis/correlation_coefficient.cpp \
    image_analysis/frangi_utilization.cpp \
    image_analysis/image_processing.cpp \
###############################################
    dialogs/graphet_parent.cpp \
    dialogs/hdd_settings.cpp \
    dialogs/multiplevideoet.cpp \
    dialogs/clickimageevent.cpp \
    dialogs/errordialog.cpp \
    dialogs/grafet.cpp \
    dialogs/singlevideoet.cpp \
    dialogs/multivideoregistration.cpp \
    dialogs/singlevideoregistration.cpp \
    dialogs/matviewer.cpp \
    dialogs/newversiondialog.cpp \
###############################################            
    shared_staff/qcustomplot.cpp \
    shared_staff/systemmonitorwidget.cpp \
    shared_staff/globalsettings.cpp \
    shared_staff/sharedvariables.cpp \
###############################################
    main_program/frangi_detektor.cpp \
    main_program/registratetwo.cpp \
    main_program/registratevideo.cpp \
    main_program/directories_loader.cpp \
    main_program/registrationresult.cpp \
    main_program/chartinit.cpp \
    main_program/tabs.cpp \
################################################
    registration/phase_correlation_function.cpp \
    registration/registration_correction.cpp \
    registration/multiPOC_Ai1.cpp \
    registration/registrationthread.cpp \
################################################
    util/aboutprogram.cpp \
    util/files_folders_operations.cpp \
    util/playbutton.cpp \
    util/systemmonitor.cpp \
    util/util_grafet.cpp \
    util/registrationparent.cpp \
    util/vector_operations.cpp \
    util/etanalysisparent.cpp \
################################################
    power/cpuwidget.cpp \
    power/hddusageplot.cpp \
    power/memorywidget.cpp \
    power/windowsimpl.cpp \
################################################
    main.cpp \
    mainwindow.cpp \
################################################
    multithreadET/qThreadFirstPart.cpp \
    multithreadET/qthreadsecondpart.cpp \
    multithreadET/qthreadthirdpart.cpp \
    multithreadET/qthreadfourthpart.cpp \
    multithreadET/qthreadfifthpart.cpp \
    util/versionchecker.cpp \
    util/versioncheckerparent.cpp

HEADERS += \
    image_analysis/entropy.h \
    image_analysis/frangi.h \
    image_analysis/correlation_coefficient.h \
    image_analysis/frangi_utilization.h \
    image_analysis/image_processing.h \
################################################
    dialogs/clickimageevent.h \
    dialogs/errordialog.h \
    dialogs/grafet.h \
    dialogs/graphet_parent.h \
    dialogs/hdd_settings.h \
    dialogs/multiplevideoet.h \
    dialogs/singlevideoet.h \
    dialogs/multivideoregistration.h\
    dialogs/singlevideoregistration.h \
    dialogs/newversiondialog.h \
    dialogs/matviewer.h \
################################################
    shared_staff/qcustomplot.h \
    shared_staff/systemmonitorwidget.h \
    shared_staff/globalsettings.h \
    shared_staff/errors.h \
    shared_staff/sharedvariables.h \
################################################
    main_program/frangi_detektor.h \
    main_program/registratetwo.h \
    main_program/registratevideo.h \
    main_program/registrationresult.h \
    main_program/directories_loader.h \
    main_program/chartinit.h \
    main_program/tabs.h \
################################################
    registration/phase_correlation_function.h \
    registration/registration_correction.h \
    registration/multiPOC_Ai1.h \
    registration/registrationthread.h \
################################################
    shared_staff/version.h \
    util/aboutprogram.h \
    util/adminCheck.h \
    util/files_folders_operations.h \
    util/playbutton.h \
    util/systemmonitor.h \
    util/util_grafet.h \
    util/registrationparent.h \
    util/vector_operations.h \
    util/etanalysisparent.h \
    util/versionchecker.h \
    util/versioncheckerparent.h \
################################################
    power/cpuwidget.h \
    power/hddusageplot.h \
    power/memorywidget.h \
    power/windowsimpl.h \
#################################################
    mainwindow.h \
#################################################
    multithreadET/qThreadFirstPart.h \
    multithreadET/qthreadsecondpart.h \
    multithreadET/qthreadthirdpart.h \
    multithreadET/qthreadfourthpart.h \
    multithreadET/qthreadfifthpart.h

FORMS += \
    dialogs/hdd_settings.ui \
    dialogs/newversiondialog.ui \
    dialogs/singlevideoregistration.ui \
    ui/graphet_parent.ui \
    ui/chartinit.ui \
    ui/registratetwo.ui \
    ui/registratevideo.ui \
    ui/registrationresult.ui \
    ui/directories_loader.ui \
    ui/frangi_detektor.ui \
    ui/mainwindow.ui \
    dialogs/grafet.ui \
    dialogs/singlevideoet.ui \
    dialogs/multiplevideoet.ui \
    ui/tabs.ui \
    dialogs/multivideoregistration.ui \
    dialogs/clickimageevent.ui \
    util/aboutprogram.ui

QMAKE_RESOURCE_FLAGS += -no-compress

LIBS += C:\Windows\System32\pdh.dll
LIBS += D:\Programy\OpenCV\opencv\release\install\x86\mingw\bin\libopencv_world320.dll
INCLUDEPATH += D:\Programy\OpenCV\opencv\release\install\include
INCLUDEPATH += D:\Programy\OpenCV\opencv\release\install\x86\mingw

RESOURCES += \
    images/icons.qrc \
    releaseNotes.qrc \
    translations.qrc

RC_FILE += images/mainIco.rc

TRANSLATIONS += translations/czech.ts translations/english.ts
