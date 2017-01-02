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

#define EXTERNAL_EXTENSION_IID "org.albert.extension.external/v1.0"

/** ***************************************************************************/
ExternalExtensions::Extension::Extension()
    : Core::Extension("org.albert.extension.externalextensions") {

    pluginDirs_ = QStandardPaths::locateAll(
                QStandardPaths::DataLocation, "external",
                QStandardPaths::LocateDirectory);

    fileSystemWatcher_.addPaths(pluginDirs_);

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

    // Iterate over all files in the plugindirs
    for (const QString &pluginDir : pluginDirs_) {
        QDirIterator dirIterator(pluginDir, QDir::Files|QDir::Executable, QDirIterator::NoIteratorFlags);
        while (dirIterator.hasNext()) {

            QString path = dirIterator.next();

            // Get Metadata
            QProcess extProc;
            extProc.start(path, {"METADATA"});
            extProc.waitForFinished(-1);

            // Read JSON data
            QJsonDocument doc = QJsonDocument::fromJson(extProc.readAllStandardOutput());
            if (doc.isNull() || !doc.isObject()) {
                qWarning() << QString("Reply to 'METADATA' is not a valid JSON object.").arg(path);
                continue;
            }

            QJsonObject metadata = doc.object();

            // Check for a sane interface ID (IID)
            if (metadata["iid"].isUndefined()) {
                qWarning() << QString("%1: Metadata does not contain an interface id.").arg(path);
                continue;
            }

            QString iid = metadata["iid"].toString();
            if (iid != EXTERNAL_EXTENSION_IID) {
                qWarning() << QString("%1: Interface id '%2' does not match '%3'.").arg(path, EXTERNAL_EXTENSION_IID);
                continue;
            }

            // Check for mandatory id
            if (metadata["id"].isUndefined()){
                qWarning() << QString("%1: Metadata does not contain an extension id.").arg(path);
                continue;
            }

            QString id = metadata["id"].toString();
            if (id.isEmpty()){
                qWarning() << QString("%1: Extension id is empty.").arg(path);
                continue;
            }

            // Get opional data
            QJsonValue val;

            val = metadata["trigger"];
            QString trigger = val.isString() ? val.toString() : QString();

            val = metadata["name"];
            QString name = val.isString() ? val.toString() : id;

            val = metadata["version"];
            QString version = val.isString() ? val.toString() : "N/A";

            val = metadata["author"];
            QString author = val.isString() ? val.toString() : "N/A";

            QStringList dependencies;
            for (const QJsonValue & value : metadata["dependencies"].toArray())
                 dependencies.append(value.toString());

            try {
                externalExtensions_.emplace_back(new ExternalExtension(path,
                                                                       id,
                                                                       name,
                                                                       author,
                                                                       version,
                                                                       trigger,
                                                                       dependencies));
            } catch ( QString s ) {
                qDebug() << path << "Failed to initialize external extension: " << s;
            }
        }
    }

    for ( std::unique_ptr<ExternalExtension> &obj : externalExtensions_ )
        Core::ExtensionManager::instance->registerObject(obj.get());
}
