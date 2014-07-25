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

#include "albertengine.h"
#include <QSettings>
#include <QDir>

//REMOVE
#include <iostream>
#include <functional>

/**********************************************************************//**
 * @brief AlbertEngine::AlbertEngine
 * @param parent
 */
AlbertEngine::AlbertEngine(QObject *parent) :
	QObject(parent)
{
}

/**********************************************************************//**
 * @brief AlbertEngine::~AlbertEngine
 */
AlbertEngine::~AlbertEngine()
{
	for (auto it = _index.begin(); it != _index.end(); ++it)
		delete (*it);
}

/**********************************************************************//**
 * @brief AlbertEngine::buildIndex
 */
void AlbertEngine::buildIndex()
{
	QSettings conf;
	std::cout << conf.fileName().toStdString() << std::endl;
	QStringList paths = conf.value("paths").toStringList();

	// Define a lambda for recursion
	std::function<void(const QString& p)> rec_dirsearch = [&] (const QString& p) {
		QDir dir(p);
		dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		_index.push_back(new Items::Directory(dir.dirName().toStdString(), dir.canonicalPath().toStdString()));

		// go recursive into subdirs
		QFileInfoList list = dir.entryInfoList();
		for ( QFileInfo &fi : list)
		{
			if (fi.isDir())
				rec_dirsearch(fi.absoluteFilePath());
			if (fi.isExecutable())
				_index.push_back(new Items::Executable(fi.baseName().toStdString(), fi.absoluteFilePath().toStdString()));
			if (fi.suffix() == "deskop")
				_index.push_back(new Items::DesktopApp(fi.baseName().toStdString(), fi.absoluteFilePath().toStdString()));
			// Else is document
				// TODO Check with libmagic
		}
	};

	// Finally do this recursion for all paths
	for ( auto path : paths)
		rec_dirsearch(path);
}

/**********************************************************************//**
 * @brief AlbertEngine::loadIndex
 */
void AlbertEngine::loadIndex()
{

}

/**********************************************************************//**
 * @brief AlbertEngine::saveIndex
 */
void AlbertEngine::saveIndex()
{

}
