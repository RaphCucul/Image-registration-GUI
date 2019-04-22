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
TRANSLATIONS = appTranslation_CS.ts
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
    analyza_obrazu/entropie.cpp \
    analyza_obrazu/frangi.cpp \
    analyza_obrazu/korelacni_koeficient.cpp \
    analyza_obrazu/pouzij_frangiho.cpp \
    analyza_obrazu/upravy_obrazu.cpp \
###############################################
    dialogy/multiplevideoet.cpp \
    dialogy/clickimageevent.cpp \
    dialogy/errordialog.cpp \
    dialogy/grafet.cpp \
    dialogy/singlevideoet.cpp \
    dialogy/singlevideolicovani.cpp \
###############################################
    fancy_staff/qcustomplot.cpp \
    fancy_staff/systemmonitorwidget.cpp \
    fancy_staff/zasobnik_stranek.cpp \
###############################################
    hlavni_program/frangi_detektor.cpp \
    hlavni_program/licovanidvou.cpp \
    hlavni_program/licovanividea.cpp \
    hlavni_program/t_b_ho.cpp \
    hlavni_program/vysledeklicovani.cpp \
    hlavni_program/zalozky.cpp \
################################################
    licovani/fazova_korelace_funkce.cpp \
    licovani/korekce_zapis.cpp \
    licovani/multiPOC_Ai1.cpp \
    licovani/rozhodovaci_algoritmy.cpp \
################################################
    util/souborove_operace.cpp \
    util/systemmonitor.cpp \
    util/util_grafet.cpp \
    util/util_licovanividea.cpp \
    util/licovaniparent.cpp \
    util/vlaknolicovani.cpp \
    util/prace_s_vektory.cpp \
################################################
    vykon/cpuwidget.cpp \
    vykon/hddusageplot.cpp \
    vykon/hddwidget.cpp \
    vykon/memorywidget.cpp \
    vykon/windowsimpl.cpp \
################################################
    main.cpp \
    mainwindow.cpp \
    multithreadET/qThreadFirstPart.cpp \
    multithreadET/qthreadsecondpart.cpp \
    multithreadET/qthreadthirdpart.cpp \
    multithreadET/qthreadfourthpart.cpp \
    multithreadET/qthreadfifthpart.cpp \
    licovani/registrationthread.cpp \
    dialogy/multivideolicovani.cpp \
    fancy_staff/globalsettings.cpp \
    util/etanalysisparent.cpp \
    fancy_staff/sharedvariables.cpp
HEADERS += \
    analyza_obrazu/entropie.h \
    analyza_obrazu/frangi.h \
    analyza_obrazu/korelacni_koeficient.h \
    analyza_obrazu/pouzij_frangiho.h \
    analyza_obrazu/upravy_obrazu.h \
    dialogy/clickimageevent.h \
    dialogy/errordialog.h \
    dialogy/grafet.h \
    dialogy/multiplevideoet.h \
    dialogy/singlevideoet.h \
    fancy_staff/qcustomplot.h \
    fancy_staff/systemmonitorwidget.h \
    fancy_staff/zasobnik_stranek.h \
    hlavni_program/frangi_detektor.h \
    hlavni_program/licovanidvou.h \
    hlavni_program/licovanividea.h \
    hlavni_program/t_b_ho.h \
    hlavni_program/vysledeklicovani.h \
    hlavni_program/zalozky.h \
    licovani/fazova_korelace_funkce.h \
    licovani/korekce_zapis.h \
    licovani/multiPOC_Ai1.h \
    util/souborove_operace.h \
    util/systemmonitor.h \
    util/util_grafet.h \
    util/util_licovanividea.h \
    vykon/cpuwidget.h \
    vykon/hddusageplot.h \
    vykon/hddwidget.h \
    vykon/knihovny_pro_vykony.h \
    vykon/memorywidget.h \
    vykon/windowsimpl.h \
    mainwindow.h \
    util/licovaniparent.h \
    dialogy/singlevideolicovani.h \
    util/vlaknolicovani.h \
    licovani/rozhodovaci_algoritmy.h \
    util/prace_s_vektory.h \
    multithreadET/qThreadFirstPart.h \
    multithreadET/qthreadsecondpart.h \
    multithreadET/qthreadthirdpart.h \
    multithreadET/qthreadfourthpart.h \
    multithreadET/qthreadfifthpart.h \
    licovani/registrationthread.h \
    dialogy/multivideolicovani.h \
    fancy_staff/globalsettings.h \
    fancy_staff/errors.h \
    util/etanalysisparent.h \
    fancy_staff/sharedvariables.h
FORMS += \
    ui/t_b_ho.ui \
    ui/zalozky.ui \
    ui/frangi_detektor.ui \
    ui/licovanidvou.ui \
    ui/licovanividea.ui \
    ui/mainwindow.ui \
    dialogy/grafet.ui \
    dialogy/singlevideoet.ui \
    dialogy/multiplevideoet.ui \
    dialogy/errordialog.ui \
    ui/vysledeklicovani.ui \
    dialogy/singlevideolicovani.ui \
    dialogy/multivideolicovani.ui \
    dialogy/clickimageevent.ui
RESOURCES += \
    ikonky.qrc

INCLUDEPATH += C:\opencv_installed\build\include

LIBS += C:\Windows\System32\pdh.dll

LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_core320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_highgui320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_imgcodecs320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_imgproc320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_photo320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_videoio320.dll
LIBS += D:\Qt_projekty\opencv_build\install\x86\mingw\bin\libopencv_video320.dll
