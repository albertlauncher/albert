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

#include "applicationindex.h"
#include "settings.h"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include <functional>
#include <sstream>
#include <fstream>

//REMOVE
#include <iostream>

ApplicationIndex* ApplicationIndex::_instance = nullptr;

/**************************************************************************//**
 * @brief ApplicationIndex::instance
 * @return
 */
ApplicationIndex *ApplicationIndex::instance(){
	if (_instance == nullptr)
		_instance = new ApplicationIndex;
	return _instance;
}

/**************************************************************************//**
 * @brief ApplicationIndex::buildIndex
 */

void ApplicationIndex::buildIndex()
{
	std::string paths = Settings::instance()->get("app_index_paths");
	std::cout << "[ApplicationIndex] Looking in: " << paths << std::endl;
	std::vector<std::string> pathList;
	boost::split(pathList, paths, boost::is_any_of(":,"), boost::token_compress_on);

	// Define a lambda for recursion
	std::function<void(const boost::filesystem::path &p)> rec_dirsearch = [&] (const boost::filesystem::path &p)
	{
		boost::filesystem::path path(p);
		boost::filesystem::directory_iterator end_iterator;
		if ( boost::filesystem::exists(path))
		{
			if (boost::filesystem::is_directory(path))
			{
				for( boost::filesystem::directory_iterator d(path); d != end_iterator; ++d)
					rec_dirsearch(*d);
			}
			if (boost::filesystem::is_regular_file(path))
			{
				std::map<std::string, std::string> desktopfile;
				std::ifstream file(path.string());
				if (!file.good())
					return;
				std::string str;
				while (std::getline(file, str))
				{
					std::size_t found = str.find_first_of('=');
					if (found == std::string::npos || found == str.length())
						continue;
					desktopfile[str.substr(0,found)] = str.substr(found+1);
				}

				_index.push_back(
					new ApplicationIndexItem(
						desktopfile["Name"],
						(desktopfile["Comment"].empty())?desktopfile["GenericName"]:desktopfile["Comment"],
						desktopfile["IconName"],
						desktopfile["Exec"],
						(desktopfile["Terminal"]=="false")?false:true
					)
				);
			}
		}
	};

	// Finally do this recursion for all paths
	for ( std::string &p : pathList)
		rec_dirsearch(boost::filesystem::path(p));
	std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare());

	std::cout << "[ApplicationIndex] Indexing done. Found " << _index.size() << " apps." << std::endl;
}

/*****************************************************************************/
/*****************************************************************************/
/************************** ApplicationIndexItem *****************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief ApplicationIndex::ApplicationIndexItem::action
 * @param a
 */
void ApplicationIndex::ApplicationIndexItem::action(Action a)
{
//	_lastAccess = std::chrono::system_clock::now();

	if (a == Action::Enter)
		return startDetached(_exec, "");

	if (a == Action::Ctrl)
		return startDetached(_exec, "");

	// else Action::Alt
	fallbackAction(a);
}

/**************************************************************************//**
 * @brief ApplicationIndex::ApplicationIndexItem::actionText
 * @param a
 * @return
 */
std::string ApplicationIndex::ApplicationIndexItem::actionText(Action a) const
{
	std::ostringstream stringStream;

	if (a == Action::Enter){
		stringStream << "Start '" << _title << "'.";
		return stringStream.str();
	}

	if (a == Action::Ctrl){
		stringStream << "Start '" << _title << "' as root.";
		return stringStream.str();
	}

	// else Action::Alt
	stringStream << "Search for '" << _title << "' in web.";
	return stringStream.str();
}

