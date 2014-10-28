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
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include "globals.h"
#include "mainwidget.h"

QSettings *gSettings = nullptr;

int main(int argc, char *argv[])
{
	// Settings
	gSettings = new QSettings(QSettings::UserScope, "albert", "albert");

	// Application
	QApplication a(argc, argv);
	QCoreApplication::setApplicationName(QString::fromLocal8Bit("albert"));
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

	// Create the app
	MainWidget *w = new MainWidget;

	/*  BEGIN Style */
	// Get theme name from config
	QString theme = gSettings->value("theme").toString();

	// Get theme dirs
	QStringList themeDirs = QStandardPaths::locateAll(QStandardPaths::DataLocation,
													  "themes",
													  QStandardPaths::LocateDirectory);
	// Get all themes
	QFileInfoList themes;
	for (QDir d : themeDirs)
		themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);

	// Find the theme
	bool success = false;
	for (QFileInfo fi : themes){
		if (fi.fileName() == theme)
		{
			// Apply the theme
			QFile styleFile(fi.canonicalFilePath()); // TODO errorhandling
			if (styleFile.open(QFile::ReadOnly)) {
				qApp->setStyleSheet(styleFile.readAll());
				styleFile.close();
				success = true;
			}
		}
	}

	if (!success)
	{
		qWarning() << "Error setting style. Using fallback";
		a.setStyleSheet(QString::fromLocal8Bit("file:///:/resources/Standard.qss"));
	}
	/*  END Style */

	// Enter eventloop
	int retval = a.exec();

	// Cleanup
	delete w;
	delete gSettings;

	// Quit
	return retval;
}
