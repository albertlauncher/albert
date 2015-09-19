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
#include "configwidget.h"
#include "query.h"


namespace Applications {


/** ***************************************************************************/
void Extension::initialize() {
    qDebug() << "[Applications] Initialize extension";

    // Load settings
    QSettings s;
    _searchIndex.setFuzzy(s.value(CFG_FUZZY, CFG_FUZZY_DEF).toBool());

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        _rootDirs = v.toStringList();
    else
        restorePaths();

    // Keep the applications in sync with the OS
    _updateDelayTimer.setInterval(UPDATE_DELAY);
    _updateDelayTimer.setSingleShot(true);
    connect(&_watcher, &QFileSystemWatcher::directoryChanged, &_updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(this, &Extension::rootDirsChanged, &_updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(&_updateDelayTimer, &QTimer::timeout, this, &Extension::updateIndex);

    /* Deserialze data
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "Deserializing from" << f.fileName();
        QDataStream in(&f);
        quint64 size;
        in >> size;
        for (quint64 i = 0; i < size; ++i) {
            SharedAppPtr app(new Item(this));
            in >> app->_path >> app->_usage;
            if (getAppInfo(app->_path, app.get()))
                _index.push_back(app);
        }
        f.close();
    } else
        qWarning() << "Could not open file: " << f.fileName();*/

    // Initial update
    updateIndex();
}



/** ***************************************************************************/
void Extension::finalize() {
    qDebug() << "[Applications] Finalize extension";

    // Save settings
    QSettings s;
    s.setValue(CFG_FUZZY, _searchIndex.fuzzy());
    s.setValue(CFG_PATHS, _rootDirs);

    /* Serialze data
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "Serializing to " << f.fileName();
        QDataStream out( &f );
        out << static_cast<quint64>(_index.size());
        for (SharedAppPtr app : _index)
            out << app->_path << app->_usage;
        f.close();
    } else
        qCritical() << "Could not write to " << f.fileName();*/
}



/** ***************************************************************************/
QWidget *Extension::widget() {
    if (_widget.isNull()){
        _widget = new ConfigWidget;

        // Paths
        _widget->ui.listWidget_paths->addItems(_rootDirs);
        _widget->ui.label_info->setText(QString("%1 applications indexed.").arg(_appIndex.size()));
        connect(this, &Extension::rootDirsChanged, _widget->ui.listWidget_paths, &QListWidget::clear);
        connect(this, &Extension::rootDirsChanged, _widget->ui.listWidget_paths, &QListWidget::addItems);
        connect(_widget, &ConfigWidget::requestAddPath, this, &Extension::addDir);
        connect(_widget, &ConfigWidget::requestRemovePath, this, &Extension::removeDir);
        connect(_widget->ui.pushButton_restorePaths, &QPushButton::clicked, this, &Extension::restorePaths);

        // Fuzzy
        _widget->ui.checkBox_fuzzy->setChecked(_searchIndex.fuzzy());
        connect(_widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Info
        connect(this, &Extension::statusInfo, _widget->ui.label_info, &QLabel::setText);
    }
    return _widget;
}



/** ***************************************************************************/
void Extension::handleQuery(Query *q) {
    q->addResults(_searchIndex.search(q->searchTerm()));
}



/** ***************************************************************************/
void Extension::setFuzzy(bool b) {
    _searchIndex.setFuzzy(b);
}



/** ***************************************************************************/
void Extension::addDir(const QString & path) {
    qDebug() << "[Applications] Adding path" << path;

    QFileInfo fileInfo(path);

    // Get an absolute file path
    QString absPath = fileInfo.absoluteFilePath();

    // Check existance
    if (!fileInfo.exists()){
        QMessageBox(QMessageBox::Critical, "Error", absPath + " does not exist.").exec();
        return;
    }

    // Check type
    if(!fileInfo.isDir()){
        QMessageBox(QMessageBox::Critical, "Error", absPath + " is not a directory.").exec();
        return;
    }

    // Check if there is an identical existing path
    if (_rootDirs.contains(absPath)){
        QMessageBox(QMessageBox::Critical, "Error", absPath + " has already been indexed.").exec();
        return;
    }

    // Check if this dir is a subdir of an existing dir
    for (const QString &p: _rootDirs)
        if (absPath.startsWith(p + '/')){
            QMessageBox(QMessageBox::Critical, "Error", absPath + " is subdirectory of " + p).exec();
            return;
        }

    // Check if this dir is a superdir of an existing dir, in case delete subdir
    for (QStringList::iterator it = _rootDirs.begin(); it != _rootDirs.end();)
        if (it->startsWith(absPath + '/')){
            QMessageBox(QMessageBox::Warning, "Warning",
                        (*it) + " is subdirectory of " + absPath + ". " + (*it) + " will be removed.").exec();
            it = _rootDirs.erase(it);
        } else ++it;

    // Add the path to root dirs
    _rootDirs << absPath;

    // Inform observers
    emit rootDirsChanged(_rootDirs);
}

/** ***************************************************************************/
void Extension::removeDir(const QString & path) {
    qDebug() << "[Applications] Removing path" << path;

    // Get an absolute file path
    QString absPath = QFileInfo(path).absoluteFilePath();

    // Check existance
    if (!_rootDirs.contains(absPath))
        return;

    // Remove the path
    _rootDirs.removeAll(absPath);

    // Update the widget, if it is visible atm
    emit rootDirsChanged(_rootDirs);
}

/** ***************************************************************************/
void Extension::restorePaths() {
    qDebug() << "[Applications] Restore paths to defaults";

    // Add standard paths
    _rootDirs.clear();

    //  Add standard paths
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        addDir(path);
}


/** ***************************************************************************/
void Extension::updateIndex() {
    qDebug() << "[Applications] Updating index";

    /*
     * Go through the directory entries matching the .desktop suffix. This is
     * complex bullshit O(N²), but the amount of apps will not get that large.
     */

    QList<SharedApp> newIndex;
    for (const QString &path : _rootDirs) {
        QDirIterator fit(path, QStringList("*.desktop"), QDir::Files);
        while (fit.hasNext()) {

            // Create a new app object
            QString filePath = fit.next();
            SharedApp sa(new Application());
            if (getAppInfo(filePath, sa))
                newIndex.append(sa);

            // If it is in the index copy the usagecounter
            QList<SharedApp>::iterator it;
            it = std::find_if(_appIndex.begin(), _appIndex.end(),
                              [=](SharedApp ai){return ai->_path == filePath;});
            if (it != _appIndex.end())
                sa->_usage=it->data()->usage();
        }
    }

    // Set the new index
    _appIndex=newIndex;
    _searchIndex.clear();
    _searchIndex.build(_appIndex);

    // Finally update the watches
    _watcher.removePaths(_watcher.directories());
    _watcher.addPaths(_rootDirs);
    for (const QString &path : _rootDirs) {
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            _watcher.addPath(dit.next());
    }

    emit statusInfo(QString("Indexed %1 applications.").arg(_appIndex.size()));
}


/** ***************************************************************************/
bool Extension::getAppInfo(const QString &path, SharedApp appInfo)
{
	// TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
	QSettings s(path, QSettings::NativeFormat);
    s.setIniCodec("UTF-8");

	s.beginGroup("Desktop Entry");

	if (s.value("NoDisplay", false).toBool())
        return false;
	if (s.value("Term", false).toBool())
        return false;

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
     * locate the icon. oh funny qt-bug h8 u
     */

    qDebug() << QIcon::themeName();
    qDebug() << QIcon::themeSearchPaths();

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
            QString currentTheme = QIcon::themeName(); // missing fallback (qt-bug)
            QIcon::setThemeName("hicolor");
            if (QIcon::hasThemeIcon(iconName)){
                appInfo->_icon = QIcon::fromTheme(iconName);
                QIcon::setThemeName(currentTheme);
            }
            else
            {
                QIcon::setThemeName(currentTheme);
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
}
