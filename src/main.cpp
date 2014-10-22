// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QSettings>
#include <QDebug>
#include "albert.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName(QString::fromLocal8Bit("albert"));
	QCoreApplication::setApplicationName(QString::fromLocal8Bit("albert"));
	qDebug() << "[QSettings]\t\t" << QSettings().fileName() ;

	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false); // Dont quit after settings close

	// Create the app
	gAlbertWidget;

	// Enter eventloop
	int retval = a.exec();

	return retval;
}
