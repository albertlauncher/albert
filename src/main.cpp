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
#include "albert.h"

int main(int argc, char *argv[])
{
	// Application
	QApplication a(argc, argv);
	QCoreApplication::setApplicationName(QString::fromLocal8Bit("albert"));
	a.setQuitOnLastWindowClosed(false); // Dont quit after settings close

	// Create the app
	AlbertWidget w;

	/* Style */
	// Get theme name from config
	QString theme = QSettings(QSettings::UserScope, "albert", "albert")
			.value("theme", "Standard.qss").toString();

	// Get theme dirs
	QStringList themeDirs = QStandardPaths::locateAll(QStandardPaths::DataLocation,
													  "themes",
													  QStandardPaths::LocateDirectory);
	// Find the theme
	for (QDir d : themeDirs)
	{
		QFileInfoList fil = d.entryInfoList(QStringList("*.qss"),
											QDir::Files | QDir::NoSymLinks);
		for (QFileInfo fi : fil){
			if (fi.fileName() == theme)
			{
				// Apply the theme
				QFile styleFile(fi.canonicalFilePath()); // TODO errorhandling
				if (styleFile.open(QFile::ReadOnly)) {
					qApp->setStyleSheet(styleFile.readAll());
					styleFile.close();
				}
				else
					qWarning() << "Could not open style file";;
			}
		}
	}


	// Enter eventloop
	int retval = a.exec();
	return retval;
}
