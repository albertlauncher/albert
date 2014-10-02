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
#include "calculatoritem.h"
#include "muParser.h"
#include <QDebug>

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
void Calculator::query(const QString &req, QVector<Service::Item *> *res) const noexcept
{
	using namespace mu;
	try
	{
		Parser p;
		p.SetExpr(req.toStdString());
		_theOneAndOnly->_query = req;
		_theOneAndOnly->_result = QString::number(p.Eval());
		res->push_back(_theOneAndOnly);
	}
	catch (Parser::exception_type &e)
	{
	  std::cout << "[muparser] " << e.GetMsg() << std::endl;
	}
}

/**************************************************************************/
void Calculator::save(const QString &) const
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
}

/**************************************************************************/
void Calculator::load(const QString &)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
}
