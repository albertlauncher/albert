#-------------------------------------------------
#
# Project created by QtCreator 2014-07-17T00:53:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = albert
TEMPLATE = app

VPATH += frontend/\
	backend/ \
	backend/index/ \
	backend/index/genericmimeindex/

HEADERS  += albert.h \
	xhotkeymanager.h \
	albertengine.h \
	proposallistdelegate.h \
	proposallistmodel.h \
	proposallistview.h \
	abstractitem.h \
	abstractserviceprovider.h \
	abstractindexitem.h \
	genericmimeitem.h \
	index.h \
	abstractindex.h \
	genericmimeindex.h

SOURCES += main.cpp\
	albert.cpp \
	xhotkeymanager.cpp \
	albertengine.cpp \
	proposallistdelegate.cpp \
	proposallistmodel.cpp \
	proposallistview.cpp \
	genericmimeitem.cpp \
	index.cpp \
	abstractindex.cpp \
	genericmimeindex.cpp

RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11

DEFINES += QT_NO_CAST_FROM_ASCII \
			 QT_NO_CAST_TO_ASCII

QMAKE_CXXFLAGS_WARN_ON += -Werror
