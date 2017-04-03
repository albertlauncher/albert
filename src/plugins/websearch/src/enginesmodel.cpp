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


#include "enginesmodel.h"
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QStandardPaths>

namespace {

enum class Section{ Enabled, Name, Trigger, URL, Count };

std::map<QString,QIcon> iconCache;

}


/** ***************************************************************************/
Websearch::EnginesModel::EnginesModel(std::vector<SearchEngine>& se, QObject *parent)
    : QAbstractTableModel(parent), searchEngines_(se) {
}


/** ***************************************************************************/
int Websearch::EnginesModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(searchEngines_.size());
}



/** ***************************************************************************/
int Websearch::EnginesModel::columnCount(const QModelIndex &) const {
    return static_cast<int>(Section::Count);
}



/** ***************************************************************************/
QVariant Websearch::EnginesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section<0 || static_cast<int>(Section::Count)<=section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Enabled:{
            switch (role) {
            case Qt::ToolTipRole: return "Enables the searchengine as fallback.";
            default: return QVariant();
            }
        }
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
        case Section::Count:
            return QVariant();
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant Websearch::EnginesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= static_cast<int>(searchEngines_.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return searchEngines_[index.row()].name;
        case Section::Trigger:  return searchEngines_[index.row()].trigger;
        case Section::URL:  return searchEngines_[index.row()].url;
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return searchEngines_[index.row()].name;
        case Section::Trigger:  return searchEngines_[index.row()].trigger;
        case Section::URL:  return searchEngines_[index.row()].url;
        default: return QVariant();
        }
    }
    case Qt::DecorationRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:{
            // Resizing request thounsands of repaints. Creating an icon for
            // ever paint event is to expensive. Therefor maintain an icon cache
            QString &iconPath = searchEngines_[index.row()].iconPath;
            decltype(iconCache)::iterator it = iconCache.find(iconPath);
            if ( it != iconCache.end() )
                return it->second;
            return iconCache.emplace(iconPath, QIcon(iconPath)).second;
        }
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:  return "Check to use as fallback";
        default: return "Double click to edit";
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:  return (searchEngines_[index.row()].enabled)?Qt::Checked:Qt::Unchecked;
        default: return QVariant();
        }
    }
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool Websearch::EnginesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()
            || index.row() >= static_cast<int>(searchEngines_.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return false;

    switch (role) {
    case Qt::EditRole: {
        if ( !value.canConvert(QMetaType::QString) )
            return false;
        QString s = value.toString();
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            return false;
        case Section::Name:
            searchEngines_[index.row()].name = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::Trigger:
            searchEngines_[index.row()].trigger = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::URL:
            searchEngines_[index.row()].url = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        default:
            return false;
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            searchEngines_[index.row()].enabled = value.toBool();
            dataChanged(index, index, QVector<int>({Qt::CheckStateRole}));
            return true;
        default:
            return false;
        }
    }
    case Qt::DecorationRole: {
        QFileInfo fileInfo(value.toString());
        QString newFilePath;
        uint i = 0;
        do {
        // Build the new path in cache dir
        newFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
                .filePath(QString("%1-%2.%3")
                            .arg(searchEngines_[index.row()].name)
                            .arg(i++)
                            .arg(fileInfo.suffix()));
        // Copy the file into cache dir
        } while (!QFile::copy(fileInfo.filePath(), newFilePath));
        // Set the copied file as icon
        searchEngines_[index.row()].iconPath = newFilePath;
        dataChanged(index, index, QVector<int>({Qt::DecorationRole}));
        iconCache.clear();
        return true;
    }
    default:
        return false;
    }
    return false;
}



/** ***************************************************************************/
Qt::ItemFlags Websearch::EnginesModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    switch (static_cast<Section>(index.column())) {
    case Section::Enabled:
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
    default:
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable;
    }
}



/** ***************************************************************************/
bool Websearch::EnginesModel::insertRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(searchEngines_.size())<position)
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = position; row < position + rows; ++row){
        searchEngines_.insert(searchEngines_.begin() + row,
                              SearchEngine({false,
                                            "<name>",
                                            "<trigger>",
                                            ":default",
                                            "<http://url/containing/the/?query=%s>"}));
    }
    endInsertRows();
    return true;
}



/** ***************************************************************************/
bool Websearch::EnginesModel::removeRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(searchEngines_.size())<position+rows)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows-1);
    searchEngines_.erase(searchEngines_.begin()+position,searchEngines_.begin()+(position+rows));
    endRemoveRows();
    return true;
}



/** ***************************************************************************/
bool Websearch::EnginesModel::moveRows(const QModelIndex &src, int srcRow, int cnt, const QModelIndex &dst, int dstRow) {
    if (srcRow<0 || cnt<1 || dstRow<0
            || static_cast<int>(searchEngines_.size())<srcRow+cnt-1
            || static_cast<int>(searchEngines_.size())<dstRow
            || ( srcRow<=dstRow && dstRow<srcRow+cnt) ) // If its inside the source do nothing
        return false;

    std::vector<SearchEngine> tmp;
    beginMoveRows(src, srcRow, srcRow+cnt-1, dst, dstRow);
    tmp.insert(tmp.end(), make_move_iterator(searchEngines_.begin()+srcRow), make_move_iterator(searchEngines_.begin() + srcRow+cnt));
    searchEngines_.erase(searchEngines_.begin()+srcRow, searchEngines_.begin() + srcRow+cnt);
    const size_t finalDst = dstRow > srcRow ? dstRow - cnt : dstRow;
    searchEngines_.insert(searchEngines_.begin()+finalDst , make_move_iterator(tmp.begin()), make_move_iterator(tmp.end()));
    endMoveRows();
    return true;
}

