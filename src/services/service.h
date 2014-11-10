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

class Service
{
public:

	/**************************************************************************/
	class Item
	{
	public:
		enum class Mod {None, Ctrl, Meta, Alt};

		/**************************************************************************/
		struct ATimeCompare
		{
			bool operator()( Service::Item const* lhs, Service::Item  const* rhs ) const {
				return lhs->lastAccess() > rhs->lastAccess();
			}
		};

		/**************************************************************************/
		struct CaseInsensitiveCompare
		{
			inline bool operator()( Service::Item const* lhs, Service::Item const* rhs )       const {return (*this)( lhs->title(), rhs->title());}
			inline bool operator()( QString const& lhs, Service::Item const* rhs )    const {return (*this)( lhs, rhs->title() );}
			inline bool operator()( Service::Item const* lhs, QString const& rhs )    const {return (*this)( lhs->title(), rhs );}
			inline bool operator()( QString const& lhs, QString const& rhs ) const {return lhs.compare(rhs, Qt::CaseInsensitive)<0;}
		};

		/**************************************************************************/
		struct CaseInsensitiveComparePrefix
		{
			inline bool operator()( Service::Item const* pre, Service::Item const* rhs ) const {return (*this)( pre->title(), rhs->title() );}
			inline bool operator()( QString const& pre, Service::Item const* rhs ) const {return (*this)( pre, rhs->title() );}
			inline bool operator()( Service::Item const* pre, QString const& rhs ) const {return (*this)( pre->title(), rhs );}
			inline bool operator()( QString const& pre, QString const& rhs ) const	{
				QString::const_iterator a,b;
				a = pre.cbegin();
				b = rhs.cbegin();
				QChar ca,cb;
				while (a != pre.cend() && b != rhs.cend()){
					ca = a++->toLower();
					cb = b++->toLower();
					if (ca < cb)
						return true;
					if (ca > cb)
						return false;
				}
				return false;
			}
		};
		/**************************************************************************/

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

		virtual QDataStream& serialize(QDataStream &out) const = 0;
		virtual QDataStream& deserialize(QDataStream &in) = 0;

	protected:
		qint64 _lastAccess;
	};
	/**************************************************************************/

	Service() : _widget(nullptr){}
	virtual ~Service(){}
	virtual void query(const QString&, QVector<Item*>*) const noexcept = 0;
	virtual QWidget* widget() = 0;
	virtual void initialize() = 0;
	virtual void restoreDefaults() = 0;
	virtual QDataStream& serialize(QDataStream &out) const = 0;
	virtual QDataStream& deserialize(QDataStream &in) = 0;

protected:
	QWidget* _widget;
};

#endif // SERVICE_H
