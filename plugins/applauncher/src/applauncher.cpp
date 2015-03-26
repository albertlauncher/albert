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
    if (gSettings->value(CFG_PATHS).isValid())
        paths = gSettings->value(CFG_PATHS).toStringList();
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
    gSettings->setValue(CFG_PATHS, _watcher.directories());
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
    gSettings->setValue(CFG_PATHS, paths);
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
    // http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    // http://standards.freedesktop.org/desktop-entry-spec/latest/

    /*
     * Icon to display in file manager, menus, etc. If the name is an absolute
     * path, the given file will be used. If the name is not an absolute path,
     * the algorithm described in the Icon Theme Specification will be used to
     * locate the icon.
     */

    if (iconName.startsWith('/'))
        return QIcon(iconName);

    if (QIcon::hasThemeIcon(iconName))
        return QIcon::fromTheme(iconName);

    qDebug() << "unknown";
    return QIcon::fromTheme("exec");
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
