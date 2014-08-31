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

#ifndef ITEM_H
#define ITEM_H

#include <QString>


///  ---  to remove
#include <QDebug>

namespace Items
{
	/**********************************************************************//**
	 * @brief The Item class
	 */
	class AbstractItem
	{
	public:
		AbstractItem() = delete;
		AbstractItem(QString name, QString path) : _name(name), _path(path), _score(0) {}
		virtual ~AbstractItem() {}

		virtual void  action() = 0;
//		inline QString  iconName() = 0;
		inline QString name() const { return _name; }
		inline QString path() const { return _path; }
		inline double score() const { return _score; }
		inline void setScore(double score) { _score = score; }


	protected:
		const QString _name;
		const QString _path;
		double       _score;

	};

	/**********************************************************************//**
	 * @brief The TerminalApp class
	 */
	class Executable : public AbstractItem
	{
	public:
		Executable() = delete;
		Executable(QString name, QString path) : AbstractItem(name, path) {}
		~Executable() {}
		virtual void action(){
			qDebug() << QString(_name.data()) << "should be runned now. (Executable)";
		}
	};

	/**********************************************************************//**
	 * @brief The DesktopApp class
	 */
	class DesktopApp : public AbstractItem
	{
	public:
		DesktopApp() = delete;
		DesktopApp(QString name, QString path) : AbstractItem(name, path) {}
		~DesktopApp() {}
		virtual void action(){
			qDebug() << QString(_name.data()) << "should be runned now. (DesktopApp)";
		}
	};

	/**********************************************************************//**
	 * @brief The Directory class
	 */
	class Directory : public AbstractItem
	{
	public:
		Directory() = delete;
		Directory(QString name, QString path) : AbstractItem(name, path) {}
		~Directory() {}
		virtual void action(){
			qDebug() << QString(_name.data()) << "should be runned now. (Directory)";
		}
	};

	/**********************************************************************//**
	 * @brief The Document class
	 */
	class Document : public AbstractItem
	{
	public:
		Document() = delete;
		Document(QString name, QString path) : AbstractItem(name, path) {}
		~Document() {}
		virtual void action(){
			qDebug() << QString(_name.data()) << "should be runned now. (Document)";
		}
	};
}
#endif // ITEM_H
