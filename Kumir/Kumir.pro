TARGET = kumir
TEMPLATE = app
message(CONFIG=$$CONFIG)
CONFIG += qt uic release warn_on
QT += \
	gui \
	xml \
	script \
	svg \
	network\
	printsupport\
	webkit\
	webkitwidgets


FORMS += \
	Forms/ProgramTab.ui \
	Forms/NewMacroDialog.ui \
	Forms/MacrosManagementDialog.ui \
	Forms/Assistant.ui \
	Forms/GoToLineDialog.ui \
	Forms/DebugDialog.ui \
	Forms/SettingsDialog.ui \
	Forms/about.ui \
	Forms/tooltip.ui \
	Forms/kum_tables.ui \
	Forms/ModulesRegistration.ui \
	Forms/CheckModules.ui \
	Forms/textwindow.ui \
	Forms/textwindow1.ui \
	Forms/RoboWindow.ui \
	Forms/cell_dialog.ui \
	Forms/editField.ui \
	Forms/pult.ui \
	Forms/printDialog.ui \
	Forms/externalIsp.ui \
	Forms/MainWindow.ui \
	Forms/multifilesavewizard.ui \
	Forms/PDDebugWindow.ui

HEADERS += \
	Headers/mainwindow.h \
	Headers/kumiredit.h \
	Headers/lineprop.h \
	Headers/syntaxhighlighter.h \
	Headers/settingsdialog.h \
	Headers/kumirmargin.h \
	Headers/application.h \
	Headers/programtab.h \
	Headers/texttab.h \
	Headers/textedit.h \
	Headers/int_proga.h \
	Headers/const.h \
	Headers/tools.h \
	Headers/error.h \
	Headers/error_internal.h \
	Headers/text_analiz.h \
	Headers/compiler.h \
	Headers/messagesprovider.h \
	Headers/macro.h \
	Headers/newmacrodialog.h \
	Headers/macrosmanagementdialog.h \
	Headers/tab.h \
	Headers/assistant.h \
	Headers/gotolinedialog.h \
	Headers/network.h \
	Headers/run.h \
	Headers/kum_instrument.h \
	Headers/debugdialog.h \
	Headers/KumIOArea.h \
	Headers/kumfiledialog.h \
	Headers/kumtabwidget.h \
	Headers/findpanel.h \
	Headers/aboutdialog.h \
	Headers/tooltip.h \
	Headers/files_isp.h \
	Headers/kum_tables.h \
	Headers/kassert.h \
	Headers/kumundostack.h \
	Headers/modulesregistration.h \
	Headers/textwindow.h \
	Headers/config.h \
	Headers/semaphorewidget.h \
	Headers/integeroverflowchecker.h \
	Headers/doubleoverflowchecker.h \
	Headers/robotwindow.h \
	Headers/cell_dialog.h \
	Headers/editField.h \
	Headers/pushdownautomata.h \
	Headers/pwrulesnormalizer.h \
	Headers/pult.h \
	Headers/printDialog.h \
	Headers/signal_crosser.h \
	Headers/externalIsp.h \
	Headers/httpdaemon.h \
	Headers/kumtabbar.h \
	Headers/subprocess.h \
	Headers/kumsinglemodule.h \
	Headers/kummodules.h \
	Headers/files_p_isp.h \
	Headers/extstrings.h\
	Headers/latinhighlighter.h \
	Headers/pult_webkit.h \
	Headers/secondarywindow.h \
	Headers/kumrobot.h \
	Headers/kumdraw.h \
	Headers/kumscene.h \
	Headers/tableviewer.h \
	../TaskControl/csInterface.h \
	../Addons/plugin_interface.h

SOURCES += \
	Sources/mainwindow.cpp \
	Sources/main.cpp \
	Sources/kumiredit.cpp \
	Sources/syntaxhighlighter.cpp \
	Sources/settingsdialog.cpp \
	Sources/kumirmargin.cpp \
	Sources/application.cpp \
	Sources/kumtabbar.cpp \
	Sources/programtab.cpp \
	Sources/textedit.cpp \
	Sources/int_proga.cpp \
	Sources/tools.cpp \
	Sources/text_analiz.cpp \
	Sources/compiler.cpp \
	Sources/messagesprovider.cpp \
	Sources/macro.cpp \
	Sources/newmacrodialog.cpp \
	Sources/macrosmanagementdialog.cpp \
	Sources/texttab.cpp \
	Sources/assistant.cpp \
	Sources/gotolinedialog.cpp \
	Sources/network.cpp \
	Sources/run.cpp \
	Sources/kum_instrument.cpp \
	Sources/debugdialog.cpp \
	Sources/tab.cpp \
	Sources/KumIOArea.cpp \
	Sources/kumfiledialog.cpp \
	Sources/kumtabwidget.cpp \
	Sources/findpanel.cpp \
	Sources/aboutdialog.cpp \
	Sources/tooltip.cpp \
	Sources/files_isp.cpp \
	Sources/kum_tables.cpp \
	Sources/modulesregistration.cpp \
	Sources/textwindow.cpp \
	Sources/semaphorewidget.cpp \
	Sources/integeroverflowchecker.cpp \
	Sources/doubleoverflowchecker.cpp \
	Sources/robotwindow.cpp \
	Sources/cell_dialog.cpp \
	Sources/editField.cpp \
	Sources/pushdownautomata.cpp \
	Sources/pwrulesnormalizer.cpp \
	Sources/pult.cpp \
	Sources/printDialog.cpp \
	Sources/externalIsp.cpp \
	Sources/httpdaemon.cpp \
	Sources/subprocess.cpp \
	Sources/kumsinglemodule.cpp \
	Sources/kummodules.cpp \
	Sources/files_p_isp.cpp \
	Sources/extstrings.cpp\
	Sources/latinhighlighter.cpp \
	Sources/pult_webkit.cpp \
	Sources/kumrobot.cpp \
	Sources/kumdraw.cpp \
	Sources/kumscene.cpp \
	Sources/tableviewer.cpp

TRANSLATIONS = \
	Languages/english.ts \
	Languages/russian.ts \
	Languages/french.ts

INCLUDEPATH += Headers
INCLUDEPATH += ../
INCLUDEPATH += ../Addons/protoModule

RESOURCES += Resources/MainWindow.qrc

macx: ICON=../app_icons/mac/kumir.icns


include(../Scripts/common.pri)

unix:dummy.extra=python ../Scripts/install_kumir.py --spec=unix --prefix=$$PREFIX --kumir-dir=$$KUMIR_DIR

macx:dummy.extra=../Mac/Kumir/embed_macosx_resources.sh

win32:dummy.extra=python ../Scripts/install_kumir.py --spec=win32 --prefix=$$PREFIX --kumir-dir=$$KUMIR_DIR

dummy.path=./

INSTALLS = dummy


unix {
	HEADERS += Headers/secondarywindow_x11.h
	SOURCES += Sources/secondarywindow_x11.cpp
	QT += x11extras
	CONFIG += x11
}

macx {
	HEADERS -= Headers/secondarywindow_x11.h
	SOURCES -= Sources/secondarywindow_x11.cpp
	HEADERS += Headers/secondarywindow_mac.h
	SOURCES += Sources/secondarywindow_mac.cpp
}

win32 {
	HEADERS += Headers/secondarywindow_win32.h
	SOURCES += Sources/secondarywindow_win32.cpp
}

win32-msvc* {
	LIBS += -lUser32
}

#unix {
#    QMAKE_CXXFLAGS_RELEASE += -DQT_NO_DEBUG_OUTPUT
#}
