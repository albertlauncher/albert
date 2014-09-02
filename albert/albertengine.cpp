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
#include <algorithm>

//REMOVE
#include <string>
#include <iostream>
#include <functional>
#include <QDebug>

using std::string;

bool lexicographically (Items::AbstractItem*  i, Items::AbstractItem* j)
{
	return 0 > i->name().compare(j->name(), Qt::CaseInsensitive);
}

//bool sortByName(const Items::AbstractItem &lhs, const Items::AbstractItem &rhs) { return lhs.name() < rhs.name(); }

AlbertEngine* AlbertEngine::_instance = nullptr;

/**********************************************************************//**
 * @brief AlbertEngine::AlbertEngine
 * @param parent
 */
AlbertEngine::AlbertEngine()
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
	std::cout << "Config: " << conf.fileName().toStdString() << std::endl;
	QStringList paths = conf.value(QString::fromLocal8Bit("paths")).toStringList();

	// Define a lambda for recursion
	std::function<void(const QString& p)> rec_dirsearch = [&] (const QString& p) {
		QDir dir(p);
		dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
		_index.push_back(new Items::Directory(dir.dirName(), dir.canonicalPath()));

		// go recursive into subdirs
		QFileInfoList list = dir.entryInfoList();
		for ( QFileInfo &fi : list)
		{
			if (fi.isDir()){
				rec_dirsearch(fi.absoluteFilePath());
//				std::cout << "Entry(dir): " << fi.absoluteFilePath().toStdString() << std::endl;
			}
			if (fi.isFile()) {
				//				if (fi.isExecutable()){
				//					_index.push_back(new Items::Executable(fi.completeBaseName(), fi.absoluteFilePath()));
				//					std::cout << "Entry(exe): " << fi.absoluteFilePath().toStdString() << std::endl;
				//				}
				//				if (fi.suffix() == "deskop"){
				//					_index.push_back(new Items::DesktopApp(fi.completeBaseName(), fi.absoluteFilePath()));
				//					std::cout << "Entry(DesktopApp): " << fi.absoluteFilePath().toStdString() << std::endl;
				//				} else {
									_index.push_back(new Items::Document(fi.completeBaseName(), fi.absoluteFilePath()));
//									std::cout << "Entry(Document): " << fi.absoluteFilePath().toStdString() << std::endl;
				//
			}
		}
	};

	// Finally do this recursion for all paths
	for ( auto path : paths)
		rec_dirsearch(path);


	std::sort(_index.begin(), _index.end(), lexicographically);
	qDebug() << "Found" << _index.size() << "items.";
}


///**********************************************************************//**
// * @brief AlbertEngine::loadIndex
// */
//void AlbertEngine::loadIndex()
//{

//}

///**********************************************************************//**
// * @brief AlbertEngine::saveIndex
// */
//void AlbertEngine::saveIndex()
//{

//}

/**********************************************************************//**
 * @brief AlbertEngine::saveIndex
 */
std::vector<Items::AbstractItem *> AlbertEngine::request(const QString &req)
{
	QString reqlo = req.toLower();
	std::vector<Items::AbstractItem *>::const_iterator it, first, last, lb;
	std::iterator_traits<std::vector<Items::AbstractItem *>::const_iterator>::difference_type count, step;

	// lower bound
	first = _index.cbegin();
	last = _index.cend();
	count = distance(first,last);
	while (count>0) {
		it = first;
		step=count/2;
		advance (it,step);
		if (strncmp(reqlo.toStdString().c_str(), (*it)->name().toLower().toStdString().c_str(), reqlo.size()) > 0) {
			first=++it;
			count-=step+1;
		}
		else
			count=step;
	}
	lb = it;

	// upper bound
	while (it != _index.end() && reqlo.toStdString().compare(0, string::npos, (*it)->name().toLower().toStdString(),0,reqlo.size()) == 0){
		++it;
	}

	return std::vector<Items::AbstractItem *>(lb, it);
}
