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

#include "fileindex.h"
#include "settings.h"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include <functional>
#include <unistd.h>
#include "websearch/websearch.h"
#include "boost/serialization/access.hpp"

//REMOVE
#include <iostream>
#include <fstream>

/**************************************************************************/
FileIndex::FileIndex(){
	_indexFile = Settings::instance()->configDir() + "idx_files";
}

/**************************************************************************/
void FileIndex::buildIndex()
{
	// If there is a serialized index use it
	std::ifstream f(_indexFile);
	if (f.good()){
		boost::archive::text_iarchive ia(f);
		ia.template register_type<Item>();
		ia >> _index;
		f.close();
	}
	else
	{
		bool indexHiddenFiles = (Settings::instance()->get("showHiddenFiles").compare("true") == 0);

		std::string paths = Settings::instance()->get("file_index_paths");
		std::vector<std::string> pathList;
		boost::split(pathList, paths, boost::is_any_of(","), boost::token_compress_on);

		// Define a lambda for recursion
		std::function<void(const boost::filesystem::path &p)> rec_dirsearch = [&] (const boost::filesystem::path &p)
		{
			boost::filesystem::path path(p);
			boost::filesystem::directory_iterator end_iterator;
			if ( boost::filesystem::exists(path) && !boost::filesystem::is_symlink(path))
			{
				if  (p.filename().c_str()[0] == '.' && !indexHiddenFiles)
					return;

				if (boost::filesystem::is_regular_file(path))
					_index.push_back(new Item(path));
				if (boost::filesystem::is_directory(path))
				{
					_index.push_back(new Item(path));
					for( boost::filesystem::directory_iterator d(path); d != end_iterator; ++d)
						rec_dirsearch(*d);
				}
			}
		};

		// Finally do this recursion for all paths
		for ( std::string &p : pathList){
			std::cout << "[FileIndex] Looking in: " << p << std::endl;
			rec_dirsearch(boost::filesystem::path(p));
		}

		std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare(Settings::instance()->locale()));
	}

	std::cout << "[FileIndex] Indexing done. Found " << _index.size() << " files." << std::endl;
}

/**************************************************************************/
void FileIndex::saveIndex() const
{
	std::ofstream f(_indexFile);
	boost::archive::text_oarchive oa(f);
	oa.template register_type<Item>();
	oa << _index;
	f.close();
}


/*****************************************************************************/
/*****************************************************************************/
/******************************* FileIndexItem *******************************/
/*****************************************************************************/

/**************************************************************************/

const QMimeDatabase FileIndex::Item::mimeDb;


/**************************************************************************/
void FileIndex::Item::action(Action a)
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();

	pid_t pid;
	switch (a) {
	case Action::Enter:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", _path.c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Alt:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", _path.parent_path().c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Ctrl:
		WebSearch::instance()->defaultSearch(_name);
		break;
	}
}

/**************************************************************************/
std::string FileIndex::Item::actionText(Action a) const
{
	switch (a) {
	case Action::Enter:
		return "Open '" + _name + "' with default application";
		break;
	case Action::Alt:
		return "Open the folder containing '" + _name + "' in file browser";
		break;
	case Action::Ctrl:
		return WebSearch::instance()->defaultSearchText(_name);
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************/
QIcon FileIndex::Item::icon() const
{
	QString iconName = mimeDb.mimeTypeForFile(QString::fromStdString(_path.string())).iconName();
	if (QIcon::hasThemeIcon(iconName))
		return QIcon::fromTheme(iconName);
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}
