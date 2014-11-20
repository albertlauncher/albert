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
#include "calculatorwidget.h"
#include "muParser.h"

/**************************************************************************/
Calculator::Calculator()
{
	_p = new mu::Parser;
	_p->SetDecSep(loc.decimalPoint().toLatin1());
	_p->SetThousandsSep(loc.groupSeparator().toLatin1());
	_theOneAndOnly = new Calculator::Item;
}

/**************************************************************************/
Calculator::~Calculator()
{
	delete _theOneAndOnly;
	delete _p;
}

/**************************************************************************/
void Calculator::initialize()
{
}

/**************************************************************************/
void Calculator::saveSettings(QSettings &s) const
{
	// Save settings
	s.beginGroup("Calculator");
	s.endGroup();
}

/**************************************************************************/
void Calculator::loadSettings(QSettings &s)
{
	// Load settings
	s.beginGroup("Calculator");
	s.endGroup();
}

/**************************************************************************/
void Calculator::serilizeData(QDataStream &out) const
{
	// Serialize data
	_theOneAndOnly->serialize(out);
}

/**************************************************************************/
void Calculator::deserilizeData(QDataStream &in)
{
	// Deserialize the index
	_theOneAndOnly->deserialize(in);
}

/**************************************************************************/
QWidget *Calculator::widget()
{
	return new CalculatorWidget(this);
}

/**************************************************************************/
void Calculator::query(const QString &req, QVector<Service::Item *> *res) const
{
	_p->SetExpr(req.toStdString());
	try {
		_theOneAndOnly->_result = loc.toString(_p->Eval());
	}
	catch (mu::Parser::exception_type &e) {
	  std::cout << "[muparser] " << e.GetMsg() << std::endl;
	  return;
	}
	_theOneAndOnly->_query = req;
	res->push_back(_theOneAndOnly);
}

/**************************************************************************/
void Calculator::queryFallback(const QString &, QVector<Service::Item *> *) const
{
}
