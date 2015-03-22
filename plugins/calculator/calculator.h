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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "abstractservice.h"
#include "muParser.h"
#include <QLocale>

class Calculator : public Service
{
	friend class CalculatorWidget;

public:
	class Item;

	Calculator();
	~Calculator();

	QWidget* widget() override;
	inline QString moduleName() override {return "Calculator";}

	void initialize() override;

	void saveSettings(QSettings &s) const override;
	void loadSettings(QSettings &s) override;
	void serilizeData(QDataStream &out) const override;
	void deserilizeData(QDataStream &in) override;

	void query(const QString&, QVector<Service::Item*>*) const override;
	void queryFallback(const QString&, QVector<Service::Item*>*) const override;

protected:
	Item *_theOneAndOnly;
	mu::Parser *_p;
	QLocale loc;
};

#endif // CALCULATOR_H
