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

#include "bookmarkindex.h"
#include "settings.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <pwd.h>
#include <unistd.h>
#include <functional>
#include <fstream>
#include "websearch/websearch.h"


//REMOVE
#include <iostream>

/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndex
 */
BookmarkIndex::BookmarkIndex(){
	_indexFile = Settings::instance()->configDir() + "idx_bookmarks";
}

/**************************************************************************//**
 * @brief BookmarkIndex::buildIndex
 */
void BookmarkIndex::buildIndex()
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
		using namespace boost::property_tree;

		// Define a lambda for recursion
		std::function<void(const ptree &pt)> rec_bmsearch = [&] (const ptree &pt)
		{
			for (const ptree::value_type& ptvt : pt) {
				if (!ptvt.second.empty()){
					if (!ptvt.second.get_child("type").get_value<std::string>().compare("folder"))
						rec_bmsearch(ptvt.second.get_child("children"));
					else // (!ptvt.get_child("type").get_value<std::string>().compare("url"))
					{
						std::string i = ptvt.second.get_child("name").get_value<std::string>();
						std::string j = ptvt.second.get_child("url").get_value<std::string>();
						_index.push_back(new Item(i,j));
					}
				}
			}
		};

		// Finally do this recursion for all paths
		std::string bookmarkPath(getpwuid(getuid())->pw_dir);
		bookmarkPath += "/" + Settings::instance()->get("chromium_bookmark_path");
		std::cout << "[BookmarkIndex] Parsing in: " << bookmarkPath << std::endl;
		try
		{
			ptree pt;
			read_json(bookmarkPath, pt);// Settings::instance()->locale());
			rec_bmsearch(pt.get_child("roots"));
		}
		catch (std::exception const& e)
		{
			std::cerr << "[BookmarkIndex] Could not build index: " << e.what() << std::endl;
		}

		std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare(Settings::instance()->locale()));
	}

	std::cout << "[BookmarkIndex] Indexing done. Found " << _index.size() << " bookmarks." << std::endl;
}

/**************************************************************************//**
 * @brief BookmarkIndex::saveIndex
 */
void BookmarkIndex::saveIndex() const
{
	std::ofstream f(_indexFile);
	boost::archive::text_oarchive oa(f);
	oa.template register_type<Item>();
	oa << _index;
	f.close();
}


/*****************************************************************************/
/*****************************************************************************/
/******************************* BookmarkIndexItem *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndexItem::action
 * @param a
 */
void BookmarkIndex::Item::action(Action a)
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();

	pid_t pid;
	switch (a) {
	case Action::Enter:
	case Action::Alt:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", _url.c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Ctrl:
		WebSearch::instance()->defaultSearch(_name);
		break;
	}
}

/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndexItem::actionText
 * @param a
 * @return
 */
std::string BookmarkIndex::Item::actionText(Action a) const
{
	switch (a) {
	case Action::Enter:
	case Action::Alt:
		return "Visit '" + _name + "'";
		break;
	case Action::Ctrl:
		return WebSearch::instance()->defaultSearchText(_name);
		break;
	}
	// Will never happen
	return "";
}
