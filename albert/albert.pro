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
		commandline.cpp \
	xhotkeymanager.cpp \
	albertengine.cpp \
	item.cpp

HEADERS  += albert.h \
		commandline.h \
	xhotkeymanager.h \
	albertengine.h \
	item.h

RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11
