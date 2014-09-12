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

//REMOVE
#include <iostream>

FileIndex* FileIndex::_instance = nullptr;

/**************************************************************************//**
 * @brief FileIndex::instance
 * @return
 */
FileIndex *FileIndex::instance(){
	if (_instance == nullptr)
		_instance = new FileIndex;
	return _instance;
}

/**************************************************************************//**
 * @brief FileIndex::FileIndex
 */
FileIndex::FileIndex()
{
//	magic_t _magic_cookie = magic_open(MAGIC_MIME);
//	if (_magic_cookie == NULL) {
//		std::cout << "Unable to initialize magic library" << std::endl;
//	}

//	printf("Loading default magic database\n");
//	if (magic_load(_magic_cookie, NULL) != 0) {
//		std::cout << "Cannot load magic database: " << magic_error(_magic_cookie) << std::endl;
//		magic_close(_magic_cookie);
//	}
}

/**************************************************************************//**
 * @brief FileIndex::~FileIndex
 */
FileIndex::~FileIndex()
{
//	magic_close(_magic_cookie);

}
/**************************************************************************//**
 * @brief FileIndex::buildIndex
 */
void FileIndex::buildIndex()
{
	std::string paths = Settings::instance()->get("file_index_paths");
	std::cout << "[FileIndex] Looking in: " << paths << std::endl;
	std::vector<std::string> pathList;
	boost::split(pathList, paths, boost::is_any_of(","), boost::token_compress_on);

	// Define a lambda for recursion
	std::function<void(const boost::filesystem::path &p)> rec_dirsearch = [&] (const boost::filesystem::path &p)
	{
		boost::filesystem::path path(p);
		boost::filesystem::directory_iterator end_iterator;
		if ( boost::filesystem::exists(path) && !boost::filesystem::is_symlink(path) && p.filename().c_str()[0] != '.')
		{
			if (boost::filesystem::is_regular_file(path))
				_index.push_back(new FileIndexItem(path));
			if (boost::filesystem::is_directory(path))
			{
				_index.push_back(new FileIndexItem(path));
				for( boost::filesystem::directory_iterator d(path); d != end_iterator; ++d)
					rec_dirsearch(*d);
			}
		}
	};

	// Finally do this recursion for all paths
	for ( std::string &p : pathList)
		rec_dirsearch(boost::filesystem::path(p));

	std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare(Settings::instance()->locale()));

//	for ( auto &i : _index)
//		std::cout << i->title() << std::endl;
	std::cout << "[FileIndex] Indexing done. Found " << _index.size() << " files." << std::endl;
}

/*****************************************************************************/
/*****************************************************************************/
/******************************* FileIndexItem *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief FileIndex::FileIndexItem::action
 * @param a
 */
void FileIndex::FileIndexItem::action(Action a)
{
	_lastAccess = std::chrono::system_clock::now();
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
	case Action::Ctrl:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", _path.parent_path().c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Alt:
		WebSearch::instance()->defaultSearch(_name);
		break;
	}
}

/**************************************************************************//**
 * @brief FileIndex::FileIndexItem::actionText
 * @param a
 * @return
 */
std::string FileIndex::FileIndexItem::actionText(Action a) const
{
	switch (a) {
	case Action::Enter:
		return "Open '" + _name + "' with default application";
		break;
	case Action::Ctrl:
		return "Open '" + _name + "' in default file browser";
		break;
	case Action::Alt:
		return WebSearch::instance()->defaultSearchText(_name);
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************//**
 * @brief FileIndex::FileIndexItem::iconName
 * @return
 */
std::string FileIndex::FileIndexItem::iconName() const
{
#ifdef FRONTEND_QT
	return FileIndex::instance()->mimeDb.mimeTypeForFile(QString::fromStdString(_path.string())).iconName().toStdString();
#endif

//	std::string s("xdg-mime query filetype ");
//	s.append(_path.string());
//	FILE* pipe = popen(s.c_str(), "r");
//	if (!pipe)
//		return "ERROR";

//	s.clear();
//	while(!feof(pipe)) {
//		char buffer[128];
//		if(fgets(buffer, 128, pipe) != NULL)
//			s += buffer;
//	}
//	pclose(pipe);
//	return s
//--------------------------------------------------------------------------...
	//	std::string s(magic_file(FileIndex::instance()->_magic_cookie, _path.c_str()));
//	return s;
//--------------------------------------------------------------------------...
}
