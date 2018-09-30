#-------------------------------------------------
#
# Project created by QtCreator 2018-06-14T19:48:21
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Licovani_videa_GUI
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
    dialogy/t_b_ho.cpp \
    fancy_staff/kruhovyprogressbar.cpp \
    main.cpp \
    mainwindow.cpp \
    dialogy/zalozky.cpp \
    dialogy/frangi_detektor.cpp \
    util/frangi.cpp \
    util/upravy_obrazu.cpp \
    util/systemmonitor.cpp \
    fancy_staff/systemmonitorwidget.cpp \
    vykon/cpuwidget.cpp \
    vykon/hddwidget.cpp \
    vykon/memorywidget.cpp \
    vykon/windowsimpl.cpp \
    util/souborove_operace.cpp \
    util/pouzij_frangiho.cpp \
    dialogy/licovanidvou.cpp \
    licovani/multiPOC_Ai1.cpp \
    licovani/fazova_korelace_funkce.cpp \
    dialogy/errordialog.cpp \
    dialogy/clickimageevent.cpp \
    licovani/korekce_zapis.cpp \
    util/korelacni_koeficient.cpp \
    dialogy/licovanividea.cpp \
    qcustomplot.cpp \
    dialogy/vysledeklicovanidvou.cpp \
    util/entropie.cpp \
    vykon/hddusageplot.cpp \
    util/util_licovanividea.cpp \
    dialogy/grafet.cpp \
    util/util_grafet.cpp \
    util/vicevlaknovezpracovani.cpp \
    dialogy/singlevideoet.cpp \
    dialogy/multiplevideoet.cpp


HEADERS += \
    dialogy/t_b_ho.h \
    fancy_staff/kruhovyprogressbar.h \
    mainwindow.h \
    dialogy/zalozky.h \
    dialogy/frangi_detektor.h \
    util/frangi.h \
    util/upravy_obrazu.h \
    util/systemmonitor.h \
    fancy_staff/systemmonitorwidget.h \
    vykon/cpuwidget.h \
    vykon/hddwidget.h \
    vykon/memorywidget.h \
    vykon/knihovny_pro_vykony.h \
    vykon/windowsimpl.h \
    util/souborove_operace.h \
    util/pouzij_frangiho.h \
    dialogy/licovanidvou.h \
    licovani/multiPOC_Ai1.h \
    licovani/fazova_korelace_funkce.h \
    dialogy/errordialog.h \
    dialogy/clickimageevent.h \
    licovani/korekce_zapis.h \
    util/korelacni_koeficient.h \
    dialogy/licovanividea.h \
    qcustomplot.h \
    dialogy/vysledeklicovanidvou.h \
    util/entropie.h \
    vykon/hddusageplot.h \
    util/util_licovanividea.h \
    dialogy/grafet.h \
    util/util_grafet.h \
    util/vicevlaknovezpracovani.h \
    dialogy/singlevideoet.h \
    dialogy/multiplevideoet.h


FORMS += \
    ui/mainwindow.ui \
    ui/t_b_ho.ui \
    ui/zalozky.ui \
    ui/frangi_detektor.ui \
    ui/licovanidvou.ui \
    dialogy/errordialog.ui \
    dialogy/clickimageevent.ui \
    ui/licovanividea.ui \
    dialogy/vysledeklicovanidvou.ui \
    dialogy/grafet.ui \
    dialogy/singlevideoet.ui \
    dialogy/multiplevideoet.ui

RESOURCES += \
    ikonky.qrc

INCLUDEPATH += C:\opencv_installed\build\include
INCLUDEPATH += D:\Programy\Qt\5.9.1\mingw53_32\bin
LIBS += C:\Windows\System32\pdh.dll

LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_core320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_highgui320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_imgcodecs320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_imgproc320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_photo320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_videoio320.dll

