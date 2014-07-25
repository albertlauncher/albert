#include "albert.h"
#include <QApplication>
#include <QSettings>
#include <iostream>
#include <QObject>




int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("albert");
	QCoreApplication::setApplicationName("albert");

    QApplication a(argc, argv);
    a.setStyleSheet("file:///:/resources/basicskin.qss");
	AlbertWidget w;
    return a.exec();
}
