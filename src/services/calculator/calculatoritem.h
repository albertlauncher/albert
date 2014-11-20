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

#ifndef CALCULATORITEM_H
#define CALCULATORITEM_H

#include "calculator.h"
#include <QString>
#include <QIcon>
#include <QDataStream>

class Calculator::Item : public Service::Item
{
	friend class Calculator;

public:
	Item(){}
	~Item(){}

	inline QString title() const override {return _result;}
	inline QString complete() const override {return _query;}
	inline QString infoText() const override {return "Result of '"+_query+"'";}
	QIcon icon() const override;

	void    action() override;
	QString actionText() const override;
	void    altAction() override;
	QString altActionText() const override;

protected:
	QString _query;
	QString _result;

	// Serialization
	void serialize (QDataStream &out) const override;
	void deserialize (QDataStream &in) override;
};
#endif // CALCULATORITEM_H
