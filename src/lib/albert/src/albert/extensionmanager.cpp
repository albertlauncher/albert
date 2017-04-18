// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QLibrary>
#include <QPluginLoader>
#include <QSettings>
#include <QStandardPaths>
#include <chrono>
#include <memory>
#include "extensionmanager.h"
#include "extensionspec.h"
using std::set;
using std::unique_ptr;
using std::vector;
using std::chrono::system_clock;

Core::ExtensionManager *Core::ExtensionManager::instance = nullptr;

/** ***************************************************************************/
class Core::ExtensionManagerPrivate {
public:
    vector<unique_ptr<ExtensionSpec>> extensionSpecs_; // TASK: Rename _
    set<QObject*> extensions_;
    QStringList pluginDirs;
};


/** ***************************************************************************/
Core::ExtensionManager::ExtensionManager() : d(new ExtensionManagerPrivate) {
    // DO NOT LOAD EXTENSIONS HERE!

    // Get plugindirs
#if defined __linux__

    QStringList dirs = {
        "/usr/lib/", "/usr/local/lib/", "/usr/lib64/", "/usr/local/lib64/",
        QDir::home().filePath(".local/lib/"),
        QDir::home().filePath(".local/lib64/")
    };

    for ( const QString& dir : dirs ) {
        QFileInfo fileInfo = QFileInfo(QDir(dir).filePath("albert/plugins"));
        if ( fileInfo.isDir() )
            d->pluginDirs << fileInfo.absoluteFilePath();
    }

#elif defined __APPLE__
    throw "Not implemented";
#elif defined _WIN32
    throw "Not implemented";
#endif

}


/** ***************************************************************************/
Core::ExtensionManager::~ExtensionManager() {
    for (unique_ptr<ExtensionSpec> & extensionSpec : d->extensionSpecs_)
        unloadExtension(extensionSpec);
}


/** ***************************************************************************/
void Core::ExtensionManager::setPluginDirs(const QStringList &dirs) {
    d->pluginDirs = dirs;
}


/** ***************************************************************************/
void Core::ExtensionManager::reloadExtensions() {

    // Unload everything
    for (unique_ptr<ExtensionSpec> & extensionSpec : d->extensionSpecs_)
        unloadExtension(extensionSpec);

    d->extensionSpecs_.clear();

    // Iterate over all files in the plugindirs
    for (const QString &pluginDir : d->pluginDirs) {
        QDirIterator dirIterator(pluginDir, QDir::Files);
        while (dirIterator.hasNext()) {
            dirIterator.next();

            QString path = dirIterator.fileInfo().canonicalFilePath();

            // Check if this path is a lib
            if ( !QLibrary::isLibrary(path) ) {
                qWarning() << "File is not a library:" << path;
                continue;
            }

            QPluginLoader loader(path);

            if ( loader.metaData().empty() ) {
                qWarning() << qPrintable(QString("Metadata empty. Is this a QPlugin? (%1)").arg(path));
                continue;
            }

            // Check for a sane interface ID  (IID)
            QString iid = loader.metaData()["IID"].toString();
            if (iid != ALBERT_EXTENSION_IID) {
                qWarning() << qPrintable(QString("Extension IDs do not match. App:'%1'. Ext: '%2' (%3)")
                              .arg(ALBERT_EXTENSION_IID, iid, path));
                continue;
            }

            // Check for duplicates
            QString id = loader.metaData()["MetaData"].toObject()["id"].toString();
            if (std::any_of(d->extensionSpecs_.begin(), d->extensionSpecs_.end(),
                            [&id](const unique_ptr<ExtensionSpec> & spec){ return id == spec->id(); })) {
                qWarning() << qPrintable(QString("Extension IDs already exists. Skipping. (%1)").arg(path));
                continue;
            }

            // Put it to the results
            d->extensionSpecs_.emplace_back(new ExtensionSpec(path));
        }
    }

    // Sort alphabetically
    std::sort(d->extensionSpecs_.begin(),
              d->extensionSpecs_.end(),
              [](const unique_ptr<ExtensionSpec>& lhs, const unique_ptr<ExtensionSpec>& rhs){ return lhs->name() < rhs->name(); });

    // Load if enabled
    QSettings settings(qApp->applicationName());
    for (unique_ptr<ExtensionSpec> & extensionSpec : d->extensionSpecs_){
        QString configName = QString("%1/enabled").arg(extensionSpec->id());
        if ( (settings.contains(configName) && settings.value(configName).toBool())
             || (!settings.contains(configName) && extensionSpec->enabledByDefault()) )
            loadExtension(extensionSpec);
    }
}


/** ***************************************************************************/
const vector<unique_ptr<Core::ExtensionSpec>>& Core::ExtensionManager::extensionSpecs() const {
    return d->extensionSpecs_;
}


/** ***************************************************************************/
const set<QObject*> Core::ExtensionManager::objects() const {
    return d->extensions_;
}


/** ***************************************************************************/
void Core::ExtensionManager::loadExtension(const unique_ptr<ExtensionSpec> &spec) {
    if (spec->state() != ExtensionSpec::State::Loaded){
//        system_clock::time_point start = system_clock::now();
        if ( spec->load() ) {
//            TODO wrtie to database
//            auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now()-start);
//            qDebug() << QString("Loading %1 done in %2 milliseconds").arg(spec->id()).arg(msecs.count()).toLocal8Bit().data();
            d->extensions_.insert(spec->instance());
        } else
            qDebug() << QString("Loading %1 failed. (%2)").arg(spec->id(), spec->lastError()).toLocal8Bit().data();
    }
}


/** ***************************************************************************/
void Core::ExtensionManager::unloadExtension(const unique_ptr<ExtensionSpec> &spec) {
    if (spec->state() != ExtensionSpec::State::NotLoaded) {
        d->extensions_.erase(spec->instance());
        spec->unload();
    }
}


/** ***************************************************************************/
void Core::ExtensionManager::enableExtension(const unique_ptr<ExtensionSpec> &extensionSpec) {
    QSettings(qApp->applicationName()).setValue(QString("%1/enabled").arg(extensionSpec->id()), true);
    loadExtension(extensionSpec);
}


/** ***************************************************************************/
void Core::ExtensionManager::disableExtension(const unique_ptr<ExtensionSpec> &extensionSpec) {
    QSettings(qApp->applicationName()).setValue(QString("%1/enabled").arg(extensionSpec->id()), false);
    unloadExtension(extensionSpec);
}


/** ***************************************************************************/
bool Core::ExtensionManager::extensionIsEnabled(const unique_ptr<ExtensionSpec> &extensionSpec) {
    QSettings settings(qApp->applicationName());
    QString configName = QString("%1/enabled").arg(extensionSpec->id());
    return ( (settings.contains(configName) && settings.value(configName).toBool())
             || (!settings.contains(configName) && extensionSpec->enabledByDefault()) );
}


/** ***************************************************************************/
void Core::ExtensionManager::registerObject(QObject *object) {
    d->extensions_.insert(object);
}


/** ***************************************************************************/
void Core::ExtensionManager::unregisterObject(QObject *object) {
    d->extensions_.erase(object);
}
