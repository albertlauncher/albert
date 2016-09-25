// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include <QIcon>
#include "loadermodel.h"
#include "extensionmanager.h"
#include "pluginloader.h"


/** ***************************************************************************/
LoaderModel::LoaderModel(ExtensionManager* pm, QObject *parent)
    : QAbstractListModel(parent), extensionManager_(pm){
}



/** ***************************************************************************/
int LoaderModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(extensionManager_->plugins().size());
}



/** ***************************************************************************/
QVariant LoaderModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return QVariant();

    const unique_ptr<PluginSpec> &plugin = extensionManager_->plugins()[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return plugin->name();
    case Qt::ToolTipRole:
        return QString(
                    "ID: %1\n"
                    "Version: %2\n"
                    "Author: %3\n"
                    "Path: %4\n"
                    "Platform: %5\n"
                    "Dependencies: %6"
                    ).arg(
                    plugin->id(),
                    plugin->version(),
                    plugin->author(),
                    plugin->fileName(),
                    plugin->platform(),
                    plugin->dependencies()
                    );
    case Qt::DecorationRole:
        switch (plugin->status()) {
        case PluginSpec::Status::Loaded:
            return QIcon(":plugin_loaded");
        case PluginSpec::Status::NotLoaded:
            return QIcon(":plugin_notloaded");
        case PluginSpec::Status::Error:
            return QIcon(":plugin_error");
        }
    case Qt::CheckStateRole:
        return (extensionManager_->pluginIsEnabled(plugin))?Qt::Checked:Qt::Unchecked;
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool LoaderModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        if (value == Qt::Checked)
            extensionManager_->enablePlugin(extensionManager_->plugins()[index.row()]);
        else
            extensionManager_->disablePlugin(extensionManager_->plugins()[index.row()]);
        dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    default:
        return false;
    }
}



/** ***************************************************************************/
Qt::ItemFlags LoaderModel::flags(const QModelIndex &) const {
    return Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemNeverHasChildren;
}
