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
#include "websearch/websearch.h"
#include <chrono>
#include <QClipboard>
#include <QGuiApplication>
#include <QProcess>

/**************************************************************************/
void Calculator::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	QGuiApplication::clipboard()->setText(_result);
}

/**************************************************************************/
QString Calculator::Item::actionText() const
{
	return QString("Copy '%1' to clipboard.").arg(_result);
}

/**************************************************************************/
void Calculator::Item::altAction()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	QGuiApplication::clipboard()->setText(_query);
}

/**************************************************************************/
QString Calculator::Item::altActionText() const
{
	return QString("Copy '%1' to clipboard.").arg(_query);
}

/**************************************************************************/
QIcon Calculator::Item::icon() const
{
	if (QIcon::hasThemeIcon(QString::fromLocal8Bit("calc")))
		return QIcon::fromTheme(QString::fromLocal8Bit("calc"));
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}

/**************************************************************************/
void Calculator::Item::serialize(QDataStream &out) const
{
	out << _lastAccess;
}

/**************************************************************************/
void Calculator::Item::deserialize(QDataStream &in)
{
	in >> _lastAccess;
}

