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
#include <QStandardPaths>
#include <QDir>
#include "mainwidget.h"


int main(int argc, char *argv[])
{
	// Application
	QApplication a(argc, argv);
	QCoreApplication::setApplicationName(QString::fromLocal8Bit("albert"));
	a.setWindowIcon(QIcon(":app_icon"));
	a.setQuitOnLastWindowClosed(false); // Dont quit after settings close


	{ // FIRST RUN STUFF
		QDir data(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
		if (!data.exists())
			data.mkpath(".");
		QDir conf(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
				  +"/"+qApp->applicationName());
		if (!conf.exists())
			conf.mkpath(".");
	}

	MainWidget *w = new MainWidget;
	int retval = a.exec();
	delete w;
	return retval;
}
