#include "filesystemindex.h"
#include <QSettings>
#include <functional>


#include <QMimeType>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QUrl>

//REMOVE
#include <QDebug>

bool lexicographically (AbstractServiceProvider::AbstractItem*  i, AbstractServiceProvider::AbstractItem* j)
{
	return 0 > i->title().compare(j->title(), Qt::CaseInsensitive);
}

/*****************************************************************************/
/*****************************************************************************/
/*********************************** MimeIndex *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief FileSystemIndex::buildIndex
 */
void FileSystemIndex::buildIndex()
{
	QSettings conf;
	qDebug() << "Config:" << conf.fileName();
	QStringList paths = conf.value(QString::fromLocal8Bit("paths")).toStringList();

	// Define a lambda for recursion
	std::function<void(const QString& p)> rec_dirsearch = [&] (const QString& p)
	{
		QDir dir(p);
		dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
		_index.push_back(new DirIndexItem(dir));

		// go recursive into subdirs
		QFileInfoList list = dir.entryInfoList();
		for ( QFileInfo &fi : list)
		{
			if (fi.isDir())
				rec_dirsearch(fi.absoluteFilePath());
			_index.push_back(new FileIndexItem(fi));
		}
	};

	// Finally do this recursion for all paths
	for ( auto path : paths)
		rec_dirsearch(path);

	std::sort(_index.begin(), _index.end(), lexicographically);
	qDebug() << "Found" << _index.size() << "items.";
}

/**************************************************************************//**
 * @brief MimeIndex::configWidget
 * @return
 */
QWidget *FileSystemIndex::configWidget()
{
	return new QWidget;
}

/*****************************************************************************/
/*****************************************************************************/
/******************************** DirIndexItem *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief FileSystemIndex::DirIndexItem::action
 * @param a
 */
void FileSystemIndex::DirIndexItem::action(Action a)
{
	if (a == Action::Enter || a == Action::Ctrl) {
		QDesktopServices::openUrl(QUrl("file://" + uri()));
		return;
	}

	// else Action::Alt
	fallbackAction(a);

}

/**************************************************************************//**
 * @brief FileSystemIndex::DirIndexItem::actionText
 * @param a
 * @return
 */
QString FileSystemIndex::DirIndexItem::actionText(Action a) const
{
	if (a == Action::Enter || a == Action::Ctrl)
		return QString::fromLocal8Bit("Open '%1' in default file browser.").arg(_title);

	// else Action::Alt
	return QString::fromLocal8Bit("Search for '%1' in web.").arg(_title);
}

/*****************************************************************************/
/*****************************************************************************/
/******************************* FileIndexItem *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief FileSystemIndex::FileIndexItem::action
 * @param a
 */
void FileSystemIndex::FileIndexItem::action(Action a)
{
	if (a == Action::Enter) {
		pid_t pid = fork();
		if (pid == 0) {
			pid_t sid = setsid();
			if (sid < 0) exit(EXIT_FAILURE);
			execl("/usr/bin/xdg-open", "xdg-open", uri().toStdString().c_str(), (char *)0);
			exit(1);
		}
		return;
	}

	if (a == Action::Ctrl){
		QDesktopServices::openUrl(QUrl("file://" + uri()));
		return;
	}

	// else Action::Alt
	fallbackAction(a);
}

/**************************************************************************//**
 * @brief FileSystemIndex::FileIndexItem::actionText
 * @param a
 * @return
 */
QString FileSystemIndex::FileIndexItem::actionText(Action a) const
{
	if (a == Action::Enter)
		return QString::fromLocal8Bit("Open '%1' with default application.").arg(_title);

	if (a == Action::Ctrl)
		return QString::fromLocal8Bit("Open '%1' in default file browser.").arg(_title);

	// else Action::Alt
	return QString::fromLocal8Bit("Search for '%1' in web.").arg(_title);
}

