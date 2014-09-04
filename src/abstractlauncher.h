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

#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include <QString>
#include <QIcon>
#include <QWidget>

class AbstractLauncher
{
public:
	virtual ~AbstractLauncher() =  0;

	std::vector<Item> query(const QString &req) = 0;

	virtual QString name(int) = 0;
	virtual QIcon icon(int) = 0;
	virtual QString primaryActionText(int) = 0;
	virtual QString secondaryActionText(int) = 0;
	virtual void primaryAction(int) = 0;
	virtual void secondaryAction(int) = 0;

	virtual QWidget getConfigWidget() = 0;
};

#endif // ABSTRACTPLUGIN_H
