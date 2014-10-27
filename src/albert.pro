#-------------------------------------------------
#
# Project created by QtCreator 2014-07-17T00:53:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = albert

INCLUDEPATH += services/ ui/


HEADERS  +=	albertengine.h \
	singleton.h \
	Timer.h \
	services/service.h \
	services/fileindex/fileindex.h \
	services/bookmarkindex/bookmarkindex.h \
	services/calculator/calculator.h \
	services/websearch/websearch.h \
	services/fileindex/fileitem.h \
	services/calculator/calculatoritem.h \
	services/bookmarkindex/bookmarkitem.h \
	services/websearch/websearchitem.h \
	services/appindex/appindex.h \
	services/appindex/appindexwidget.h \
	services/appindex/appitem.h \
	services/bookmarkindex/bookmarkindexwidget.h \
	services/calculator/calculatorwidget.h \
	services/fileindex/fileindexwidget.h \
	services/websearch/websearchwidget.h \
	services/indexservice.h \
	ui/albert.h \
	ui/proposallistdelegate.h \
	ui/proposallistview.h \
	ui/inputline.h \
	ui/settingsbutton.h \
	ui/settingsdialog.h \
	ui/globalhotkey.h \
	ui/globalhotkey_p.h \
	ui/hotkeywidget.h

SOURCES += main.cpp \
	albertengine.cpp \
	services/fileindex/fileindex.cpp \
	services/bookmarkindex/bookmarkindex.cpp \
	services/calculator/calculator.cpp \
	services/websearch/websearch.cpp \
	services/fileindex/fileitem.cpp \
	services/calculator/calculatoritem.cpp \
	services/bookmarkindex/bookmarkitem.cpp \
	services/websearch/websearchitem.cpp \
	services/appindex/appindexwidget.cpp \
	services/appindex/appindex.cpp \
	services/appindex/appitem.cpp \
	services/bookmarkindex/bookmarkindexwidget.cpp \
	services/calculator/calculatorwidget.cpp \
	services/fileindex/fileindexwidget.cpp \
	services/websearch/websearchwidget.cpp \
	services/indexservice.cpp \
	ui/albert.cpp \
	ui/proposallistdelegate.cpp \
	ui/proposallistview.cpp \
	ui/inputline.cpp \
	ui/settingsbutton.cpp \
	ui/settingsdialog.cpp \
	ui/globalhotkey.cpp \
	ui/hotkeywidget.cpp

win32 {
	SOURCES += ui/globalhotkey_win.cpp
}
unix:!macx {
	QT += x11extras
	SOURCES += ui/globalhotkey_x11.cpp
}








RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11 \
		-lmuparser

FORMS = ui/settingsdialog.ui \
		services/appindex/appindexwidget.ui \
		services/bookmarkindex/bookmarkindexwidget.ui \
		services/calculator/calculatorwidget.ui \
		services/fileindex/fileindexwidget.ui \
		services/websearch/websearchwidget.ui

#DEFINES += QT_NO_CAST_FROM_ASCII \
			 #QT_NO_CAST_TO_ASCII

#QMAKE_CXXFLAGS_WARN_ON += -Werror
