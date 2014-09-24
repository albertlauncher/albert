#-------------------------------------------------
#
# Project created by QtCreator 2014-07-17T00:53:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = albert
TEMPLATE = app



INCLUDEPATH += frontend/ \
	backend
VPATH += frontend/ \
	backend

HEADERS  +=	backend/abstractindexprovider.h \
	backend/abstractserviceprovider.h \
	backend/albertengine.h \
	albert.h \
	frontend/proposallistdelegate.h \
	frontend/proposallistmodel.h \
	frontend/proposallistview.h \
	frontend/xhotkeymanager.h \
	frontend/inputline.h \
	backend/settings.h \
	singleton.h \
	backend/applicationindex/applicationindex.h \
	backend/bookmarkindex/bookmarkindex.h \
	backend/fileindex/fileindex.h \
	backend/calculator/calculator.h \
	backend/websearch/websearch.h

SOURCES += main.cpp \
	backend/abstractindexprovider.cpp \
	backend/abstractserviceprovider.cpp \
	backend/albertengine.cpp \
	albert.cpp \
	frontend/proposallistdelegate.cpp \
	frontend/proposallistmodel.cpp \
	frontend/proposallistview.cpp \
	frontend/xhotkeymanager.cpp \
	frontend/inputline.cpp \
	backend/settings.cpp \
	backend/applicationindex/applicationindex.cpp \
	backend/bookmarkindex/bookmarkindex.cpp \
	backend/fileindex/fileindex.cpp \
	backend/calculator/calculator.cpp \
	backend/websearch/websearch.cpp

RESOURCES += albert.qrc

CONFIG += c++11

LIBS += -lX11 \
		-lboost_system \
		-lboost_filesystem \
		-lmuparser \
		-lboost_serialization


DEFINES += FRONTEND_QT \
			QT_NO_CAST_FROM_ASCII \
			QT_NO_CAST_TO_ASCII


target.path = /usr/local/bin/
INSTALLS += target


