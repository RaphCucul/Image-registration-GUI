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
    hlavni_program/vysledeklicovanidvou.cpp \
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
    licovani/registrationthread.cpp
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
    hlavni_program/vysledeklicovanidvou.h \
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
    licovani/registrationthread.h
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
    ui/vysledeklicovanidvou.ui \
    dialogy/singlevideolicovani.ui
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

SUBDIRS += \
    Licovani_videa_GUI.pro \
    qcustomplot/examples/axis-tags/axis-tags-example.pro \
    qcustomplot/examples/interactions/interaction-example.pro \
    qcustomplot/examples/plots/plot-examples.pro \
    qcustomplot/examples/scrollbar-axis-range-control/scrollbar-axis-range-control.pro \
    qcustomplot/examples/text-document-integration/text-document-integration.pro \
    Licovani_videa_GUI.pro

DISTFILES += \
    images/adresar.png \
    images/frangi.png \
    images/if_kfm_18009.png \
    images/image.png \
    images/licovaniDvou.png \
    images/video-camera-icon.png \
    images/Frangi.ico \
    images/video-camera-icon.ico \
    qcustomplot/documentation/html/dynsections.js \
    qcustomplot/documentation/html/jquery.js \
    qcustomplot/documentation/qcustomplot.qch \
    qcustomplot/examples/plots/balboa.jpg \
    qcustomplot/examples/plots/solarpanels.jpg \
    images/adresar.png \
    images/frangi.png \
    images/if_kfm_18009.png \
    images/image.png \
    images/licovaniDvou.png \
    images/video-camera-icon.png \
    qcustomplot/documentation/html/adaptive-sampling-line.png \
    qcustomplot/documentation/html/adaptive-sampling-scatter.png \
    qcustomplot/documentation/html/AxisNamesOverview.png \
    qcustomplot/documentation/html/AxisRectSpacingOverview.png \
    qcustomplot/documentation/html/axisticker-datetime.png \
    qcustomplot/documentation/html/axisticker-fixed.png \
    qcustomplot/documentation/html/axisticker-log-powers.png \
    qcustomplot/documentation/html/axisticker-log.png \
    qcustomplot/documentation/html/axisticker-pi.png \
    qcustomplot/documentation/html/axisticker-text.png \
    qcustomplot/documentation/html/axisticker-time.png \
    qcustomplot/documentation/html/axisticker-time2.png \
    qcustomplot/documentation/html/bc_s.png \
    qcustomplot/documentation/html/bdwn.png \
    qcustomplot/documentation/html/classQCPAbstractItem__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAbstractLegendItem__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAbstractPaintBuffer__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAbstractPlottable1D__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAbstractPlottable__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxis__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisRect__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTicker__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTickerDateTime__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTickerFixed__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTickerLog__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTickerPi__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTickerText__inherit__graph.png \
    qcustomplot/documentation/html/classQCPAxisTickerTime__inherit__graph.png \
    qcustomplot/documentation/html/classQCPBars__inherit__graph.png \
    qcustomplot/documentation/html/classQCPColorMap__inherit__graph.png \
    qcustomplot/documentation/html/classQCPColorScale__inherit__graph.png \
    qcustomplot/documentation/html/classQCPCurve__inherit__graph.png \
    qcustomplot/documentation/html/classQCPErrorBars__inherit__graph.png \
    qcustomplot/documentation/html/classQCPFinancial__inherit__graph.png \
    qcustomplot/documentation/html/classQCPGraph__inherit__graph.png \
    qcustomplot/documentation/html/classQCPGrid__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemAnchor__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemBracket__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemCurve__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemEllipse__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemLine__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemPixmap__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemPosition__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemRect__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemStraightLine__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemText__inherit__graph.png \
    qcustomplot/documentation/html/classQCPItemTracer__inherit__graph.png \
    qcustomplot/documentation/html/classQCPLayerable__inherit__graph.png \
    qcustomplot/documentation/html/classQCPLayout__inherit__graph.png \
    qcustomplot/documentation/html/classQCPLayoutElement__inherit__graph.png \
    qcustomplot/documentation/html/classQCPLayoutGrid__inherit__graph.png \
    qcustomplot/documentation/html/classQCPLayoutInset__inherit__graph.png \
    qcustomplot/documentation/html/classQCPLegend__inherit__graph.png \
    qcustomplot/documentation/html/classQCPPaintBufferGlFbo__inherit__graph.png \
    qcustomplot/documentation/html/classQCPPaintBufferGlPbuffer__inherit__graph.png \
    qcustomplot/documentation/html/classQCPPaintBufferPixmap__inherit__graph.png \
    qcustomplot/documentation/html/classQCPPlottableInterface1D__inherit__graph.png \
    qcustomplot/documentation/html/classQCPPlottableLegendItem__inherit__graph.png \
    qcustomplot/documentation/html/classQCPSelectionDecorator__inherit__graph.png \
    qcustomplot/documentation/html/classQCPSelectionDecoratorBracket__inherit__graph.png \
    qcustomplot/documentation/html/classQCPSelectionRect__inherit__graph.png \
    qcustomplot/documentation/html/classQCPStatisticalBox__inherit__graph.png \
    qcustomplot/documentation/html/classQCPTextElement__inherit__graph.png \
    qcustomplot/documentation/html/closed.png \
    qcustomplot/documentation/html/doc.png \
    qcustomplot/documentation/html/doxygen.png \
    qcustomplot/documentation/html/folderclosed.png \
    qcustomplot/documentation/html/folderopen.png \
    qcustomplot/documentation/html/inherit_graph_0.png \
    qcustomplot/documentation/html/inherit_graph_1.png \
    qcustomplot/documentation/html/inherit_graph_10.png \
    qcustomplot/documentation/html/inherit_graph_11.png \
    qcustomplot/documentation/html/inherit_graph_12.png \
    qcustomplot/documentation/html/inherit_graph_13.png \
    qcustomplot/documentation/html/inherit_graph_14.png \
    qcustomplot/documentation/html/inherit_graph_15.png \
    qcustomplot/documentation/html/inherit_graph_16.png \
    qcustomplot/documentation/html/inherit_graph_17.png \
    qcustomplot/documentation/html/inherit_graph_18.png \
    qcustomplot/documentation/html/inherit_graph_19.png \
    qcustomplot/documentation/html/inherit_graph_2.png \
    qcustomplot/documentation/html/inherit_graph_20.png \
    qcustomplot/documentation/html/inherit_graph_21.png \
    qcustomplot/documentation/html/inherit_graph_22.png \
    qcustomplot/documentation/html/inherit_graph_23.png \
    qcustomplot/documentation/html/inherit_graph_24.png \
    qcustomplot/documentation/html/inherit_graph_3.png \
    qcustomplot/documentation/html/inherit_graph_4.png \
    qcustomplot/documentation/html/inherit_graph_5.png \
    qcustomplot/documentation/html/inherit_graph_6.png \
    qcustomplot/documentation/html/inherit_graph_7.png \
    qcustomplot/documentation/html/inherit_graph_8.png \
    qcustomplot/documentation/html/inherit_graph_9.png \
    qcustomplot/documentation/html/InheritanceOverview.png \
    qcustomplot/documentation/html/layoutsystem-addinglegendtitle.png \
    qcustomplot/documentation/html/layoutsystem-addingplottitle.png \
    qcustomplot/documentation/html/layoutsystem-movinglegend.png \
    qcustomplot/documentation/html/layoutsystem-multipleaxisrects.png \
    qcustomplot/documentation/html/LayoutsystemSketch.png \
    qcustomplot/documentation/html/nav_f.png \
    qcustomplot/documentation/html/nav_g.png \
    qcustomplot/documentation/html/nav_h.png \
    qcustomplot/documentation/html/open.png \
    qcustomplot/documentation/html/qcp-doc-logo.png \
    qcustomplot/documentation/html/QCPBars.png \
    qcustomplot/documentation/html/QCPBarsGroup.png \
    qcustomplot/documentation/html/QCPColorGradient-levelcount.png \
    qcustomplot/documentation/html/QCPColorGradient-periodic.png \
    qcustomplot/documentation/html/QCPColorGradient.png \
    qcustomplot/documentation/html/QCPColorMap-interpolate.png \
    qcustomplot/documentation/html/QCPColorMap-tightboundary.png \
    qcustomplot/documentation/html/QCPColorMap.png \
    qcustomplot/documentation/html/QCPColorScale.png \
    qcustomplot/documentation/html/QCPCurve.png \
    qcustomplot/documentation/html/QCPErrorBars.png \
    qcustomplot/documentation/html/QCPFinancial.png \
    qcustomplot/documentation/html/QCPGraph.png \
    qcustomplot/documentation/html/QCPItemBracket-length.png \
    qcustomplot/documentation/html/QCPItemBracket.png \
    qcustomplot/documentation/html/QCPItemCurve.png \
    qcustomplot/documentation/html/QCPItemEllipse.png \
    qcustomplot/documentation/html/QCPItemLine.png \
    qcustomplot/documentation/html/QCPItemPixmap.png \
    qcustomplot/documentation/html/QCPItemRect.png \
    qcustomplot/documentation/html/QCPItemStraightLine.png \
    qcustomplot/documentation/html/QCPItemText.png \
    qcustomplot/documentation/html/QCPItemTracer.png \
    qcustomplot/documentation/html/QCPLineEnding.png \
    qcustomplot/documentation/html/QCPMarginGroup.png \
    qcustomplot/documentation/html/QCPStatisticalBox.png \
    qcustomplot/documentation/html/RelationOverview.png \
    qcustomplot/documentation/html/selectiontype-datarange.png \
    qcustomplot/documentation/html/selectiontype-multipledataranges.png \
    qcustomplot/documentation/html/selectiontype-none.png \
    qcustomplot/documentation/html/selectiontype-singledata.png \
    qcustomplot/documentation/html/selectiontype-whole.png \
    qcustomplot/documentation/html/splitbar.png \
    qcustomplot/documentation/html/ssCircle.png \
    qcustomplot/documentation/html/ssCross.png \
    qcustomplot/documentation/html/ssCrossCircle.png \
    qcustomplot/documentation/html/ssCrossSquare.png \
    qcustomplot/documentation/html/ssDiamond.png \
    qcustomplot/documentation/html/ssDisc.png \
    qcustomplot/documentation/html/ssDot.png \
    qcustomplot/documentation/html/ssPeace.png \
    qcustomplot/documentation/html/ssPlus.png \
    qcustomplot/documentation/html/ssPlusCircle.png \
    qcustomplot/documentation/html/ssPlusSquare.png \
    qcustomplot/documentation/html/ssSquare.png \
    qcustomplot/documentation/html/ssStar.png \
    qcustomplot/documentation/html/ssTriangle.png \
    qcustomplot/documentation/html/ssTriangleInverted.png \
    qcustomplot/documentation/html/sync_off.png \
    qcustomplot/documentation/html/sync_on.png \
    qcustomplot/documentation/html/tab_a.png \
    qcustomplot/documentation/html/tab_b.png \
    qcustomplot/documentation/html/tab_h.png \
    qcustomplot/documentation/html/tab_s.png \
    qcustomplot/examples/plots/sun.png \
    images/Frangi.ico \
    images/video-camera-icon.ico \
    qcustomplot/documentation/html/qcp.css \
    qcustomplot/documentation/html/tabs.css \
    qcustomplot/documentation/html/annotated.html \
    qcustomplot/documentation/html/classes.html \
    qcustomplot/documentation/html/classoverview.html \
    qcustomplot/documentation/html/classQCPAbstractItem.html \
    qcustomplot/documentation/html/classQCPAbstractLegendItem.html \
    qcustomplot/documentation/html/classQCPAbstractPaintBuffer.html \
    qcustomplot/documentation/html/classQCPAbstractPlottable.html \
    qcustomplot/documentation/html/classQCPAbstractPlottable1D.html \
    qcustomplot/documentation/html/classQCPAxis.html \
    qcustomplot/documentation/html/classQCPAxisRect.html \
    qcustomplot/documentation/html/classQCPAxisTicker.html \
    qcustomplot/documentation/html/classQCPAxisTickerDateTime.html \
    qcustomplot/documentation/html/classQCPAxisTickerFixed.html \
    qcustomplot/documentation/html/classQCPAxisTickerLog.html \
    qcustomplot/documentation/html/classQCPAxisTickerPi.html \
    qcustomplot/documentation/html/classQCPAxisTickerText.html \
    qcustomplot/documentation/html/classQCPAxisTickerTime.html \
    qcustomplot/documentation/html/classQCPBars.html \
    qcustomplot/documentation/html/classQCPBarsData.html \
    qcustomplot/documentation/html/classQCPBarsGroup.html \
    qcustomplot/documentation/html/classQCPColorGradient.html \
    qcustomplot/documentation/html/classQCPColorMap.html \
    qcustomplot/documentation/html/classQCPColorMapData.html \
    qcustomplot/documentation/html/classQCPColorScale.html \
    qcustomplot/documentation/html/classQCPCurve.html \
    qcustomplot/documentation/html/classQCPCurveData.html \
    qcustomplot/documentation/html/classQCPDataContainer.html \
    qcustomplot/documentation/html/classQCPDataRange.html \
    qcustomplot/documentation/html/classQCPDataSelection.html \
    qcustomplot/documentation/html/classQCPErrorBars.html \
    qcustomplot/documentation/html/classQCPErrorBarsData.html \
    qcustomplot/documentation/html/classQCPFinancial.html \
    qcustomplot/documentation/html/classQCPFinancialData.html \
    qcustomplot/documentation/html/classQCPGraph.html \
    qcustomplot/documentation/html/classQCPGraphData.html \
    qcustomplot/documentation/html/classQCPGrid.html \
    qcustomplot/documentation/html/classQCPItemAnchor.html \
    qcustomplot/documentation/html/classQCPItemBracket.html \
    qcustomplot/documentation/html/classQCPItemCurve.html \
    qcustomplot/documentation/html/classQCPItemEllipse.html \
    qcustomplot/documentation/html/classQCPItemLine.html \
    qcustomplot/documentation/html/classQCPItemPixmap.html \
    qcustomplot/documentation/html/classQCPItemPosition.html \
    qcustomplot/documentation/html/classQCPItemRect.html \
    qcustomplot/documentation/html/classQCPItemStraightLine.html \
    qcustomplot/documentation/html/classQCPItemText.html \
    qcustomplot/documentation/html/classQCPItemTracer.html \
    qcustomplot/documentation/html/classQCPLayer.html \
    qcustomplot/documentation/html/classQCPLayerable.html \
    qcustomplot/documentation/html/classQCPLayout.html \
    qcustomplot/documentation/html/classQCPLayoutElement.html \
    qcustomplot/documentation/html/classQCPLayoutGrid.html \
    qcustomplot/documentation/html/classQCPLayoutInset.html \
    qcustomplot/documentation/html/classQCPLegend.html \
    qcustomplot/documentation/html/classQCPLineEnding.html \
    qcustomplot/documentation/html/classQCPMarginGroup.html \
    qcustomplot/documentation/html/classQCPPaintBufferGlFbo.html \
    qcustomplot/documentation/html/classQCPPaintBufferGlPbuffer.html \
    qcustomplot/documentation/html/classQCPPaintBufferPixmap.html \
    qcustomplot/documentation/html/classQCPPainter.html \
    qcustomplot/documentation/html/classQCPPlottableInterface1D.html \
    qcustomplot/documentation/html/classQCPPlottableLegendItem.html \
    qcustomplot/documentation/html/classQCPRange.html \
    qcustomplot/documentation/html/classQCPScatterStyle.html \
    qcustomplot/documentation/html/classQCPSelectionDecorator.html \
    qcustomplot/documentation/html/classQCPSelectionDecoratorBracket.html \
    qcustomplot/documentation/html/classQCPSelectionRect.html \
    qcustomplot/documentation/html/classQCPStatisticalBox.html \
    qcustomplot/documentation/html/classQCPStatisticalBoxData.html \
    qcustomplot/documentation/html/classQCPTextElement.html \
    qcustomplot/documentation/html/classQCPVector2D.html \
    qcustomplot/documentation/html/classQCustomPlot.html \
    qcustomplot/documentation/html/core_8cpp.html \
    qcustomplot/documentation/html/datacontainer_8h.html \
    qcustomplot/documentation/html/dataselection.html \
    qcustomplot/documentation/html/dir_17cee08e17148530a7aedf85f92ee362.html \
    qcustomplot/documentation/html/dir_4749ca9c363ee6e69b445a1c06361cef.html \
    qcustomplot/documentation/html/dir_68267d1309a1af8e8297ef4c3efbcdba.html \
    qcustomplot/documentation/html/dir_78a97038723513eb3e3a9e944a7f482f.html \
    qcustomplot/documentation/html/dir_d1ab82e73f77cddd99808c625b504457.html \
    qcustomplot/documentation/html/files.html \
    qcustomplot/documentation/html/functions.html \
    qcustomplot/documentation/html/functions_b.html \
    qcustomplot/documentation/html/functions_c.html \
    qcustomplot/documentation/html/functions_d.html \
    qcustomplot/documentation/html/functions_e.html \
    qcustomplot/documentation/html/functions_enum.html \
    qcustomplot/documentation/html/functions_eval.html \
    qcustomplot/documentation/html/functions_f.html \
    qcustomplot/documentation/html/functions_func.html \
    qcustomplot/documentation/html/functions_func_b.html \
    qcustomplot/documentation/html/functions_func_c.html \
    qcustomplot/documentation/html/functions_func_d.html \
    qcustomplot/documentation/html/functions_func_e.html \
    qcustomplot/documentation/html/functions_func_f.html \
    qcustomplot/documentation/html/functions_func_g.html \
    qcustomplot/documentation/html/functions_func_h.html \
    qcustomplot/documentation/html/functions_func_i.html \
    qcustomplot/documentation/html/functions_func_k.html \
    qcustomplot/documentation/html/functions_func_l.html \
    qcustomplot/documentation/html/functions_func_m.html \
    qcustomplot/documentation/html/functions_func_n.html \
    qcustomplot/documentation/html/functions_func_o.html \
    qcustomplot/documentation/html/functions_func_p.html \
    qcustomplot/documentation/html/functions_func_q.html \
    qcustomplot/documentation/html/functions_func_r.html \
    qcustomplot/documentation/html/functions_func_s.html \
    qcustomplot/documentation/html/functions_func_t.html \
    qcustomplot/documentation/html/functions_func_u.html \
    qcustomplot/documentation/html/functions_func_v.html \
    qcustomplot/documentation/html/functions_func_w.html \
    qcustomplot/documentation/html/functions_func_z.html \
    qcustomplot/documentation/html/functions_g.html \
    qcustomplot/documentation/html/functions_h.html \
    qcustomplot/documentation/html/functions_i.html \
    qcustomplot/documentation/html/functions_k.html \
    qcustomplot/documentation/html/functions_l.html \
    qcustomplot/documentation/html/functions_m.html \
    qcustomplot/documentation/html/functions_n.html \
    qcustomplot/documentation/html/functions_o.html \
    qcustomplot/documentation/html/functions_p.html \
    qcustomplot/documentation/html/functions_q.html \
    qcustomplot/documentation/html/functions_r.html \
    qcustomplot/documentation/html/functions_rela.html \
    qcustomplot/documentation/html/functions_s.html \
    qcustomplot/documentation/html/functions_t.html \
    qcustomplot/documentation/html/functions_u.html \
    qcustomplot/documentation/html/functions_v.html \
    qcustomplot/documentation/html/functions_vars.html \
    qcustomplot/documentation/html/functions_w.html \
    qcustomplot/documentation/html/functions_x.html \
    qcustomplot/documentation/html/functions_y.html \
    qcustomplot/documentation/html/functions_z.html \
    qcustomplot/documentation/html/global_8h.html \
    qcustomplot/documentation/html/globals.html \
    qcustomplot/documentation/html/globals_type.html \
    qcustomplot/documentation/html/hierarchy.html \
    qcustomplot/documentation/html/index.html \
    qcustomplot/documentation/html/inherits.html \
    qcustomplot/documentation/html/namespacemembers.html \
    qcustomplot/documentation/html/namespacemembers_enum.html \
    qcustomplot/documentation/html/namespacemembers_eval.html \
    qcustomplot/documentation/html/namespacemembers_func.html \
    qcustomplot/documentation/html/namespaceQCP.html \
    qcustomplot/documentation/html/namespaces.html \
    qcustomplot/documentation/html/pages.html \
    qcustomplot/documentation/html/performanceimprovement.html \
    qcustomplot/documentation/html/plottable-bars_8h.html \
    qcustomplot/documentation/html/plottable-curve_8h.html \
    qcustomplot/documentation/html/plottable-errorbar_8h.html \
    qcustomplot/documentation/html/plottable-financial_8h.html \
    qcustomplot/documentation/html/plottable-graph_8h.html \
    qcustomplot/documentation/html/plottable-statisticalbox_8h.html \
    qcustomplot/documentation/html/specialqtflags.html \
    qcustomplot/documentation/html/thelayoutsystem.html \
    qcustomplot/changelog.txt \
    qcustomplot/GPL.txt
