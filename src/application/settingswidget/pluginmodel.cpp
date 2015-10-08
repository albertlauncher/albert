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

#include <QIcon>
#include "pluginmodel.h"
#include "pluginmanager.h"
#include "pluginloader.h"


/** ***************************************************************************/
PluginModel::PluginModel(PluginManager* pm, QObject *parent)
    : QAbstractListModel(parent), pluginManager_(pm){
}



/** ***************************************************************************/
int PluginModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(pluginManager_->plugins().size());
}



/** ***************************************************************************/
QVariant PluginModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return QVariant();

    const unique_ptr<PluginSpec> &plugin = pluginManager_->plugins()[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return plugin->name();
    case Qt::ToolTipRole:
        return QString(
                    "ID: %1\n"
                    "Version: %2\n"
                    "Author: %3\n"
                    "Platform: %4\n"
                    "Dependencies: %5"
                    ).arg(
                    plugin->id(),
                    plugin->version(),
                    plugin->author(),
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
        return (pluginManager_->pluginIsEnabled(plugin))?Qt::Checked:Qt::Unchecked;
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool PluginModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        if (value == Qt::Checked)
            pluginManager_->enablePlugin(pluginManager_->plugins()[index.row()]);
        else
            pluginManager_->disablePlugin(pluginManager_->plugins()[index.row()]);
        dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    default:
        return false;
    }
}



/** ***************************************************************************/
Qt::ItemFlags PluginModel::flags(const QModelIndex &) const {
    return Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemNeverHasChildren;
}
