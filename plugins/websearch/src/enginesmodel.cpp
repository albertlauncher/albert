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


#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QStandardPaths>
#include <QUuid>
#include "enginesmodel.h"
#include "extension.h"
#include "searchengine.h"

namespace {

enum class Section{ Name, Trigger, URL} ;
const int sectionCount = 3;

std::map<QString,QIcon> iconCache;

}


/** ***************************************************************************/
Websearch::EnginesModel::EnginesModel(Extension *extension, QObject *parent)
    : QAbstractTableModel(parent), extension_(extension) {
}


/** ***************************************************************************/
int Websearch::EnginesModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(extension_->engines().size());
}



/** ***************************************************************************/
int Websearch::EnginesModel::columnCount(const QModelIndex &) const {
    return sectionCount;
}



/** ***************************************************************************/
QVariant Websearch::EnginesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section < 0 || sectionCount <= section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Name:{
            switch (role) {
            case Qt::DisplayRole: return "Name";
            case Qt::ToolTipRole: return "The name of the searchengine.";
            default: return QVariant();
            }

        }
        case Section::Trigger:{
            switch (role) {
            case Qt::DisplayRole: return "Trigger";
            case Qt::ToolTipRole: return "The term that triggers this searchengine.";
            default: return QVariant();
            }

        }
        case Section::URL:{
            switch (role) {
            case Qt::DisplayRole: return "URL";
            case Qt::ToolTipRole: return "The URL of this searchengine. %s will be replaced by your searchterm.";
            default: return QVariant();
            }

        }
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant Websearch::EnginesModel::data(const QModelIndex &index, int role) const {
    if ( !index.isValid() ||
         index.row() >= static_cast<int>(extension_->engines().size()) ||
         index.column() >= sectionCount )
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:
            return extension_->engines()[static_cast<ulong>(index.row())].name;
        case Section::Trigger:
            return extension_->engines()[static_cast<ulong>(index.row())].trigger;
        case Section::URL:
            return extension_->engines()[static_cast<ulong>(index.row())].url;
        }
    }
    case Qt::DecorationRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:{
            // Resizing request thounsands of repaints. Creating an icon for
            // ever paint event is to expensive. Therefor maintain an icon cache
            const QString &iconPath = extension_->engines()[static_cast<ulong>(index.row())].iconPath;
            std::map<QString,QIcon>::iterator it = iconCache.find(iconPath);
            if ( it != iconCache.end() )
                return it->second;
            return iconCache.insert(std::make_pair(iconPath, QIcon(iconPath))).second;
        }
        case Section::URL:
        case Section::Trigger:
            return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        return "Double click to edit";
    }
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool Websearch::EnginesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if ( !index.isValid() ||
         index.row() >= static_cast<int>(extension_->engines().size()) ||
         index.column() >= sectionCount)
        return false;

    switch (role) {
    case Qt::DisplayRole: {
        if ( !value.canConvert(QMetaType::QString) )
            return false;
        QString s = value.toString();
        switch (static_cast<Section>(index.column())) {
        case Section::Name: {
            std::vector<SearchEngine> newEngines = extension_->engines();
            newEngines[static_cast<ulong>(index.row())].name = s;
            extension_->setEngines(newEngines);
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        }
        case Section::Trigger: {
            std::vector<SearchEngine> newEngines = extension_->engines();
            newEngines[static_cast<ulong>(index.row())].trigger = s;
            extension_->setEngines(newEngines);
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        }
        case Section::URL: {
            std::vector<SearchEngine> newEngines = extension_->engines();
            newEngines[static_cast<ulong>(index.row())].url = s;
            extension_->setEngines(newEngines);
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        }
        }
    }
    case Qt::DecorationRole: {
        QFileInfo fileInfo(value.toString());

        if ( !fileInfo.exists() )
            return false;

        // Remove icon from cache
        iconCache.erase(extension_->engines()[static_cast<ulong>(index.row())].iconPath);

        // Create extension dir if necessary
        QDir dataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        if ( !dataDir.exists(extension_->Core::Extension::id) ) {
            if ( !dataDir.mkdir(extension_->Core::Extension::id) ) {
                qWarning() << "Could not create extension data dir.";
                return false;
            }
        }

        dataDir.cd(extension_->Core::Extension::id);

        // Build the new random path
        QString newFilePath = dataDir.filePath(QString("%1.%2")
                                               .arg(QUuid::createUuid().toString())
                                               .arg(fileInfo.suffix()));

        // Copy the file into data dir
        if ( !QFile::copy(fileInfo.filePath(), newFilePath) ) {
            qWarning() << "Could not copy icon to cache.";
            return false;
        }

        // Remove old icon and set the copied file as icon
        std::vector<SearchEngine> newEngines = extension_->engines();
        QFile::remove(newEngines[static_cast<ulong>(index.row())].iconPath);
        newEngines[static_cast<ulong>(index.row())].iconPath = newFilePath;
        extension_->setEngines(newEngines);

        // Update the icon in the first section of the row
        QModelIndex firstSectionIndex = index.model()->index(index.row(), 0);
        dataChanged(firstSectionIndex, firstSectionIndex, QVector<int>({Qt::DecorationRole}));

        return true;
    }
    default:
        return false;
    }
}



/** ***************************************************************************/
Qt::ItemFlags Websearch::EnginesModel::flags(const QModelIndex &index) const {
    if (index.isValid())
        return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
    else
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;
}



/** ***************************************************************************/
bool Websearch::EnginesModel::insertRows(int position, int rows, const QModelIndex &) {
    if ( position < 0 || rows < 1 ||
         static_cast<int>(extension_->engines().size()) < position)
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    std::vector<SearchEngine> newEngines = extension_->engines();
    for ( int row = position; row < position + rows; ++row )
        newEngines.insert(newEngines.begin() + row,
                          SearchEngine({"<name>", "<trigger>", ":default",
                                        "<http://url/containing/the/?query=%s>"}));
    extension_->setEngines(newEngines);
    endInsertRows();
    return true;
}



/** ***************************************************************************/
bool Websearch::EnginesModel::removeRows(int position, int rows, const QModelIndex &) {
    if ( position < 0 || rows < 1 ||
         static_cast<int>(extension_->engines().size()) < position + rows)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    std::vector<SearchEngine> newEngines = extension_->engines();
    newEngines.erase(newEngines.begin() + position,
                     newEngines.begin() + position + rows);
    extension_->setEngines(newEngines);
    endRemoveRows();
    return true;
}



/** ***************************************************************************/
bool Websearch::EnginesModel::moveRows(const QModelIndex &srcParent, int srcRow, int cnt,
                                       const QModelIndex &dstParent, int dstRow) {
    if ( srcRow < 0 || cnt < 1 || dstRow < 0 ||
         static_cast<int>(extension_->engines().size()) < srcRow + cnt - 1 ||
         static_cast<int>(extension_->engines().size()) < dstRow ||
         ( srcRow <= dstRow && dstRow < srcRow + cnt) ) // If its inside the source do nothing
        return false;

    std::vector<SearchEngine> newEngines = extension_->engines();
    beginMoveRows(srcParent, srcRow, srcRow + cnt - 1, dstParent, dstRow);
    newEngines.insert(newEngines.begin() + dstRow,
                      extension_->engines().begin() + srcRow,
                      extension_->engines().begin() + srcRow + cnt);
    if ( srcRow < dstRow )
        newEngines.erase(newEngines.begin() + srcRow,
                         newEngines.begin() + srcRow + cnt);
    else
        newEngines.erase(newEngines.begin() + srcRow + cnt,
                         newEngines.begin() + srcRow + cnt * 2);
    extension_->setEngines(newEngines);
    endMoveRows();
    return true;
}



/** ***************************************************************************/
void Websearch::EnginesModel::restoreDefaults() {
    beginResetModel();
    extension_->restoreDefaultEngines();
    endResetModel();
}



/** ***************************************************************************/
Qt::DropActions Websearch::EnginesModel::supportedDropActions() const {
    return Qt::MoveAction;
}



/** ***************************************************************************/
bool Websearch::EnginesModel::dropMimeData(const QMimeData *data,
                                           Qt::DropAction /*action*/,
                                           int dstRow,
                                           int /*column*/,
                                           const QModelIndex &/*parent*/) {
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    int srcRow = 0;
    if (!stream.atEnd())
        stream >> srcRow;
    moveRows(QModelIndex(), srcRow, 1, QModelIndex(), dstRow);
    return false;
}
