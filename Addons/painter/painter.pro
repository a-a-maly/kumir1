#-------------------------------------------------
#
# Project created by QtCreator 2010-12-06T12:40:29
#
#-------------------------------------------------

include(../../Scripts/common.pri)
CONFIG += plugin
QT += core gui widgets

TEMPLATE = lib

INCLUDEPATH += ../
DESTDIR = ../

SOURCES += \
    painterplugin.cpp \
    painterwindow.cpp \
    painterview.cpp \
    painterruler.cpp \
    painternewimagedialog.cpp \
    paintertools.cpp

HEADERS += painterplugin.h \
    painterwindow.h \
    painterview.h \
    painterruler.h \
    painternewimagedialog.h \
    paintertools.h

FORMS += \
    painterwindow.ui \
    painternewimagedialog.ui

TARGET = painter
macx:TARGET = ../../kumir.app/Contents/Resources/Addons/painter
