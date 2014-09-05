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

#ifndef ABSTRACTITEM_H
#define ABSTRACTITEM_H

#include <QString>

class AbstractItem
{
public:
	AbstractItem() = delete;
	AbstractItem(QString title) : _title(title){}
	virtual ~AbstractItem(){}

	inline QString   title() const { return _title; }
	virtual void     action(int) = 0;
	virtual QString  actionText(int) = 0;
protected:
	const QString _title;
private:

};

#endif // ABSTRACTITEM_H



















//namespace Items
//{
//	/**********************************************************************//**
//	 * @brief The Item class
//	 */
//	class AbstractItem
//	{
//	public:
//		AbstractItem() = delete;
//		AbstractItem(QString name, QString path) : _name(name), _path(path), _score(0) {}
//		virtual ~AbstractItem() {}

//		virtual void  action() = 0;
////		inline QString  iconName() = 0;
//		inline QString name() const { return _name; }
//		inline QString info() const { return _path; }
//		inline double score() const { return _score; }
//		inline void setScore(double score) { _score = score; }


//	protected:
//		const QString _name;
//		const QString _path;
//		double       _score;

//	};


//	/**********************************************************************//**
//	 * @brief The Directory class
//	 */
//	class Directory : public AbstractItem
//	{
//	public:
//		Directory() = delete;
//		Directory(QString name, QString path) : AbstractItem(name, path) {}
//		~Directory() {}
//		virtual void action(){
//			pid_t pid = fork();
//			if (pid == 0) {
//				execl("/usr/bin/xdg-open", "xdg-open", _path.toStdString().c_str(), (char *)0);
//				exit(1);
//			}
//			qDebug() << QString(_name.data()) << "should be runned now. (Directory)";
//		}
//	};

//	/**********************************************************************//**
//	 * @brief The Document class
//	 */
//	class Document : public AbstractItem
//	{
//	public:
//		Document() = delete;
//		Document(QString name, QString path) : AbstractItem(name, path) {}
//		~Document() {}
//		virtual void action() {
//			pid_t pid = fork();
//			if (pid == 0) {
//				execl("/usr/bin/xdg-open", "xdg-open", _path.toStdString().c_str(), (char *)0);
//				exit(1);
//			}
//			qDebug() << QString(_name.data()) << "should be runned now. (Document)";
//		}
//	};













