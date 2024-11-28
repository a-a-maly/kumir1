TEMPLATE = lib
include(../../../Scripts/common.pri)
QT += svg network
CONFIG += plugin dll

SOURCES += main.cpp \
    pult.cpp \
    network.cpp \
    vodoley.cpp \
    dialog.cpp \
    kumfiledialog.cpp \
    interface.cpp

HEADERS += pult.h \
    network.h \
    vodoley.h \
    dialog.h \
    kumfiledialog.h \
    ../../plugin_interface.h \
    interface.h \
    ../../isp_window_header.h

FORMS += pult.ui \
    dialog.ui

RESOURCES = application.qrc

TARGET = ../../vodoley
win32:TARGET = ../../../vodoley
macx:TARGET = ../../../kumir.app/Contents/Resources/Addons/vodoley
