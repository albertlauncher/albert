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

#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include "item.h"
#include "extension.h"
#include "configwidget.h"
#include "interfaces/iextensionmanager.h"
#include "app.h"
#include "query.h"

namespace Applications {


/** ***************************************************************************/
Extension::Extension() : _updateOnTearDown(false) {

}



/** ***************************************************************************/
Extension::~Extension(){

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
void Extension::initialize(IExtensionManager *em) {
    qDebug() << "[Applications] Initialize extension";

    _manager = em;

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

    // Deserialze data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "[Applications] Deserializing from" << dataFile.fileName();
        QDataStream in(&dataFile);
        App app;
        int size;
        in >> size;
        for (int i = 0; i < size; ++i) {
            in >> app.path >> app.usage;
//            if (getAppInfo(app.path, &app)) // Not necessry if index is updated after this
            _appIndex.push_back(new App(app));
        }
        dataFile.close();
    } else
        qWarning() << "Could not open file: " << dataFile.fileName();

    // Initial update
    updateIndex();

    qDebug() << "[Applications] Extension initialized";
}



/** ***************************************************************************/
void Extension::finalize() {
    qDebug() << "[Applications] Finalize extension";

    // Save settings
    QSettings s;
    s.setValue(CFG_FUZZY, _searchIndex.fuzzy());
    s.setValue(CFG_PATHS, _rootDirs);

    // Serialze data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[Applications] Deserializing from" << dataFile.fileName();
        QDataStream out( &dataFile );
        out << _appIndex.size();
        for (App *app : _appIndex)
            out << app->path << app->usage;
        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "[Applications] Extension finalized";
}



/** ***************************************************************************/
void Extension::teardownSession()
{
    if (_updateOnTearDown){
        updateIndex();
        _updateOnTearDown=false;
    }
}



/** ***************************************************************************/
void Extension::handleQuery(IQuery *q) {
    // Search for matches. Lock memory against scanworker
    QList<IIndexable*> indexables = _searchIndex.search(q->searchTerm());

    // Add results to query. This cast is safe since index holds files only
    for (IIndexable *obj : indexables)
        q->add(new Item(static_cast<App*>(obj), this , q));
}



/** ***************************************************************************/
void Extension::addDir(const QString & dirPath) {
    qDebug() << "[Applications] Adding dir" << dirPath;

    QFileInfo fileInfo(dirPath);

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
void Extension::removeDir(const QString &dirPath) {
    qDebug() << "[Applications] Removing path" << dirPath;

    // Get an absolute file path
    QString absPath = QFileInfo(dirPath).absoluteFilePath();

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
    if (_manager->sessionIsActive()){
        _updateOnTearDown=true;
        return;
    }

    qDebug() << "[Applications] Scanning applications...";
    emit statusInfo(QString("Indexing applications...").arg(_appIndex.size()));

    /*
     * Go through the directory entries matching the .desktop suffix. This is
     * complex bullshit O(N²), but the amount of apps will not get that large.
     */

    QList<App*> newIndex;
    App app;
    for (const QString &path : _rootDirs) {
        QDirIterator fIt(path, QStringList("*.desktop"), QDir::Files);
        while (fIt.hasNext()) {
            // Try to read the desktop file
            if (!getAppInfo(fIt.next(), &app))
                continue;

            // If it is alread in the index copy the usagecounter
            QList<App*>::iterator it;
            it = std::find_if(_appIndex.begin(), _appIndex.end(),
                              [&](App* app){ return app->path == fIt.filePath();});
            if (it != _appIndex.end())
                app.usage = (*it)->usage;

            // Everthing okay, index it
            newIndex.append(new App(app));
        }
    }

    // Build the offline index
    _searchIndex.clear();
    for (App *app : newIndex)
        _searchIndex.add(app);

    // Set the new index
    std::swap(_appIndex, newIndex);

    for (App *app : newIndex)
        delete app;

    // Finally update the watches
    _watcher.removePaths(_watcher.directories());
    _watcher.addPaths(_rootDirs);
    for (const QString &path : _rootDirs) {
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            _watcher.addPath(dit.next());
    }

    qDebug() << "[Applications] Scanning applications done.";
    emit statusInfo(QString("Indexed %1 applications.").arg(_appIndex.size()));
}



/** ***************************************************************************/
bool Extension::fuzzy() {
    return _searchIndex.fuzzy();
}



/** ***************************************************************************/
void Extension::setFuzzy(bool b) {
    _searchIndex.setFuzzy(b);
}



/** ***************************************************************************/
bool Extension::getAppInfo(const QString &path, App *app)
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
        app->name = v.toString();
    else
        return false;


    // Try to get the command
    v = s.value("Exec");
    if (v.isValid() && v.canConvert(QMetaType::QString)){
        app->exec = v.toString();
    } else
        return false;
    app->exec.replace("%c", app->name);
    app->exec.remove(QRegExp("%."));


    // Try to get the icon name

    // http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    // http://standards.freedesktop.org/desktop-entry-spec/latest/

    /*
     * Icon to display in file manager, menus, etc. If the name is an absolute
     * path, the given file will be used. If the name is not an absolute path,
     * the algorithm described in the Icon Theme Specification will be used to
     * locate the icon. oh funny qt-bug h8 u
     */

    v = s.value("Icon");
    if (v.isValid() && v.canConvert(QMetaType::QString)){
        QString iconName = v.toString();
        // If it is a full path
        if (iconName.startsWith('/'))
            app->icon = QIcon(iconName);
        // If it is in the theme
        else if (QIcon::hasThemeIcon(iconName))
            app->icon = QIcon::fromTheme(iconName);
        else{
            QString currentTheme = QIcon::themeName(); // missing fallback (qt-bug)
            QIcon::setThemeName("hicolor");
            if (QIcon::hasThemeIcon(iconName)){
                app->icon = QIcon::fromTheme(iconName);
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
                        app->icon = QIcon(fi.canonicalFilePath());
                        found = true;
                        break;
                    }
                }
                if (!found){
                    // If it is still not found use a generic one
                    qWarning() << "Unknown icon:" << iconName;
                    app->icon = QIcon::fromTheme("exec");
                }
            }
        }
    } else{
        qWarning() << "No icon specified in " << path;
        app->icon = QIcon::fromTheme("exec");
    }


    // Try to get any [localized] secondary information comment
    if (((v = s.value(QString("Comment[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("Comment[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("Comment")).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("GenericName[%1]").arg(locale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value(QString("GenericName[%1]").arg(shortLocale))).isValid() && v.canConvert(QMetaType::QString))
            || ((v = s.value("GenericName")).isValid() && v.canConvert(QMetaType::QString)))
        app->altName = v.toString();
    else
        app->altName = app->exec;
	s.endGroup();

    app->path = path;
	return true;
}
}
