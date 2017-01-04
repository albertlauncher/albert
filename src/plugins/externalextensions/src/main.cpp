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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include "main.h"
#include "configwidget.h"
#include "externalextension.h"
#include "externalextensionmodel.h"
#include "extensionmanager.h"

/** ***************************************************************************/
ExternalExtensions::Extension::Extension()
    : Core::Extension("org.albert.extension.externalextensions") {

    pluginDirs_ = QStandardPaths::locateAll(
                QStandardPaths::DataLocation, "external",
                QStandardPaths::LocateDirectory);

    fileSystemWatcher_.addPaths(pluginDirs_);

    connect(&fileSystemWatcher_, &QFileSystemWatcher::fileChanged,
            this, &Extension::reloadExtensions);

    connect(&fileSystemWatcher_, &QFileSystemWatcher::directoryChanged,
            this, &Extension::reloadExtensions);

    reloadExtensions();
}



/** ***************************************************************************/
ExternalExtensions::Extension::~Extension() {

    // Unregister and delete all extensions
    auto it = externalExtensions_.rbegin();
    while (it != externalExtensions_.rend()) {
        Core::ExtensionManager::instance->unregisterObject(it->get());
        it = std::reverse_iterator<decltype(externalExtensions_)::iterator>(externalExtensions_.erase(std::next(it).base()));
    }
}



/** ***************************************************************************/
QWidget *ExternalExtensions::Extension::widget(QWidget *parent) {
    if (widget_.isNull()){
        widget_ = new ConfigWidget(parent);

        ExternalExtensionsModel *model = new ExternalExtensionsModel(externalExtensions_, widget_->ui.tableView);
        widget_->ui.tableView->setModel(model);

        connect(widget_->ui.tableView, &QTableView::activated,
                model, &ExternalExtensionsModel::onActivated);

        // Reset the widget when
        connect(this, &Extension::extensionsUpdated,
                widget_->ui.tableView, &QTableView::reset);
    }
    return widget_;
}



/** ***************************************************************************/
void ExternalExtensions::Extension::reloadExtensions() {

    // Unregister and delete all extensions
    auto it = externalExtensions_.rbegin();
    while (it != externalExtensions_.rend()) {
        Core::ExtensionManager::instance->unregisterObject(it->get());
        it = std::reverse_iterator<decltype(externalExtensions_)::iterator>(externalExtensions_.erase(std::next(it).base()));
    }

    // Remove all watches
    if ( !fileSystemWatcher_.files().isEmpty() )
        fileSystemWatcher_.removePaths(fileSystemWatcher_.files());

    // Iterate over all files in the plugindirs
    for (const QString &pluginDir : pluginDirs_) {
        QDirIterator dirIterator(pluginDir, QDir::Files|QDir::Executable, QDirIterator::NoIteratorFlags);
        while (dirIterator.hasNext()) {

            QString path = dirIterator.next();
            QString id = dirIterator.fileInfo().fileName();

            // Skip if this id already exists
            if ( std::find_if(externalExtensions_.begin(),
                              externalExtensions_.end(),
                              [&id](const std::unique_ptr<ExternalExtension> & rhs){
                              return id == rhs->id(); }) != externalExtensions_.end())
                continue;

            try {
                externalExtensions_.emplace_back(new ExternalExtension(path, id));
                fileSystemWatcher_.addPath(path);
            } catch ( QString s ) {
                qCritical("Failed to initialize external extension: %s", s.toLocal8Bit().data());
            }
        }
    }

    for ( std::unique_ptr<ExternalExtension> &obj : externalExtensions_ )
        Core::ExtensionManager::instance->registerObject(obj.get());

    emit extensionsUpdated();
}
