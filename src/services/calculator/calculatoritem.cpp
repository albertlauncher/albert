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

#include "calculatoritem.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QProcess>

void Calculator::Item::action()
{
//	switch (a) {
//	case Action::Enter:
		QGuiApplication::clipboard()->setText(_result);
//		break;
//	case Action::Alt:
//		QGuiApplication::clipboard()->setText(_query);
//		break;
//	case Action::Ctrl:
////		WebSearch::instance()->defaultSearch(_query);
//		break;
//	}
}

/**************************************************************************/
QString Calculator::Item::actionText() const
{
//	switch (a) {
//	case Action::Enter:
		return "Copy '" + _result + "' to clipboard";
//		break;
//	case Action::Alt:
//		return "Copy '" + _query + "' to clipboard";
//		break;
//	case Action::Ctrl:
////		return WebSearch::instance()->defaultSearchText(_query);
//		break;
//	}
//	// Will never happen
//	return "";
}

/**************************************************************************/
QIcon Calculator::Item::icon() const
{
	if (QIcon::hasThemeIcon(QString::fromLocal8Bit("calc")))
		return QIcon::fromTheme(QString::fromLocal8Bit("calc"));
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}
