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
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "externalextensionmodel.h"
#include "externalextension.h"

/** ***************************************************************************/
int ExternalExtensions::ExternalExtensionsModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(externalExtensions_.size());
}



/** ***************************************************************************/
int ExternalExtensions::ExternalExtensionsModel::columnCount(const QModelIndex &) const {
    return static_cast<int>(Section::Count);
}



/** ***************************************************************************/
QVariant ExternalExtensions::ExternalExtensionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section<0 || static_cast<int>(Section::Count)<=section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Name:{
            switch (role) {
            case Qt::DisplayRole: return "Name";
            case Qt::ToolTipRole: return "The name of the extension.";
            default: return QVariant();
            }

        }
        case Section::Trigger:{
            switch (role) {
            case Qt::DisplayRole: return "Trigger";
            case Qt::ToolTipRole: return "The term that triggers this extension.";
            default: return QVariant();
            }

        }
        case Section::Path:{
            switch (role) {
            case Qt::DisplayRole: return "Path";
            case Qt::ToolTipRole: return "The path of this extension.";
            default: return QVariant();
            }

        }
        default: return QVariant();
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant ExternalExtensions::ExternalExtensionsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= static_cast<int>(externalExtensions_.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return externalExtensions_[index.row()]->name();
        case Section::Trigger:  return externalExtensions_[index.row()]->triggers()[0] ;
        case Section::Path:  return externalExtensions_[index.row()]->path();
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return externalExtensions_[index.row()]->name();
        case Section::Trigger:  return externalExtensions_[index.row()]->triggers()[0];
        case Section::Path:  return externalExtensions_[index.row()]->path();
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        switch (static_cast<Section>(index.column())) {
        default: return "Double click to edit";
        }
    }
    default:
        return QVariant();
    }
}


/** ***************************************************************************/
Qt::ItemFlags ExternalExtensions::ExternalExtensionsModel::flags(const QModelIndex &index) const {
    if ( !index.isValid() )
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtensionsModel::onActivated(const QModelIndex &index) {
    QDesktopServices::openUrl(QUrl(externalExtensions_[index.row()]->path()));
}
