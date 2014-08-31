#-------------------------------------------------
#
# Project created by QtCreator 2014-07-17T00:53:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = albert
TEMPLATE = app


SOURCES += main.cpp\
		albert.cpp \
	xhotkeymanager.cpp \
	albertengine.cpp \
	item.cpp \
	proposallistwidget.cpp \
    proposallistdelegate.cpp

HEADERS  += albert.h \
	xhotkeymanager.h \
	albertengine.h \
	item.h \
	proposallistwidget.h \
    proposallistdelegate.h

RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11

DEFINES += QT_NO_CAST_FROM_ASCII \
			 QT_NO_CAST_TO_ASCII
