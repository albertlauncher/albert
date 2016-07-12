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

#include <QDesktopServices>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QDir>
#include "extension.h"
#include "configwidget.h"
#include "searchengine.h"
#include "query.h"

///**************************************************************************/
//void Websearch::WebSearch::queryFallback(const QString &req, QVector<Service::Item *> *res) const
//{
//	for (Item *w : searchEngines_){
//		w->_searchTerm = req;
//		res->push_back(w);
//	}
//}



/** ***************************************************************************/
Websearch::Extension::Extension() : IExtension("org.albert.extension.websearch")  {

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.exists()) {
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug("[%s] Deserializing from %s", id, dataFile.fileName().toLocal8Bit().data());
            QDataStream in(&dataFile);
            quint64 size;
            in >> size;
            for (quint64 i = 0; i < size; ++i) {
                shared_ptr<SearchEngine> se = std::make_shared<SearchEngine>();
                se->deserialize(in);
                index_.push_back(se);
            }
            dataFile.close();
        } else {
            qWarning() << "Could not open file: " << dataFile.fileName();
            restoreDefaults();
        }
    } else restoreDefaults(); // Without warning
}



/** ***************************************************************************/
Websearch::Extension::~Extension() {
    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug("[%s] Serializing to %s", id, dataFile.fileName().toLocal8Bit().data());
        QDataStream out( &dataFile );
        out << static_cast<quint64>(index_.size());
        for (const shared_ptr<SearchEngine> &se : index_)
            se->serialize(out);
        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();
}



/** ***************************************************************************/
QWidget *Websearch::Extension:: widget(QWidget *parent) {
    if (widget_.isNull()){
        widget_ = new ConfigWidget(parent);
        widget_->ui.tableView_searches->setModel(this);
        // TODO Fix all *.data if least supported Qt supports its omittance
        QObject::connect(widget_.data(), &ConfigWidget::restoreDefaults,
                this, &Extension::restoreDefaults);
    }
    return widget_;
}



/** ***************************************************************************/
void Websearch::Extension::handleQuery(shared_ptr<Query> query) {
    for (const shared_ptr<SearchEngine> &se : index_)
        if (query->searchTerm().toLower().section(' ',0,0) == se->trigger()) {
            se->setQuery(query->searchTerm().section(' ', 1, -1, QString::SectionSkipEmpty));
            query->addMatch(se);
        }
}



/** ***************************************************************************/
void Websearch::Extension::handleFallbackQuery(shared_ptr<Query> query) {
    for (const shared_ptr<SearchEngine> &se : index_) {
        if (se->enabled()) {
            se->setQuery(query->searchTerm());
            query->addMatch(se,3);
        }
    }
}



/** ***************************************************************************/
QStringList Websearch::Extension::triggers() const {
    QStringList triggers;
    for (const auto &i : index_)
        triggers.push_back(i->trigger());
    return triggers;
}



/** ***************************************************************************/
void Websearch::Extension::restoreDefaults() {
    /* Init std searches */
    index_.clear();

    beginResetModel();
    index_.push_back(std::make_shared<SearchEngine>("Google", "https://www.google.com/#q=%s", "gg", ":google"));
    index_.push_back(std::make_shared<SearchEngine>("Youtube", "https://www.youtube.com/results?search_query=%s", "yt", ":youtube"));
    index_.push_back(std::make_shared<SearchEngine>("Amazon", "http://www.amazon.com/s/?field-keywords=%s", "ama", ":amazon"));
    index_.push_back(std::make_shared<SearchEngine>("Ebay", "http://www.ebay.com/sch/i.html?_nkw=%s", "eb", ":ebay"));
    index_.push_back(std::make_shared<SearchEngine>("GitHub", "https://github.com/search?utf8=✓&q=%s", "gh", ":github"));
    index_.push_back(std::make_shared<SearchEngine>("Wolfram Alpha", "https://www.wolframalpha.com/input/?i=%s", "=", ":wolfram"));
    endResetModel();
}



/** ***************************************************************************/
int Websearch::Extension::rowCount(const QModelIndex &) const {
    return static_cast<int>(index_.size());
}



/** ***************************************************************************/
int Websearch::Extension::columnCount(const QModelIndex &) const {
    return static_cast<int>(Section::Count);
}



/** ***************************************************************************/
QVariant Websearch::Extension::headerData(int section, Qt::Orientation orientation, int role) const {
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
        default: return QVariant();
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant Websearch::Extension::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= static_cast<int>(index_.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return index_[index.row()]->name();
        case Section::Trigger:  return index_[index.row()]->trigger();
        case Section::URL:  return index_[index.row()]->url();
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return index_[index.row()]->name();
        case Section::Trigger:  return index_[index.row()]->trigger();
        case Section::URL:  return index_[index.row()]->url();
        default: return QVariant();
        }
    }
    case Qt::DecorationRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return QIcon(index_[index.row()]->iconPath());
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
        case Section::Enabled:  return (index_[index.row()]->enabled())?Qt::Checked:Qt::Unchecked;
        default: return QVariant();
        }
    }
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool Websearch::Extension::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()
            || index.row() >= static_cast<int>(index_.size())
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
            index_[index.row()]->setName(s);
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::Trigger:
            index_[index.row()]->setTrigger(s);
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::URL:
            index_[index.row()]->setUrl(s);
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        default:
            return false;
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            index_[index.row()]->setEnabled(value.toBool());
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
                            .arg(index_[index.row()]->name())
                            .arg(i++)
                            .arg(fileInfo.suffix()));
        // Copy the file into cache dir
        } while (!QFile::copy(fileInfo.filePath(), newFilePath));
        // Set the copied file as icon
        index_[index.row()]->setIconPath(newFilePath);
        dataChanged(index, index, QVector<int>({Qt::DecorationRole}));
    }
    default:
        return false;
    }
    return false;
}



/** ***************************************************************************/
Qt::ItemFlags Websearch::Extension::flags(const QModelIndex &index) const {
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
bool Websearch::Extension::insertRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(index_.size())<position)
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = position; row < position + rows; ++row){
        index_.insert(index_.begin() + row,
                      std::make_shared<SearchEngine>(
                          "<name>",
                          "<http://url/containing/the/?query=%s>",
                          "<trigger>",
                          ":default",
                          false));
    }
    endInsertRows();
    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::removeRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(index_.size())<position+rows)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows-1);
    index_.erase(index_.begin()+position,index_.begin()+(position+rows));
    endRemoveRows();
    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::moveRows(const QModelIndex &src, int srcRow, int cnt, const QModelIndex &dst, int dstRow) {
    if (srcRow<0 || cnt<1 || dstRow<0
            || static_cast<int>(index_.size())<srcRow+cnt-1
            || static_cast<int>(index_.size())<dstRow
            || ( srcRow<=dstRow && dstRow<srcRow+cnt) ) // If its inside the source do nothing
        return false;

    std::vector<shared_ptr<SearchEngine>> tmp;
    beginMoveRows(src, srcRow, srcRow+cnt-1, dst, dstRow);
    tmp.insert(tmp.end(), make_move_iterator(index_.begin()+srcRow), make_move_iterator(index_.begin() + srcRow+cnt));
    index_.erase(index_.begin()+srcRow, index_.begin() + srcRow+cnt);
    const size_t finalDst = dstRow > srcRow ? dstRow - cnt : dstRow;
    index_.insert(index_.begin()+finalDst , make_move_iterator(tmp.begin()), make_move_iterator(tmp.end()));
    endMoveRows();
    return true;
}
