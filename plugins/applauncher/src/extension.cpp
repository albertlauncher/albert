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

#include "extension.h"
#include <QDebug>
#include <QProcess>
#include <QDirIterator>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include <memory>
#include "query.h"
#include "configwidget.h"

/** ***************************************************************************/
bool Extension::addPath(const QString & path)
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
bool Extension::removePath(const QString & path)
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
void Extension::restorePaths()
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
void Extension::setFuzzy(bool b)
{
    qDebug() << "Set fuzzy search to" << b;

    _fuzzy = b;
    auto nameFunctor = [](const SharedItemPtr ip) -> QString {
        return std::static_pointer_cast<AppInfo>(ip)->_name;
    };
    if (_fuzzy)
        _search = new FuzzySearch<SharedAppPtrList> (_index, nameFunctor);
    else
        _search = new PrefixSearch<SharedAppPtrList> (_index, nameFunctor);
    _search->buildIndex();
}

/** ***************************************************************************/
void Extension::update(const QString &path)
{
    qDebug() << "Index applications in" << path;

    // Get an absolute file path
    QString absPath = QFileInfo(path).canonicalFilePath();

    /*
     * Go through the directory entries matching the .desktop suffix. If this
     * app exists in the index update it, if not create it. If the initializa-
     * tion of the appliction from the desktop file fails do not add it/remove
     * it to/from the index. This is complex bullshit O(N²), but the amount of
     * apps will not get that large.
     */
    QDirIterator fit(absPath, QStringList("*.desktop"), QDir::Files);
    while (fit.hasNext()) {
        QString filePath = fit.next();
        SharedAppPtrList::iterator it;
        it = std::find_if(_index.begin(), _index.end(), [=](SharedAppPtr ai){return ai->_path == filePath;});
        if (it == _index.end()){
            SharedAppPtr appInfo(new AppInfo(this));
            appInfo->_usage=0;
            if (getAppInfo(filePath, appInfo.get()))
                _index.push_back(appInfo);
        } else {
            if (!getAppInfo(filePath, it->get()))
                _index.erase(it);
        }
    }

    if (!_watcher.directories().contains(absPath))
        if(!_watcher.addPath(absPath)) // No clue why this should happen
            qCritical() << absPath <<  "could not be watched. Changes in this path will not be noticed.";

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
void Extension::clean()
{
    qDebug() << "Clean index";

    // Remove non existant or unwanted apps
    SharedAppPtrList::iterator ait = _index.begin();
    QStringList::iterator sit;
    while (ait != _index.end()){
        // If the app and is included in one of the listed dirs and exists
        for (sit = _paths.begin(); sit != _paths.end(); ++sit)
            if ((*ait)->_path.startsWith(*sit + '/') && QFileInfo((*ait)->_path).exists())
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
QWidget *Extension::widget()
{
    if (_widget.isNull()){
        _widget = new ConfigWidget;

        // Paths
        _widget->ui.listWidget_paths->addItems(_paths);
        _widget->ui.listWidget_paths->sortItems();
        _widget->ui.label_info->setText(QString("%1 applications.").arg(_index.size()));
        connect(_widget, &ConfigWidget::requestAddPath,
                this, &Extension::addPath);
        connect(_widget, &ConfigWidget::requestRemovePath,
                this, &Extension::removePath);
        connect(_widget->ui.pushButton_restorePaths, &QPushButton::clicked,
                this, &Extension::restorePaths);

        // Fuzzy
        _widget->ui.checkBox_fuzzy->setChecked(_fuzzy);
        connect(_widget->ui.checkBox_fuzzy, &QCheckBox::toggled,
                this, &Extension::setFuzzy);
    }
    return _widget;
}

/** ***************************************************************************/
void Extension::initialize()
{
    qDebug() << "Initialize extension 'AppLauncher'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    /* Deserialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "Deserializing from" << f.fileName();
        QTextStream in(&f);
        quint64 size;
        in >> size;
        for (quint64 i = 0; i < size; ++i) {
            SharedAppPtr app(new AppInfo(this));
            in >> app->_path >> app->_usage;
            if (getAppInfo(app->_path, app.get()))
                _index.push_back(app);
        }
        f.close();
    } else
        qWarning() << "Could not open file: " << f.fileName();

    /* Initialize the search index */
    setFuzzy(s.value(CFG_FUZZY, CFG_FUZZY_DEF).toBool());

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
}

/** ***************************************************************************/
void Extension::finalize()
{
    qDebug() << "Finalize extension 'AppLauncher'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    /* Serialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "Serializing to " << f.fileName();
        QTextStream out( &f );
        out << static_cast<quint64>(_index.size());
        for (SharedAppPtr app : _index)
            out << app->_path << app->_usage;
        f.close();
    } else
        qCritical() << "Could not write to " << f.fileName();

    /* Save settings */
    s.setValue(CFG_FUZZY, _fuzzy);
    s.setValue(CFG_PATHS, _paths);
}

/** ***************************************************************************/
void Extension::handleQuery(Query *q)
{
    q->addResults(_search->find(q->searchTerm()));
}

/** ***************************************************************************/
void Extension::action(const AppInfo& ai, const Query &, Qt::KeyboardModifiers mods) const
{
    QString cmd;
    if (mods == Qt::ControlModifier) cmd.prepend("gksu ");
    cmd.append(ai._exec);
    bool succ = QProcess::startDetached(cmd);
    if(!succ){
        QMessageBox msgBox(QMessageBox::Warning, "Error",
                           "Could not run \"" + cmd + "\"",
                           QMessageBox::Ok);
        msgBox.exec();
    }
}

/** ***************************************************************************/
QString Extension::actionText(const AppInfo& ai, const Query &, Qt::KeyboardModifiers mods) const
{
    if (mods == Qt::ControlModifier)
        return "Run " + ai._name + " as root";
    return "Run " + ai._name;
}

/** ***************************************************************************/
QString Extension::titleText(const AppInfo& ai, const Query &) const
{
    return ai._name;
}

/** ***************************************************************************/
QString Extension::infoText(const AppInfo& ai, const Query &) const
{
    return ai._altName;
}

/** ***************************************************************************/
const QIcon &Extension::icon(const AppInfo& ai) const
{
    return ai._icon;
}

/******************************************************************************/
/*                              STATIC MEMBERS                                */
/******************************************************************************/

/** ***************************************************************************/
bool Extension::getAppInfo(const QString &path, AppInfo *appInfo)
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
        appInfo->_name = v.toString();
    else
        return false;


    // Try to get the command
    v = s.value("Exec");
    if (v.isValid() && v.canConvert(QMetaType::QString)){
        appInfo->_exec = v.toString();
    } else
        return false;
    appInfo->_exec.replace("%c", appInfo->_name);
    appInfo->_exec.remove(QRegExp("%."));


    // Try to get the icon name

    // http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    // http://standards.freedesktop.org/desktop-entry-spec/latest/

    /*
     * Icon to display in file manager, menus, etc. If the name is an absolute
     * path, the given file will be used. If the name is not an absolute path,
     * the algorithm described in the Icon Theme Specification will be used to
     * locate the icon.
     */

    v = s.value("Icon");
    if (v.isValid() && v.canConvert(QMetaType::QString)){
        QString iconName = v.toString();
        // If it is a full path
        if (iconName.startsWith('/'))
            appInfo->_icon = QIcon(iconName);
        // If it is in the theme
        else if (QIcon::hasThemeIcon(iconName))
            appInfo->_icon = QIcon::fromTheme(iconName);
        else{
            // if it is in the pixmaps
            QDirIterator it("/usr/share/pixmaps", QDir::Files, QDirIterator::Subdirectories);
            bool found = false;
            while (it.hasNext()) {
                it.next();
                QFileInfo fi = it.fileInfo();
                if (fi.isFile() && (fi.fileName() == iconName || fi.baseName() == iconName)){
                    appInfo->_icon = QIcon(fi.canonicalFilePath());
                    found = true;
                    break;
                }
            }
            if (!found){
                // If it is still not found use a generic one
                qWarning() << "Unknown icon:" << iconName;
                appInfo->_icon = QIcon::fromTheme("exec");
            }
        }
    } else{
        qWarning() << "No icon specified in " << path;
        appInfo->_icon = QIcon::fromTheme("exec");
    }


    // Try to get any [localized] secondary information comment
    if (((v = s.value(QString("Comment[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("Comment[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("Comment")).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("GenericName[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("GenericName[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("GenericName")).isValid() && v.canConvert(QMetaType::QString)))
        appInfo->_altName = v.toString();
    else
        appInfo->_altName = appInfo->_exec;
	s.endGroup();

    appInfo->_path = path;
	return true;
}
