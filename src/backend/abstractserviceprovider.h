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

#ifndef ABSTRACTSERVICEPROVIDER_H
#define ABSTRACTSERVICEPROVIDER_H

#include <QWidget>
#include <QString>
#include <vector>

class AbstractServiceProvider
{
public:
	class AbstractItem
	{
	public:
		enum class Action { Enter, Mod1, Mod2 };

		AbstractItem() = delete;
		AbstractItem(QString title)
			: _title(title){}
		virtual ~AbstractItem(){}

		inline  QString  title() const { return _title; }
		virtual QString  iconPath() = 0;
		virtual QString  complete() = 0;
		virtual void     action(Action) = 0;
		virtual QString  actionText(Action) = 0;
		void    fallbackAction(Action);
		QString fallbackActionText(Action);

	protected:
		const QString _title;
	};

	virtual ~AbstractServiceProvider(){ _config->deleteLater(); }
	virtual std::vector<AbstractItem*> query(QString) = 0;
	virtual QWidget* configWidget() = 0;

protected:
	QWidget* _config;
};

#endif // ABSTRACTSERVICEPROVIDER_H
