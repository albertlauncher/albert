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
#include "core_globals.h"
#include "pluginloader.h"
#include "pluginspec.h"
using std::unique_ptr;
using std::vector;
using std::chrono::system_clock;



/** ***************************************************************************/
Core::PluginLoader::PluginLoader() {


}

/** ***************************************************************************/
const vector<QString> &Core::PluginLoader::pluginDirs() const {
    return pluginDirs_;
}


/** ***************************************************************************/
void Core::PluginLoader::setPluginDirs(const std::vector<QString>&pluginDirs) {
    pluginDirs_ = pluginDirs;
}


/** ***************************************************************************/
std::vector<std::unique_ptr<Core::PluginSpec>> Core::PluginLoader::pluginSpecsByIID(const QString &iid) {

    std::vector<std::unique_ptr<Core::PluginSpec>> pluginSpecs;

    // Iterate over all files in the plugindirs
    for ( const QString &pluginDir : pluginDirs_ ) {
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

            // Is this a QPlugin
            if ( loader.metaData().empty() )
                continue;

            // Check for a sane interface ID  (IID)
            if ( loader.metaData()["IID"].toString() != iid )
                continue;

            // Check for duplicates
            QString id = loader.metaData()["MetaData"].toObject()["id"].toString();
            if (std::any_of(pluginSpecs.begin(), pluginSpecs.end(),
                            [&id](const unique_ptr<PluginSpec> &spec){ return id == spec->id(); })) {
                qWarning() << qPrintable(QString("Extension IDs already exists. Skipping. (%1)").arg(path));
                continue;
            }

            // Put it to the results
            pluginSpecs.emplace_back(new PluginSpec(path));
        }
    }
    return pluginSpecs;
}
