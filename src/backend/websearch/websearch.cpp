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

#include "websearch.h"
#include "settings.h"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/trim.hpp"
//#include <unistd.h>

/**************************************************************************/
WebSearch::WebSearch()
{
	std::string engines = Settings::instance()->get("search_engines");
	std::vector<std::string> engineList;
	boost::split(engineList, engines, boost::is_any_of(";"), boost::token_compress_on);

	for (std::string &e : engineList)
	{
		std::vector<std::string> engineComponents;
		boost::split(engineComponents, e, boost::is_any_of(","), boost::token_compress_off);
		if(engineComponents.size() == 4)
			_searchEngines.push_back(new Item(engineComponents[0],engineComponents[1],engineComponents[2],engineComponents[3]));
	}
}


/**************************************************************************/
void WebSearch::query(const std::string &req, std::vector<AbstractServiceProvider::Item *> *res)
{
	for (Item *w : _searchEngines){
		std::string fstToken(req, 0, req.find_first_of(' '));
		std::transform(fstToken.begin(), fstToken.end(),
					   fstToken.begin(), std::bind2nd(std::ptr_fun(&std::tolower<char>), Settings::instance()->locale()));

		std::string lowerFullName(w->name());
		std::transform(lowerFullName.begin(), lowerFullName.end(),
					   lowerFullName.begin(), std::bind2nd(std::ptr_fun(&std::tolower<char>), Settings::instance()->locale()));

		if (w->shortcut().compare(0, fstToken.length(), fstToken) == 0
				|| lowerFullName.compare(0, fstToken.length(), fstToken) ==0 ) {

			std::string rest(req, fstToken.length(), std::string::npos);
			boost::algorithm::trim_left(rest);
			w->setTerm(rest);
			res->push_back(w);
		}

	}
}

/**************************************************************************/
void WebSearch::queryAll(const std::string &req, std::vector<AbstractServiceProvider::Item *> *res)
{
	for (Item *w : _searchEngines)
		w->setTerm(req);
	res->insert(res->end(), _searchEngines.begin(), _searchEngines.end());
}

/**************************************************************************/
void WebSearch::defaultSearch(const std::string &term) const{
	std::string url("https://www.google.de/search?q=");
	url.append(term);
	pid_t pid = fork();
	if (pid == 0) {
		pid_t sid = setsid();
		if (sid < 0) exit(EXIT_FAILURE);
		execl("/usr/bin/xdg-open", "xdg-open", url.c_str(), (char *)0);
		exit(1);
	}
}

/**************************************************************************/
std::string WebSearch::defaultSearchText(const std::string &term) const{
	return "Search for  '" + term + "' in the web";
}

/*****************************************************************************/
/*****************************************************************************/
/******************************* WebSearchItem *******************************/
/*****************************************************************************/
/**************************************************************************/
void WebSearch::Item::action(Action a)
{
	pid_t pid;
	switch (a) {
	case Action::Enter:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", std::string(_url).replace(_url.find("%s"), 2, _searchTerm).c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Alt:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/sh", "sh", "-c", std::string("xclip -i <<< echo \""+std::string(_url).replace(_url.find("%s"), 2, _searchTerm)+"\"").c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Ctrl:
		WebSearch::instance()->defaultSearch(_name);
		break;
	}
}

/**************************************************************************/
std::string WebSearch::Item::actionText(Action a) const
{
	switch (a) {
	case Action::Enter:
		return "Visit '" + std::string(_url).replace(_url.find("%s"), 2, _searchTerm) + "'";
		break;
	case Action::Alt:
		return "Copy '" + std::string(_url).replace(_url.find("%s"), 2, _searchTerm) + "' to clipboard";
		break;
	case Action::Ctrl:
		return WebSearch::instance()->defaultSearchText(_searchTerm);
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************/
std::string WebSearch::Item::iconName() const
{
	return _iconName;
}
