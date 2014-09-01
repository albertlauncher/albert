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
	proposallistdelegate.cpp \
	proposallistmodel.cpp \
	proposallistview.cpp

HEADERS  += albert.h \
	xhotkeymanager.h \
	albertengine.h \
	item.h \
	proposallistdelegate.h \
	proposallistmodel.h \
	proposallistview.h

RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11

DEFINES += QT_NO_CAST_FROM_ASCII \
			 QT_NO_CAST_TO_ASCII

QMAKE_CXXFLAGS_WARN_ON += -Werror
