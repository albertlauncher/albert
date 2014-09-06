#include "mimeindex.h"
#include <QSettings>
#include <QDir>
#include <functional>


#include <QMimeType>
#include <QMimeDatabase>

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
 * @brief MimeIndex::MimeIndex
 */
MimeIndex::MimeIndex()
{
}

/**************************************************************************//**
 * @brief MimeIndex::~MimeIndex
 */
MimeIndex::~MimeIndex()
{

}

/**************************************************************************//**
 * @brief MimeIndex::buildIndex
 */
void MimeIndex::buildIndex()
{
	QSettings conf;
	qDebug() << "Config:" << conf.fileName();
	QStringList paths = conf.value(QString::fromLocal8Bit("paths")).toStringList();

	// Define a lambda for recursion
	std::function<void(const QString& p)> rec_dirsearch = [&] (const QString& p)
	{
		QDir dir(p);
		dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
		_index.push_back(new MimeIndexItem(dir.dirName(), dir.canonicalPath()));

		// go recursive into subdirs
		QFileInfoList list = dir.entryInfoList();
		for ( QFileInfo &fi : list)
		{
			if (fi.isDir())
				rec_dirsearch(fi.absoluteFilePath());
			_index.push_back(new MimeIndexItem(fi.completeBaseName(), fi.absoluteFilePath()));
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
QWidget *MimeIndex::configWidget()
{
	return new QWidget;
}


/*****************************************************************************/
/*****************************************************************************/
/************************** MimeIndex::MimeIndexItem *************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief MimeIndex::MimeIndexItem::iconPath
 * @return
 */
QString MimeIndex::MimeIndexItem::iconName()
{
	return QMimeDatabase().mimeTypeForFile(_uri).iconName();
}

/**************************************************************************//**
 * @brief MimeIndex::MimeIndexItem::complete
 * @return
 */
QString MimeIndex::MimeIndexItem::complete()
{
	return QString(); //TODO
}

/**************************************************************************//**
 * @brief MimeIndex::MimeIndexItem::action
 */
void MimeIndex::MimeIndexItem::action(MimeIndex::MimeIndexItem::Action)
{
	pid_t pid = fork();
	if (pid == 0) {
		pid_t sid = setsid();
		if (sid < 0) exit(EXIT_FAILURE);
		execl("/usr/bin/xdg-open", "xdg-open", _uri.toStdString().c_str(), (char *)0);
		exit(1);
	}
}

/**************************************************************************//**
 * @brief MimeIndex::MimeIndexItem::actionText
 * @return
 */
QString MimeIndex::MimeIndexItem::actionText(MimeIndex::MimeIndexItem::Action a)
{
	if (a == MimeIndex::MimeIndexItem::Action::Enter)
		return QString::fromLocal8Bit("Open '%1'.").arg(_title);

	if (a == MimeIndex::MimeIndexItem::Action::Mod1)
		return QString::fromLocal8Bit("Search for '%1' in web.").arg(_title);

	// else MimeIndex::MimeIndexItem::Action::Mod2
	return QString::fromLocal8Bit("Search for '%1' in web.").arg(_title);
}

/**************************************************************************//**
 * @brief MimeIndex::MimeIndexItem::infoText
 * @return
 */
QString MimeIndex::MimeIndexItem::infoText()
{
	return _uri;
}
