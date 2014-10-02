#-------------------------------------------------
#
# Project created by QtCreator 2014-07-17T00:53:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = albert

INCLUDEPATH += frontend/ \
	backend
VPATH += frontend/ \
	backend

HEADERS  +=	albert.h \
	albertengine.h \
	proposallistdelegate.h \
	proposallistview.h \
	xhotkeymanager.h \
	inputline.h \
	services/service.h \
	services/index.h \
	services/applicationindex/applicationindex.h \
	services/fileindex/fileindex.h \
	#bookmarkindex/bookmarkindex.h \
	#calculator/calculator.h \
	#websearch/websearch.h \
	services/searchimpl.h \
    services/applicationindex/applicationitem.h \
    services/fileindex/fileitem.h

SOURCES += main.cpp \
	albert.cpp \
	albertengine.cpp \
	proposallistdelegate.cpp \
	proposallistview.cpp \
	xhotkeymanager.cpp \
	inputline.cpp \
	services/index.cpp \
	services/applicationindex/applicationindex.cpp \
	services/fileindex/fileindex.cpp \
	#bookmarkindex/bookmarkindex.cpp \
	#calculator/calculator.cpp \
	#websearch/websearch.cpp \
	services/searchimpl.cpp \
    services/applicationindex/applicationitem.cpp \
    services/fileindex/fileitem.cpp

RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11 \
		-lmuparser
