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

#include <QIcon>
#include "loadermodel.h"
#include "extensionmanager.h"
#include "pluginspec.h"
using std::unique_ptr;
using namespace Core;



/** ***************************************************************************/
Core::LoaderModel::LoaderModel(ExtensionManager* pm, QObject *parent)
    : QAbstractListModel(parent), extensionManager_(pm){
}



/** ***************************************************************************/
int Core::LoaderModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(extensionManager_->extensionSpecs().size());
}



/** ***************************************************************************/
QVariant Core::LoaderModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return QVariant();

    const unique_ptr<PluginSpec> &loader = extensionManager_->extensionSpecs()[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return loader->name();
    case Qt::ToolTipRole:{
        QString toolTip;
        toolTip = QString("ID: %1\nVersion: %2\nAuthor: %3\n").arg(loader->id(), loader->version(), loader->author());
        if (!loader->lastError().isEmpty())
            toolTip.append(QString("Error: %1\n").arg(loader->lastError()));
        if (!loader->dependencies().empty())
            toolTip.append(QString("Dependencies: %1\n").arg(loader->dependencies().join(", ")));
        toolTip.append(QString("Path: %1").arg(loader->path()));
        return toolTip;
    }
    case Qt::DecorationRole:
        switch (loader->state()) {
        case PluginSpec::State::Loaded:
            return QIcon(":plugin_loaded");
        case PluginSpec::State::NotLoaded:
            return QIcon(":plugin_notloaded");
        case PluginSpec::State::Error:
            return QIcon(":plugin_error");
        }
    case Qt::CheckStateRole:
        return (extensionManager_->extensionIsEnabled(loader))?Qt::Checked:Qt::Unchecked;
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool Core::LoaderModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        if (value == Qt::Checked)
            extensionManager_->enableExtension(extensionManager_->extensionSpecs()[index.row()]);
        else
            extensionManager_->disableExtension(extensionManager_->extensionSpecs()[index.row()]);
        dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    default:
        return false;
    }
}



/** ***************************************************************************/
Qt::ItemFlags Core::LoaderModel::flags(const QModelIndex &) const {
    return Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemNeverHasChildren;
}
