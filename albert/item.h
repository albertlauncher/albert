#ifndef ITEM_H
#define ITEM_H

#include <string>

using std::string;

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
		AbstractItem(string name, string path) : _name(name), _path(path), _score(0) {}
		virtual ~AbstractItem() {}

		virtual void  action() = 0;
		inline string name()                 { return _name; }
		inline string path()                 { return _path; }
		inline double score()                { return _score; }
		inline void   setScore(double score) { _score = score; }

	protected:
		const string _name;
		const string _path;
		double       _score;

	};

	/**********************************************************************//**
	 * @brief The TerminalApp class
	 */
	class Executable : public AbstractItem
	{
	public:
		Executable() = delete;
		Executable(string name, string path)
			: AbstractItem(name, path) {}
		~Executable() {}
		virtual void action()
		{
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
		DesktopApp(string name, string path) : AbstractItem(name, path) {}
		~DesktopApp() {}
		virtual void action()
		{
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
		Directory(string name, string path) : AbstractItem(name, path) {}
		~Directory() {}
		virtual void action()
		{
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
		Document(string name, string path)
			: AbstractItem(name, path) {}
		~Document() {}
		virtual void action()
		{
			qDebug() << QString(_name.data()) << "should be runned now. (Document)";
		}
	};
}
#endif // ITEM_H
