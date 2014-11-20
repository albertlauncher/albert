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

#ifndef SERVICE_H
#define SERVICE_H

#include <QString>
#include <QWidget>
#include <QVector>
#include <QIcon>
#include <QDataStream>
#include <QSettings>

class Service
{
public:
	class Item
	{
	public:
		struct ATimeCompare{
			bool operator()( Service::Item const* lhs, Service::Item  const* rhs ) const{
				return lhs->lastAccess() > rhs->lastAccess();
			}
		};

		Item() : _lastAccess(0) {}
		virtual ~Item(){}

		qint64 lastAccess() const {return _lastAccess;}

		virtual QString title() const = 0;
		virtual QIcon icon() const = 0;
		virtual QString infoText() const = 0;
		virtual QString complete() const = 0;

		virtual void action() = 0;
		virtual QString actionText() const = 0;
		virtual void altAction() = 0;
		virtual QString altActionText() const = 0;

		virtual void serialize(QDataStream &out) const = 0;
		virtual void deserialize(QDataStream &in) = 0;

	protected:
		qint64 _lastAccess;
	};
	/**************************************************************************/

	Service(){}
	virtual ~Service(){}

	virtual void initialize() = 0;

	virtual void saveSettings(QSettings &s) const = 0;
	virtual void loadSettings(QSettings &s) = 0;
	virtual void serilizeData(QDataStream &out) const = 0;
	virtual void deserilizeData(QDataStream &in) = 0;


	virtual void query(const QString&, QVector<Item*>*) const = 0;
	virtual void queryFallback(const QString&, QVector<Item*>*) const = 0;

	virtual QWidget* widget() = 0;
	virtual QString moduleName() = 0;
};

#endif // SERVICE_H
