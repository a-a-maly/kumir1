TEMPLATE = lib
include(../../../Scripts/common.pri)
QT += svg network
CONFIG += plugin dll

SOURCES += \
    pult.cpp \
    network.cpp \
    vodoley.cpp \
    dialog.cpp \
    interface.cpp

HEADERS += \
    pult.h \
    network.h \
    vodoley.h \
    dialog.h \
    interface.h \
    ../../plugin_interface.h

FORMS += pult.ui dialog.ui

RESOURCES = application.qrc

TARGET = ../../vodoley
win32:TARGET = ../../../vodoley
macx:TARGET = ../../../kumir.app/Contents/Resources/Addons/vodoley
