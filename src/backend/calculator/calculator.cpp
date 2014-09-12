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

#include <QDebug>

Calculator* Calculator::_instance = nullptr;

/**************************************************************************//**
 * @brief Calculator::instance
 * @return
 */
Calculator *Calculator::instance(){
	if (_instance == nullptr)
		_instance = new Calculator;
	return _instance;
}

/**************************************************************************//**
 * @brief Calculator::query
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Calculator::query(const std::string &req, std::vector<AbstractServiceProvider::AbstractItem *> *res)
{
	using namespace mu;
	try
	{
		std::stringstream ss;
		Parser p;
		p.SetExpr(req);
		ss << p.Eval();
		_theOneAndonly.set(req, ss.str());
		res->push_back(&_theOneAndonly);
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
/**************************************************************************//**
 * @brief Calculator::CalculatorItem::action
 * @param a
 */
void Calculator::CalculatorItem::action(Action a)
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

/**************************************************************************//**
 * @brief Calculator::CalculatorItem::actionText
 * @param a
 * @return
 */
std::string Calculator::CalculatorItem::actionText(Action a) const
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

/**************************************************************************//**
 * @brief Calculator::CalculatorItem::iconName
 * @return
 */
std::string Calculator::CalculatorItem::iconName() const
{
	return "calc";
}
