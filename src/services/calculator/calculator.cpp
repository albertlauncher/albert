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

#include "calculator.h"
#include "settings.h"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/trim.hpp"
#include <unistd.h>
#include <sstream>
#include "muParser.h"
#include "websearch/websearch.h"


/**************************************************************************/
Calculator::Calculator()
{
	 _theOneAndOnly = new Calculator::Item;
}

/**************************************************************************/

Calculator::~Calculator()
{
	delete _theOneAndOnly;
}

/**************************************************************************/
void Calculator::query(const std::string &req, std::vector<AbstractServiceProvider::Item *> *res)
{
	using namespace mu;
	try
	{
		std::stringstream ss;
		Parser p;
		p.SetExpr(req);
		ss << p.Eval();
		_theOneAndOnly->_query=req;
		_theOneAndOnly->_result=ss.str();
		res->push_back(_theOneAndOnly);
	}
	catch (Parser::exception_type &e)
	{
	  std::cout << "[muparser] " << e.GetMsg() << std::endl;
	}
}


/*****************************************************************************/
/*****************************************************************************/
/******************************* CalculatorItem *******************************/
/*****************************************************************************/
/**************************************************************************/
void Calculator::Item::action(Action a)
{
	pid_t pid;
	switch (a) {
	case Action::Enter:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/sh", "sh", "-c", std::string("xclip -i <<<" + _result).c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Alt:
		pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/sh", "sh", "-c", std::string("xclip -i <<<" + _query).c_str(), (char *)0);
			exit(1);
		}
		break;
	case Action::Ctrl:
		WebSearch::instance()->defaultSearch(_query);
		break;
	}
}

/**************************************************************************/
std::string Calculator::Item::actionText(Action a) const
{
	switch (a) {
	case Action::Enter:
		return "Copy '" + _result + "' to clipboard";
		break;
	case Action::Alt:
		return "Copy '" + _query + "' to clipboard";
		break;
	case Action::Ctrl:
		return WebSearch::instance()->defaultSearchText(_query);
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************/
QIcon Calculator::Item::icon() const
{
	if (QIcon::hasThemeIcon(QString::fromLocal8Bit("calc")))
		return QIcon::fromTheme(QString::fromLocal8Bit("calc"));
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}
