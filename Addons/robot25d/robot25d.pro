TEMPLATE = lib
include(../../Scripts/common.pri)
CONFIG += plugin
QT += svg script network
TARGET = ../robot25d
INCLUDEPATH += ../

HEADERS += \
	cellgraphicsitem.h graphicsimageitem.h \
    robotcell.h roboterrors.h robotview.h robotitem.h \
	sch_environment.h sch_command.h sch_game.h sch_algorhitm.h sch_task.h sch_program.h \
    util.h kumirplugin.h robot25dwindow.h

SOURCES += \
	cellgraphicsitem.cpp graphicsimageitem.cpp \
    robotview.cpp robotitem.cpp \
	sch_environment.cpp sch_command.cpp sch_game.cpp sch_algorhitm.cpp sch_task.cpp sch_program.cpp \
    util.cpp kumirplugin.cpp robot25dwindow.cpp

FORMS += robot25dwindow.ui

RESOURCES +=
