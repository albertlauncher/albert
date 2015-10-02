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

#include <QStandardPaths>
#include <QThreadPool>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <memory>
#include "extension.h"
#include "configwidget.h"
#include "application.h"
#include "indexer.h"
#include "query.h"


/** ***************************************************************************/
QWidget *Applications::Extension::widget() {
    if (_widget.isNull()) {
        _widget = new ConfigWidget;

        // Paths
        _widget->ui.listWidget_paths->addItems(_rootDirs);
        _widget->ui.label_info->setText(QString("%1 Applications indexed.").arg(_appIndex.size()));
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
void Applications::Extension::initialize() {
    qDebug() << "[Applications] Initialize extension";

    // Add the userspace icons dir which is not covered in the specs
    QFileInfo userSpaceIconsPath(QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath("icons"));
    if (userSpaceIconsPath.exists() && userSpaceIconsPath.isDir())
        QIcon::setThemeSearchPaths(QStringList(userSpaceIconsPath.absoluteFilePath()) << QIcon::themeSearchPaths());

    // Load settings
    QSettings s;
    _searchIndex.setFuzzy(s.value(CFG_FUZZY, CFG_FUZZY_DEF).toBool());

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        _rootDirs = v.toStringList();
    else
        restorePaths();

    // Keep the Applications in sync with the OS
    _updateDelayTimer.setInterval(UPDATE_DELAY);
    _updateDelayTimer.setSingleShot(true);
    connect(&_watcher, &QFileSystemWatcher::directoryChanged, &_updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(this, &Extension::rootDirsChanged, &_updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(&_updateDelayTimer, &QTimer::timeout, this, &Extension::updateIndex);

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "[Applications] Deserializing from" << dataFile.fileName();
        QDataStream in(&dataFile);
        quint64 size;
        in >> size;
        QString path;
        short usage;
        for (quint64 i = 0; i < size; ++i) {
            in >> path >> usage;
            // index is updated after this
            _appIndex.push_back(std::make_shared<Application>(path, usage));
        }
        dataFile.close();
    } else
        qWarning() << "Could not open file: " << dataFile.fileName();

    // Initial update
    updateIndex();

    qDebug() << "[Applications] Extension initialized";
}



/** ***************************************************************************/
void Applications::Extension::finalize() {
    qDebug() << "[Applications] Finalize extension";

    // Save settings
    QSettings s;
    s.setValue(CFG_FUZZY, _searchIndex.fuzzy());
    s.setValue(CFG_PATHS, _rootDirs);

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[Applications] Serializing to" << dataFile.fileName();
        QDataStream out( &dataFile );

        // Lock index against indexer
        QMutexLocker locker(&_indexAccess);

        // Serialize
        out << static_cast<quint64>(_appIndex.size());
        for (shared_ptr<Application> &de : _appIndex)
            out << de->path() << de->usage();

        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "[Applications] Extension finalized";
}



/** ***************************************************************************/
void Applications::Extension::setupSession(){

}



/** ***************************************************************************/
void Applications::Extension::teardownSession() {

}



/** ***************************************************************************/
void Applications::Extension::handleQuery(shared_ptr<Query> query) {
    // Search for matches. Lock memory against scanworker
    _indexAccess.lock();
    vector<shared_ptr<IIndexable>> indexables = _searchIndex.search(query->searchTerm());
    _indexAccess.unlock();

    // Add results to query. This cast is safe since index holds files only
    for (shared_ptr<IIndexable> obj : indexables)
        query->addMatch(std::static_pointer_cast<Application>(obj),
                        std::static_pointer_cast<Application>(obj)->usage());
}



/** ***************************************************************************/
void Applications::Extension::addDir(const QString & dirPath) {
    qDebug() << "[Applications] Adding dir" << dirPath;

    QFileInfo fileInfo(dirPath);

    // Get an absolute file path
    QString absPath = fileInfo.absoluteFilePath();

    // Check existance
    if (!fileInfo.exists()) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " does not exist.").exec();
        return;
    }

    // Check type
    if(!fileInfo.isDir()) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " is not a directory.").exec();
        return;
    }

    // Check if there is an identical existing path
    if (_rootDirs.contains(absPath)) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " has already been indexed.").exec();
        return;
    }

    // Check if this dir is a subdir of an existing dir
    for (const QString &p: _rootDirs)
        if (absPath.startsWith(p + '/')) {
            QMessageBox(QMessageBox::Critical, "Error", absPath + " is subdirectory of " + p).exec();
            return;
        }

    // Check if this dir is a superdir of an existing dir, in case delete subdir
    for (QStringList::iterator it = _rootDirs.begin(); it != _rootDirs.end();)
        if (it->startsWith(absPath + '/')) {
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
void Applications::Extension::removeDir(const QString &dirPath) {
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
void Applications::Extension::restorePaths() {
    qDebug() << "[Applications] Restore paths to defaults";

    // Add standard paths
    _rootDirs.clear();

    //  Add standard paths
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        addDir(path);
}



/** ***************************************************************************/
void Applications::Extension::updateIndex() {
    qDebug() << "[Applications] Index update triggered";

    // If thread is running, stop it and start this functoin after termination
    if (!_indexer.isNull()) {
        _indexer->abort();
        _widget->ui.label_info->setText("Waiting for indexer to shut down ...");
        connect(_indexer, &Indexer::destroyed, this, &Extension::updateIndex);
    } else {
        // Create a new scanning runnable for the threadpool
        _indexer = new Indexer(this);

        //  Run it
        QThreadPool::globalInstance()->start(_indexer);

        // If widget is visible show the information in the status bat
        if (!_widget.isNull())
            connect(_indexer, &Indexer::statusInfo, _widget->ui.label_info, &QLabel::setText);
    }
}



/** ***************************************************************************/
bool Applications::Extension::fuzzy() {
    return _searchIndex.fuzzy();
}



/** ***************************************************************************/
void Applications::Extension::setFuzzy(bool b) {
    _searchIndex.setFuzzy(b);
}


