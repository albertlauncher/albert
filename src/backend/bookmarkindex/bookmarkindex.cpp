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


//REMOVE
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "websearch/websearch.h"
#include <iostream>

BookmarkIndex* BookmarkIndex::_instance = nullptr;

/**************************************************************************//**
 * @brief BookmarkIndex::instance
 * @return
 */
BookmarkIndex *BookmarkIndex::instance(){
	if (_instance == nullptr)
		_instance = new BookmarkIndex;
	return _instance;
}

/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndex
 */
BookmarkIndex::BookmarkIndex()
{
}

/**************************************************************************//**
 * @brief BookmarkIndex::~BookmarkIndex
 */
BookmarkIndex::~BookmarkIndex()
{
}

/**************************************************************************//**
 * @brief BookmarkIndex::buildIndex
 */
void BookmarkIndex::buildIndex()
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
//					std::cout << "name" <<  i << std::endl;
//					std::cout << i.size() << "/" <<  i.length() << std::endl;

					std::string j = ptvt.second.get_child("url").get_value<std::string>();
//					std::cout << "url" <<  j << std::endl;
//					std::cout << j.size() << "/" <<  j.length() << std::endl;

					_index.push_back(new BookmarkIndexItem(i,j));
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
	std::cout << "[BookmarkIndex] Indexing done. Found " << _index.size() << " bookmarks." << std::endl;



}

/*****************************************************************************/
/*****************************************************************************/
/******************************* BookmarkIndexItem *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndexItem::action
 * @param a
 */
void BookmarkIndex::BookmarkIndexItem::action(Action a)
{
	_lastAccess = std::chrono::system_clock::now();
	pid_t pid;
	switch (a) {
	case Action::Enter:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", _url.c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Ctrl:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", _url.c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Alt:
		WebSearch::instance()->defaultSearch(_name);
		break;
	}
}

/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndexItem::actionText
 * @param a
 * @return
 */
std::string BookmarkIndex::BookmarkIndexItem::actionText(Action a) const
{
	switch (a) {
	case Action::Enter:
		return "Visit '" + _name + "'";
		break;
	case Action::Ctrl:
		return "Visit '" + _name + "'";
		break;
	case Action::Alt:
		return WebSearch::instance()->defaultSearchText(_name);
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************//**
 * @brief BookmarkIndex::BookmarkIndexItem::iconName
 * @return
 */
std::string BookmarkIndex::BookmarkIndexItem::iconName() const
{
	return "favorites";
}
