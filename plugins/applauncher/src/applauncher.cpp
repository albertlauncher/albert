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
#include "query.h"

/** ***************************************************************************/
bool AppLauncher::addPath(const QString & path)
{
    qDebug() << "Add path" << path;

    // Get an absolute file path
    QFileInfo fileInfo(path);
    QString absPath = fileInfo.canonicalFilePath();

    // Check existance && type
    if (!(fileInfo.exists() && fileInfo.isDir()))
        return false;

    // Check if there is an identical existing path
    if (_paths.contains(absPath))
        return true;

    // Check if this dir is a subdir of an existing dir
    for (const QString &p: _paths)
        if (absPath.startsWith(p + '/'))
            return true;

    // Check if this dir is a superdir of an existing dir
    for (const QString &p : _paths)
        if(p.startsWith(absPath + '/'))
            _paths.removeAll(p); // This suffices since watches will be added later anyway

    // Add the path.
    _paths << absPath;

    // Add apps and watches in this directory tree
    update(absPath);

    // And update the widget, if it is visible atm
    if (!_widget.isNull()){
        _widget->ui.listWidget_paths->addItem(absPath);
        _widget->ui.listWidget_paths->sortItems();
    }

    // Rebuild the search index
    _search->buildIndex();
    return true;
}

/** ***************************************************************************/
bool AppLauncher::removePath(const QString & path)
{
    qDebug() << "Remove path" << path;

    // Get an absolute file path
    QString absPath = QFileInfo(path).canonicalFilePath();

    // Check existance
    if (!_paths.contains(absPath))
        return true;

    // Remove the path.
    _paths.removeAll(absPath);

    // Remove apps and watches in this directory tree
    clean();

    // And update the widget, if it is visible atm
    if (!_widget.isNull()){
        QList<QListWidgetItem *> items =
        _widget->ui.listWidget_paths->findItems(absPath, Qt::MatchExactly);
        for (QListWidgetItem *item : items)
            delete item;
        _widget->ui.listWidget_paths->sortItems();
    }

    // Rebuild the search index
    _search->buildIndex();
    return true;
}

/** ***************************************************************************/
void AppLauncher::restorePaths()
{
    qDebug() << "Restore paths to default";

    // Reset state
    _index.clear();
    _paths.clear();
    if (!_watcher.directories().isEmpty())
        _watcher.removePaths(_watcher.directories());
    if (!_watcher.files().isEmpty())
        _watcher.removePaths(_watcher.files());

    //  Add standard paths
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        addPath(path);

    // And update the widget, if it is visible atm
    if (!_widget.isNull()){
        _widget->ui.listWidget_paths->clear();
        _widget->ui.listWidget_paths->addItems(_paths);
        _widget->ui.listWidget_paths->sortItems();
    }
}

/** ***************************************************************************/
void AppLauncher::setFuzzy(bool b)
{
    qDebug() << "Set fuzzy search to" << b;

    if (_fuzzy == b) return;
    _fuzzy = b;
    if (_fuzzy)
        _search = new FuzzySearch<AppInfo>(&_index, [](const AppInfo&r) -> QString {return r.name;});
    else
        _search = new PrefixSearch<AppInfo>(&_index, [](const AppInfo&r) -> QString {return r.name;});
    _search->buildIndex();
}

/** ***************************************************************************/
void AppLauncher::update(const QString &path)
{
    qDebug() << "Index applications in" << path;

    // Get an absolute file path
    QString absPath = QFileInfo(path).canonicalFilePath();

    // Update files
    QDirIterator fit(absPath, QStringList("*.desktop"), QDir::Files);
    while (fit.hasNext()) {
        QString filePath = fit.next();
        if (_index.contains(filePath))
            continue;
        AppInfo appInfo;
        appInfo.usage=0;
        if (getAppInfo(filePath, &appInfo))
            _index.insert(filePath, appInfo);
    }

    if (!_watcher.directories().contains(absPath))
        if(!_watcher.addPath(absPath)) // No clue why this should happen
            qCritical() << absPath <<  "could not be watched. Changes in this directory will not be noticed.";

    // Update subfolders if they are not watched
    QDirIterator dit(absPath, QDir::Dirs|QDir::NoDotAndDotDot);
    while (dit.hasNext()){
        QString p = dit.next();
        if (!_watcher.directories().contains(p))
            update(p);
    }

    // And update the widget, if it is visible atm
    if (!_widget.isNull())
        _widget->ui.label_info->setText(QString("%1 applications.").arg(_index.size()));
}

/** ***************************************************************************/
void AppLauncher::clean()
{
    qDebug() << "Clean index";

    // Remove non existant or unwanted apps
    AppIndex::iterator ait = _index.begin();
    QStringList::iterator sit;
    while (ait != _index.end()){
        // If the app and is included in one of the listed dirs and exists
        for (sit = _paths.begin(); sit != _paths.end(); ++sit)
            if (ait.key().startsWith(*sit + '/') && QFileInfo(ait.key()).exists())
                break;
        (sit == _paths.end()) ? ait = _index.erase(ait) : ++ait;
    }

    // All filesystemwatches have to be in/be sub of _paths
    for (const QString &w : _watcher.directories()){
        for (sit = _paths.begin(); sit != _paths.end(); ++sit)
            if (w.startsWith(*sit + '/') || w == *sit)
                break;
        if (sit == _paths.end())
            if (!_watcher.removePath(w)) // No clue why this should happen
                qCritical() <<  "Could not remove watch from:" << w;
    }

    // And update the widget, if it is visible atm
    if (!_widget.isNull())
        _widget->ui.label_info->setText(QString("%1 applications.").arg(_index.size()));
}

/******************************************************************************/
/*                          INTERFACE IMPLEMENTATION                          */
/******************************************************************************/

/** ***************************************************************************/
QWidget *AppLauncher::widget()
{
    if (_widget.isNull()){
        _widget = new ConfigWidget;

        // Paths
        _widget->ui.listWidget_paths->addItems(_paths);
        _widget->ui.listWidget_paths->sortItems();
        _widget->ui.label_info->setText(QString("%1 applications.").arg(_index.size()));

        connect(_widget, &ConfigWidget::requestAddPath,
                this, &AppLauncher::addPath);
        connect(_widget, &ConfigWidget::requestRemovePath,
                this, &AppLauncher::removePath);
        connect(_widget->ui.pushButton_restorePaths, &QPushButton::clicked,
                this, &AppLauncher::restorePaths);

        // Fuzzy
        _widget->ui.checkBox_fuzzy->setChecked(_fuzzy);

        connect(_widget->ui.checkBox_fuzzy, &QCheckBox::toggled,
                this, &AppLauncher::setFuzzy);
    }
    return _widget;
}

/** ***************************************************************************/
QString AppLauncher::name() const
{
	return tr("AppLauncher");
}

/** ***************************************************************************/
QString AppLauncher::abstract() const
{
	return tr("An extension which lets you start applications.");
}

/** ***************************************************************************/
void AppLauncher::initialize()
{

    /* Deserialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "Deserializing from" << f.fileName();
        QDataStream in(&f);
        int size;
        in >> size;
        for (int i = 0; i < size; ++i) {
            AppInfo app;
            in >> app.path >> app.name >> app.altName >> app.iconName >> app.exec >> app.usage;
            _index.insert(app.path, app);
        }
        f.close();
    } else
        qWarning() << "Could not open file: " << f.fileName();


    /* Initialize the search index */
    QSettings s(QSettings::UserScope, "albert", "albert");
    _fuzzy = s.value(CFG_FUZZY, CFG_FUZZY_DEF).toBool();
    if (_fuzzy)
        _search = new FuzzySearch<AppInfo>(&_index, [](const AppInfo&r) -> QString {return r.name;});
    else
        _search = new PrefixSearch<AppInfo>(&_index, [](const AppInfo&r) -> QString {return r.name;});


    /* Create an index of desktop files */
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList)){
        for (const QString &p : v.toStringList())
            addPath(p);
    }
    else
        restorePaths();

    /* Keep the applications in sync with the OS */
    _timer.setInterval(UPDATE_TIMEOUT);
    _timer.setSingleShot(true);

    connect(&_watcher, &QFileSystemWatcher::directoryChanged,
            [&](const QString &path){
        qDebug() << path << "changed! Starting timer";
        if (!_toBeUpdated.contains(path))
            _toBeUpdated << path;
        _timer.start();
    });

    connect(&_timer, &QTimer::timeout,
            [this](){
        qDebug() << "Timeout! Updating paths " << _toBeUpdated;
        for (const QString &s: _toBeUpdated)
            this->update(s);
        _toBeUpdated.clear();
        this->clean();
    });

    qDebug() << "Initialized applauncher with " << _index.size() << " apps.";
}

/** ***************************************************************************/
void AppLauncher::finalize()
{
    /* Serialze data */

    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "Serializing to " << f.fileName();
        QDataStream out( &f );
        out << _index.size();
        for (const AppInfo &app : _index)
            out << app.path << app.name << app.altName << app.iconName << app.exec << app.usage;
        f.close();
    } else
        qCritical() << "FATAL: Could not write to " << f.fileName();


    /* Save settings */
    QSettings s(QSettings::UserScope, "albert", "albert");
    s.setValue(CFG_FUZZY, _fuzzy);
    s.setValue(CFG_PATHS, _paths);
}

/** ***************************************************************************/
void AppLauncher::setupSession()
{
	// Get the images of the applications
}

/** ***************************************************************************/
void AppLauncher::teardownSession()
{
	// Clear the imagecache
    qDebug() << "Clean icon cache."<< _iconCache.size();
    _iconCache.clear();
}

/** ***************************************************************************/
void AppLauncher::handleQuery(Query *q)
{
	QStringList res = _search->find(q->searchTerm());
	for(const QString &k : res)
        q->addResult(QueryResult(this, k, QueryResult::Type::Interactive, _index[k].usage));
}

/** ***************************************************************************/
QString AppLauncher::titleText(const Query &, const QueryResult &qr, Qt::KeyboardModifiers ) const
{
	return _index.value(qr.rid).name;
}

/** ***************************************************************************/
QString AppLauncher::infoText(const Query &, const QueryResult &qr, Qt::KeyboardModifiers ) const
{
	return _index.value(qr.rid).altName;
}

/** ***************************************************************************/
const QIcon &AppLauncher::icon(const Query &, const QueryResult &qr, Qt::KeyboardModifiers )
{
	if (!_iconCache.contains(_index[qr.rid].iconName))
		_iconCache.insert(_index[qr.rid].iconName, getIcon(_index[qr.rid].iconName));
	return _iconCache[_index[qr.rid].iconName];
}

/** ***************************************************************************/
void AppLauncher::action(const Query &, const QueryResult &qr, Qt::KeyboardModifiers mods)
{
    ++_index[qr.rid].usage;
    qDebug() << _index[qr.rid].usage;
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

/** ***************************************************************************/
QString AppLauncher::actionText(const Query &, const QueryResult &qr, Qt::KeyboardModifiers mods) const
{
    if (mods == Qt::ControlModifier)
        return "Run " + _index[qr.rid].name + " as root";
    return "Run " + _index[qr.rid].name;
}

/******************************************************************************/
/*                              STATIC MEMBERS                                */
/******************************************************************************/

/** ***************************************************************************/
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

/** ***************************************************************************/
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

    appInfo->path = path;
	return true;
}
