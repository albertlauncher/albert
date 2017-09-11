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
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>
#include <vector>
#include <memory>
#include "configwidget.h"
#include "externalextension.h"
#include "externalextensionmodel.h"
#include "extension.h"



class ExternalExtensions::Private
{
public:
    QStringList pluginDirs;
    std::vector<std::unique_ptr<ExternalExtension>> externalExtensions;
    QFileSystemWatcher fileSystemWatcher;
    QPointer<ConfigWidget> widget;
};




/** ***************************************************************************/
ExternalExtensions::Extension::Extension()
    : Core::Extension("org.albert.extension.externalextensions"),
    d(new Private) {

    d->pluginDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation, "external",
                QStandardPaths::LocateDirectory);

    d->fileSystemWatcher.addPaths(d->pluginDirs);

    connect(&d->fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &Extension::reloadExtensions);

    connect(&d->fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::reloadExtensions);

    reloadExtensions();
}



/** ***************************************************************************/
ExternalExtensions::Extension::~Extension() {

    // Unregister and delete all extensions
    auto it = d->externalExtensions.rbegin();
    while (it != d->externalExtensions.rend()) {
        unregisterQueryHandler(it->get());
        it = std::reverse_iterator<decltype(d->externalExtensions)::iterator>(d->externalExtensions.erase(std::next(it).base()));
    }
}



/** ***************************************************************************/
QWidget *ExternalExtensions::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        ExternalExtensionsModel *model = new ExternalExtensionsModel(d->externalExtensions, d->widget->ui.tableView);
        d->widget->ui.tableView->setModel(model);

        connect(d->widget->ui.tableView, &QTableView::activated,
                model, &ExternalExtensionsModel::onActivated);

        // Reset the widget when
        connect(this, &Extension::extensionsUpdated,
                d->widget->ui.tableView, &QTableView::reset);
    }
    return d->widget;
}



/** ***************************************************************************/
void ExternalExtensions::Extension::reloadExtensions() {

    // Unregister and delete all extensions
    auto it = d->externalExtensions.rbegin();
    while (it != d->externalExtensions.rend()) {
        unregisterQueryHandler(it->get());
        it = std::reverse_iterator<decltype(d->externalExtensions)::iterator>(d->externalExtensions.erase(std::next(it).base()));
    }

    // Remove all watches
    if ( !d->fileSystemWatcher.files().isEmpty() )
        d->fileSystemWatcher.removePaths(d->fileSystemWatcher.files());

    // Iterate over all files in the plugindirs
    for (const QString &pluginDir : d->pluginDirs) {
        QDirIterator dirIterator(pluginDir, QDir::Files|QDir::Executable, QDirIterator::NoIteratorFlags);
        while (dirIterator.hasNext()) {

            QString path = dirIterator.next();
            QString id = dirIterator.fileInfo().fileName();

            // Skip if this id already exists
            if ( std::find_if(d->externalExtensions.begin(),
                              d->externalExtensions.end(),
                              [&id](const std::unique_ptr<ExternalExtension> & rhs){
                              return id == rhs->id(); }) != d->externalExtensions.end())
                continue;

            try {
                d->externalExtensions.emplace_back(new ExternalExtension(path, id));
                d->fileSystemWatcher.addPath(path);
            } catch ( QString s ) {
                qWarning("Failed to initialize external extension: %s", s.toLocal8Bit().data());
            }
        }
    }

    for ( std::unique_ptr<ExternalExtension> &obj : d->externalExtensions )
        unregisterQueryHandler(obj.get());

    emit extensionsUpdated();
}
