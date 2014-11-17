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

#ifndef CALCULATORWIDGET_H
#define CALCULATORWIDGET_H

#include "ui_calculatorwidget.h"
#include "calculator.h"
#include <QWidget>

class CalculatorWidget : public QWidget
{
	Q_OBJECT
	Ui::CalculatorWidget ui;

public:
	explicit CalculatorWidget(Calculator*, QWidget *parent = 0);

protected:
	Calculator *_ref;
};

#endif // CALCULATORWIDGET_H
