# #####################################################################
# Automatically generated by qmake (2.01a) ?? 28. ??? 23:02:04 2009
# #####################################################################
TEMPLATE = lib
TARGET = dwunog
DESTDIR = "../"
DEPENDPATH += .
INCLUDEPATH += .
RESOURCES += icons.qrc
CONFIG += plugin

# Input
HEADERS += dwunog.h \
    dwunogworld.h \
    pult.h \
    plugin.h \
    plugin_interface.h
SOURCES += dwunog.cpp \
    dwunogworld.cpp \
    pult.cpp \
    plugin.cpp


dummy.path = .
unix:dummy.extra = python ../../Scripts/install_addon.py --addon=dwunog --spec=unix --prefix=$$PREFIX --kumir-dir=$$KUMIR_DIR --resources=dwunogmaps
macx:dummy.extra = python ../../Scripts/install_addon.py --addon=dwunog --spec=macx --prefix=$$PREFIX --kumir-dir=$$KUMIR_DIR --resources=dwunogmaps
win32:dummy.extra = python ../../Scripts/install_addon.py --addon=dwunog --spec=win32 --prefix=$$PREFIX --kumir-dir=$$KUMIR_DIR --resources=dwunogmaps
INSTALLS = dummy