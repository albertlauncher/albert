// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#define CONFIG_PATHS "AppIndex/Paths"

#include "applauncher.h"
#include <QDebug>
#include <QProcess>
#include <QDirIterator>
#include <QStandardPaths>
#include <QMessageBox>
#include "settings.h"
#include "query.h"
#include "configwidget.h"

/****************************************************************************///
QString AppLauncher::name() const
{
	return tr("AppLauncher");
}

/****************************************************************************///
QString AppLauncher::abstract() const
{
	return tr("An extension which lets you start applications.");
}

/****************************************************************************///
void AppLauncher::initialize()
{
	// Check settings for paths
	QStringList paths;
	if (gSettings->value(CONFIG_PATHS).isValid())
		paths = gSettings->value(CONFIG_PATHS).toStringList();
	else{
		paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	}

	// Get addistional subdirectories
	for (const QString& path : paths) {
		QDirIterator it(path, QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext())
			paths << it.next();
	}

	// Watch the folders containing the apps for changes
	QStringList failedPaths = _watcher.addPaths(paths);
	if (!failedPaths.isEmpty())
		qWarning() << failedPaths << "could not be watched!";
	// TODO update the ui? Save this to settings?

	// Get the serialized data
	// Deserialize the data	// Deserialize the index
//	int size;
//	in >> size;
//	AppIndex::Item *it;
//	emit beginBuildIndex();
//	for (int i = 0; i < size; ++i) {
//		it = new AppIndex::Item;
//		it->deserialize(in);
//		it->_icon = getIcon(it->_iconName);
//		_index.push_back(it);
//	}
//	in >> _lastAccess >> _name >> _exec >> _iconName >> _info >> _term;

	// Cleanup and update the database
	cleanApplications();
	for (const QString& path : paths)
		updateApplications(path);

	// Initialize the search index
	_search = new PrefixSearch<AppInfo>(&_index, [](const AppInfo&r) -> QString {return r.name;});
//	if(gSettings->value("Fuzzy", false).toBool())
//		_search = new FuzzySearch<AppInfo>(&_index, [](const AppInfo&r) -> QString {return r.name;});
//	else
	_search->buildIndex();

	// Rebuild index if watcher signaled a change
	connect(&_watcher, &QFileSystemWatcher::directoryChanged,
			this, &AppLauncher::updateApplications);

	qDebug() << "Loaded " << _index.size() << " apps.";
}

/****************************************************************************///
void AppLauncher::finalize()
{
	// Save settings
	gSettings->setValue(CONFIG_PATHS, _watcher.directories());
//	gSettings.setValue("Fuzzy", dynamic_cast<FuzzySearch*>(_search) != nullptr);

	// Serialize the data
//	// Serialize data
//	out << _index.size();
//	for (Service::Item *it : _index)
//		static_cast<AppIndex::Item*>(it)->serialize(out);
//	out << _lastAccess << _name << _exec << _iconName << _info << _term;


	// Rebuild index if watcher signaled a change
	disconnect(&_watcher, &QFileSystemWatcher::directoryChanged,
			   this, &AppLauncher::updateApplications);
}

/****************************************************************************///
void AppLauncher::restorePaths()
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	_watcher.removePaths(_watcher.files());
	_watcher.removePaths(_watcher.directories());
	_watcher.addPaths(paths);
	gSettings->setValue("AppIndex/Paths", paths);
//	updateApplications();
}

/****************************************************************************///
void AppLauncher::addPath(const QString & s)
{
//	bool failed = _watcher.addPath(s);
//	if (!failed)
//		buildIndex();
	//	return failed;
}

/****************************************************************************///
void AppLauncher::serialize(const QString &path)
{

}

/****************************************************************************///
void AppLauncher::deserialize(const QString &path)
{

}

/****************************************************************************///
void AppLauncher::removePath(const QString & s)
{
//	bool failed = _watcher.removePath(s);
//	if (!failed)
//		buildIndex();
//	return failed;
}
/****************************************************************************///
void AppLauncher::setupSession()
{
	// Get the images of the applications
}

/****************************************************************************///
void AppLauncher::teardownSession()
{
	// Clear the imagecache
    qDebug() << "Clean icon cache."<< _iconCache.size();
    _iconCache.clear();
}

/****************************************************************************///
QWidget *AppLauncher::widget()
{
	// NO OWNERSHIP HERE
	ConfigWidget *cw = new ConfigWidget;
//	cw->ui.listWidget_paths->addItems();
//	connect(cw, &ConfigWidget::pathAdded,
//			this, AppLauncher::updateApplications);
//	connect(cw, &ConfigWidget::pathRemoved,
//			this, AppLauncher::updateApplications);
//	connect(cw, &ConfigWidget::pathsRestored,
//			this, AppLauncher::updateApplications);
	return cw;
}

/****************************************************************************///
void AppLauncher::cleanApplications()
{
	// Remove nonexisting apps
	for (const QString &s : _index.keys())
		if (!QFileInfo(s).exists())
			_index.remove(s);
}

/****************************************************************************///
void AppLauncher::updateApplications(const QString &path)
{
	QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString filePath = it.next();
		if (_index.contains(filePath)) continue;
		AppInfo appInfo;
		appInfo.usage=0;
		if (getAppInfo(filePath, &appInfo))
			_index.insert(filePath, appInfo);
	}
}

/****************************************************************************///
void AppLauncher::handleQuery(Query *q)
{
	QStringList res = _search->find(q->searchTerm());
	for(const QString &k : res)
		q->addResult(QueryResult(this, k, QueryResult::Type::Interactive, 255));
}

/****************************************************************************///
QString AppLauncher::titleText(const Query &q, const QueryResult &qr, Qt::KeyboardModifiers mods) const
{
	return _index.value(qr.rid).name;
}

/****************************************************************************///
QString AppLauncher::infoText(const Query &q, const QueryResult &qr, Qt::KeyboardModifiers mods) const
{
	return _index.value(qr.rid).altName;
}

/****************************************************************************///
const QIcon &AppLauncher::icon(const Query &q, const QueryResult &qr, Qt::KeyboardModifiers mods)
{
	if (!_iconCache.contains(_index[qr.rid].iconName))
		_iconCache.insert(_index[qr.rid].iconName, getIcon(_index[qr.rid].iconName));
	return _iconCache[_index[qr.rid].iconName];
}

/****************************************************************************///
void AppLauncher::action(const Query &q, const QueryResult &qr, Qt::KeyboardModifiers mods)
{
    ++_index[qr.rid].usage;
    QString cmd = _index[qr.rid].exec;
    if (mods == Qt::ControlModifier)
        cmd.prepend("gksu ");
    bool succ = QProcess::startDetached(cmd);
    if(!succ){
        QMessageBox msgBox(QMessageBox::Warning, "Error",
                           "Could not run \"" + cmd + "\"",
                           QMessageBox::Ok);
        msgBox.exec();
    }
}

/****************************************************************************///
QString AppLauncher::actionText(const Query &q, const QueryResult &qr, Qt::KeyboardModifiers mods) const
{
    if (mods == Qt::ControlModifier)
        return "Run " + _index[qr.rid].name + " as root";
    return "Run " + _index[qr.rid].name;
}

/**************************************************************************/
QIcon AppLauncher::getIcon(const QString &iconName)
{
	/* Icons and themes are looked for in a set of directories. By default,
	 * apps should look in $HOME/.icons (for backwards compatibility), in
	 * $XDG_DATA_DIRS/icons and in /usr/share/pixmaps (in that order).
	 * Applications may further add their own icon directories to this list,
	 * and users may extend or change the list (in application/desktop specific
	 * ways).In each of these directories themes are stored as subdirectories.
	 * A theme can be spread across several base directories by having
	 * subdirectories of the same name. This way users can extend and override
	 * system themes.
	 *
	 * In order to have a place for third party applications to install their
	 * icons there should always exist a theme called "hicolor" [1]. The data
	 * for the hicolor theme is available for download at:
	 * http://www.freedesktop.org/software/icon-theme/. Implementations are
	 * required to look in the "hicolor" theme if an icon was not found in the
	 * current theme.*/







    qDebug() << "---------------------";
    qDebug() << QIcon::themeName()<< iconName;

    // PATH
    if (iconName.startsWith('/')){
        qDebug() << "BY PATH";
        return QIcon(iconName);
    }

    // Strip suffix
    QString strippedIconName = iconName;
    if (strippedIconName.contains('.'))
        strippedIconName =  strippedIconName.section('.',0,-2);

    if (QIcon::hasThemeIcon(strippedIconName)) {// HORRIBLY BUGGY QTBUG-42239 CHNAGE WITH Qt5.4
        qDebug() << "BY QIcon::fromTheme";
        return QIcon::fromTheme(strippedIconName);
    }

//    // Implementation for desktop specs
//    QStringList paths, themes, sizes;
//    paths << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
//    themes << QIcon::themeName() << "hicolor";
//    sizes << "scalable" << "512x512" << "384x384" << "256x256" << "192x192"
//          << "128x128" << "96x96" << "72x72" << "64x64" << "48x48" << "42x42"
//          << "36x36" << "32x32" << "24x24" << "22x22" << "16x16" << "8x8";
//	for (const QString & p : paths){
//		for (const QString & t : themes){
//			for (const QString & s : sizes){
//				qDebug() << QString("%1/icons/%2/%3/apps").arg(p,t,s);
//				QDirIterator it(QString("%1/icons/%2/%3/apps").arg(p,t,s), QDirIterator::FollowSymlinks);
//				while (it.hasNext()){
//					it.next();
//					QFileInfo fi = it.fileInfo();
//					if (fi.isFile() && fi.baseName() == strippedIconName)
//						return QIcon(fi.canonicalFilePath());
//				}
//			}
//		}
//	}

    // PIXMAPS
    QDirIterator it("/usr/share/pixmaps", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo fi = it.fileInfo();
        if (fi.isFile() && fi.baseName() == strippedIconName){
            qDebug() << "BY PIXMAPS";
            return QIcon(fi.canonicalFilePath());
        }
    }

    //UNKNOWN
    qDebug() << "unknown";
    return QIcon::fromTheme("unknown");

















    //TEST

    //    auto LookupIcon = [](QString iconName, QString themeName){


    //    auto fromTheme = [](QString iconName, QString themeName){
    //        QString path = LookupIcon (iconName, themeName);
    //        if (!path.isEmpty())
    //          return path;
    //          if theme has parents
    //            parents = theme.parents
    //          for parent in parents {
    //            filename = FindIconHelper (icon, size, parent)
    //            if filename != none
    //              return filename
    //          }
    //          return none
    //        }
    //    };

    //    QString path = fromTheme(iconName, QIcon::themeName());
    //    if (!path.isEmpty())
    //      return path;

    //    QString path = fromTheme(iconName, "hicolor");
    //    if (!path.isEmpty())
    //      return path;

    //    return QIcon::fromTheme("unknown");









//	// BACKUP
//	if (iconName.startsWith('/'))
//		return QIcon(iconName);
////	QString tmp = iconName;
////	if (tmp.contains('.'))
////		tmp =  tmp.section('.',0,-2);
//	if (QIcon::hasThemeIcon(iconName))
//		return QIcon::fromTheme(iconName);
//	return QIcon::fromTheme("unknown");
}

/****************************************************************************///
bool AppLauncher::getAppInfo(const QString &path, AppInfo *appInfo)
{
	// TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
	QSettings s(path, QSettings::NativeFormat);
    s.setIniCodec("UTF-8");

	s.beginGroup("Desktop Entry");

	if (s.value("NoDisplay", false).toBool()) return false;
	if (s.value("Term", false).toBool()) return false;

    QVariant v;
	QString locale = QLocale().name();
    QString shortLocale = QLocale().name();
    shortLocale.truncate(2);


	// Try to get the (localized name)
    if (((v = s.value(QString("Name[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("Name[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("Name")).isValid() && v.canConvert(QMetaType::QString)))
        appInfo->name = v.toString();
    else
        return false;


    // Try to get the command
    v = s.value("Exec");
    if (v.isValid() && v.canConvert(QMetaType::QString)){
        appInfo->exec = v.toString();
    } else
        return false;
    appInfo->exec.replace("%c", appInfo->name);
    appInfo->exec.remove(QRegExp("%."));


    // Try to get the icon name
    v = s.value("Icon");
    if (v.isValid() && v.canConvert(QMetaType::QString)){
        appInfo->iconName = v.toString();
    } else
        return false;


    // Try to get any [localized] secondary information comment
    if (((v = s.value(QString("Comment[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("Comment[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("Comment")).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("GenericName[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("GenericName[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("GenericName")).isValid() && v.canConvert(QMetaType::QString)))
		appInfo->altName = v.toString();
    else
        appInfo->altName = appInfo->exec;

	s.endGroup();
	return true;
}




/*
 *
 * TRASH
 * TODO: Move to seperate plugin (Platform: Win)
 *
 */

//#ifdef Q_OS_WIN
//	QFileIconProvider fip;
//	qDebug() << iconName;
//	return fip.icon(QFileInfo(iconName));
////	HICON ico = ExtractIconW(nullptr, this->_exec.toStdWString().c_str(), 0);
////	DestroyIcon(ico);
////	return QIcon(QPixmap::fromWinHICON(ico));
//#endif

//#ifdef Q_OS_WIN
// TODO QTBUG-40565
//	//	for ( const QString &p : _paths) {
//		QDirIterator it(
//					"C:/Documents and Settings/All Users/Start Menu/Programs",
//				   QDir::Files|QDir::NoDotAndDotDot,
//					QDirIterator::Subdirectories);
//		while (it.hasNext()) {
//			it.next();
//			QFileInfo fi = it.fileInfo();
//			if (fi.isExecutable())
//			{
//				Item *i = new Item;
//				qDebug()<< fi.baseName();
//				i->_name     = fi.baseName();
//				if (fi.isSymLink())
//					fi.setFile(fi.symLinkTarget());
//				qDebug()<< fi.fileName();
//				qDebug()<< fi.canonicalFilePath();
//				i->_info     = fi.canonicalFilePath();
//				i->_icon     = getIcon(fi.canonicalFilePath());
//				i->_exec     = QString("\"%1\"").arg(fi.canonicalFilePath());
//				i->_term     = false;
//				_index.push_back(i);
//			}
//		}
//    }
//	HKEY hUninstKey = NULL;
//	HKEY hAppKey = NULL;
//	WCHAR sAppKeyName[1024];
//	WCHAR sSubKey[1024];
//	WCHAR sDisplayName[1024];
//	WCHAR *sRoot = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
//	long lResult = ERROR_SUCCESS;
//	DWORD dwType = KEY_ALL_ACCESS;
//	DWORD dwBufferSize = 0;
//	//Open the "Uninstall" key.
//	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, sRoot, 0, KEY_READ, &hUninstKey) != ERROR_SUCCESS)
//	{
//		return;
//	}
//	for(DWORD dwIndex = 0; lResult == ERROR_SUCCESS; dwIndex++)
//	{
//		//Enumerate all sub keys...
//		dwBufferSize = sizeof(sAppKeyName);
//		if((lResult = RegEnumKeyEx(hUninstKey, dwIndex, sAppKeyName,
//			&dwBufferSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
//		{
//			//Open the sub key.
//			wsprintf(sSubKey, L"%s\\%s", sRoot, sAppKeyName);
//			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS) {
//				RegCloseKey(hAppKey);
//				RegCloseKey(hUninstKey);
//				return;
//			}
//			//Get the display name value from the application's sub key.
//			dwBufferSize = sizeof(sDisplayName);
//			if(RegQueryValueEx(hAppKey, L"DisplayName", NULL,
//				&dwType, (unsigned char*)sDisplayName, &dwBufferSize) == ERROR_SUCCESS) {
//				qDebug() << QString::fromWCharArray(sAppKeyName);
//				qDebug() << QString::fromWCharArray(sSubKey);
//				qDebug() << QString::fromWCharArray(sDisplayName);
//				Item *i = new Item;
//				i->_name     = QString::fromWCharArray(sDisplayName);
//				i->_info     = "";
//				i->_iconName = "";
//				i->_exec     = "";
//				i->_term     = false;
//				_index.push_back(i);
//			}
//			RegCloseKey(hAppKey);
//		}
//	}
//	RegCloseKey(hUninstKey);
//#endif
