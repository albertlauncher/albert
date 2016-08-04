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
#include "abstractextensionloader.h"


/** ***************************************************************************/
LoaderModel::LoaderModel(ExtensionManager* pm, QObject *parent)
    : QAbstractListModel(parent), extensionManager_(pm){
}



/** ***************************************************************************/
int LoaderModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(extensionManager_->extensionLoaders().size());
}



/** ***************************************************************************/
QVariant LoaderModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return QVariant();

    const unique_ptr<AbstractExtensionLoader> &loader = extensionManager_->extensionLoaders()[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return loader->name();
    case Qt::ToolTipRole:
        return QString(
                    "ID: %1\n"
                    "Version: %2\n"
                    "Author: %3\n"
                    "Path: %4\n"
                    "Dependencies: %5"
                    ).arg(
                    loader->id(),
                    loader->version(),
                    loader->author(),
                    loader->path(),
                    loader->dependencies().join(", ")
                    );
    case Qt::DecorationRole:
        switch (loader->state()) {
        case AbstractExtensionLoader::State::Loaded:
            return QIcon(":plugin_loaded");
        case AbstractExtensionLoader::State::NotLoaded:
            return QIcon(":plugin_notloaded");
        case AbstractExtensionLoader::State::Error:
            return QIcon(":plugin_error");
        }
    case Qt::CheckStateRole:
        return (extensionManager_->extensionIsEnabled(loader))?Qt::Checked:Qt::Unchecked;
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
            extensionManager_->enableExtension(extensionManager_->extensionLoaders()[index.row()]);
        else
            extensionManager_->disableExtension(extensionManager_->extensionLoaders()[index.row()]);
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
