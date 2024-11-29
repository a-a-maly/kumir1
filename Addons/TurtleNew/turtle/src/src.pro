TEMPLATE = lib
include(../../../../Scripts/common.pri)

CONFIG += plugin
QT += svg network

SOURCES += \
    turtle.cpp \
    pult.cpp \
    network.cpp \
    interface.cpp
HEADERS += \
    turtle.h \
    pult.h \
    network.h \
    interface.h \
    ../../../plugin_interface.h

RESOURCES = application.qrc
FORMS += pult.ui
TARGET = ../../../turtle
win32:TARGET = ../../../../turtle
macx:TARGET = ../../../../kumir.app/Contents/Resources/Addons/turtle
