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

#ifndef HISTORY_H
#define HISTORY_H

#include <QObject>
#include <QStringList>
#include <QSettings>
#include <QDataStream>

class History : public QObject
{
	Q_OBJECT
	friend class SettingsWidget;

public:
	History();

	void saveSettings(QSettings &s) const;
	void loadSettings(QSettings &s);
	void serilizeData(QDataStream &out) const;
	void deserilizeData(QDataStream &in);

	void insert(QString);
	bool hasNext() const;
	const QString& next();

private:
	QStringList           _data;
	int                   _max;
	QStringList::iterator _it;

public slots:
	void reset();
};

#endif // HISTORY_H
