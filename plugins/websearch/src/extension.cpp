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

#include <QDesktopServices>
#include <QStandardPaths>
#include <QSettings>
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
//	for (Item *w : _searchEngines){
//		w->_searchTerm = req;
//		res->push_back(w);
//	}
//}



/** ***************************************************************************/
QWidget *Websearch::Extension:: widget() {
    if (widget_.isNull()){
        widget_ = new ConfigWidget;
        widget_->ui.tableView_searches->setModel(this);

        connect(widget_->ui.pushButton_restoreDefaults, &QPushButton::clicked,
                this, &Extension::restoreDefaults);
    }
    return widget_;
}



/** ***************************************************************************/
void Websearch::Extension::initialize() {
    qDebug() << "Initialize extension 'WebSearch'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "[Websearch] Deserializing from" << dataFile.fileName();
        QDataStream in(&dataFile);
        quint64 size;
        in >> size;
        for (quint64 i = 0; i < size; ++i) {
            shared_ptr<SearchEngine> se = std::make_shared<SearchEngine>();
            in >> se->enabled_
               >> se->url_
               >> se->name_
               >> se->trigger_
               >> se->iconPath_
               >> se->usage_;
            se->icon_ = QIcon(se->iconPath_);
            index_.push_back(se);
        }
        dataFile.close();
    } else {
        qWarning() << "Could not open file: " << dataFile.fileName();
        restoreDefaults();
    }
}



/** ***************************************************************************/
void Websearch::Extension::finalize() {
    qDebug() << "Finalize extension 'WebSearch'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[Websearch] Serializing to" << dataFile.fileName();
        QDataStream out( &dataFile );
        out << static_cast<quint64>(index_.size());
        for (const shared_ptr<SearchEngine> &se : index_)
            out << se->enabled_
                << se->url_
                << se->name_
                << se->trigger_
                << se->iconPath_
                << se->usage_;
        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();
}



/** ***************************************************************************/
void Websearch::Extension::setupSession() {

}



/** ***************************************************************************/
void Websearch::Extension::teardownSession() {

}



/** ***************************************************************************/
void Websearch::Extension::handleQuery(shared_ptr<Query> query) {
    QString trigger = query->searchTerm().section(' ',0,0).toLower();
    for (const shared_ptr<SearchEngine>& se : index_) {
        if (se->enabled_ && ((trigger==se->trigger_) || query->searchTerm().startsWith(se->name_.toLower())) ) {
            // Make a new instance per query
            se->searchTerm_ = query->searchTerm().section(' ', 1, -1, QString::SectionSkipEmpty);
            query->addMatch(se,3);
        }
    }
}


/** ***************************************************************************/
void Websearch::Extension::restoreDefaults() {
    /* Init std searches */
    index_.clear();

    beginResetModel();

    shared_ptr<SearchEngine> se;

    se= std::make_shared<SearchEngine>();
    se->enabled_    = true;
    se->name_       = "Google";
    se->url_        = "https://www.google.de/#q=%s";
    se->trigger_    = "gg";
    se->iconPath_   = ":google";
    se->icon_       = QIcon(se->iconPath_);
    index_.push_back(se);

    se = std::make_shared<SearchEngine>();
    se->enabled_    = true;
    se->name_       = "Youtube";
    se->url_        = "https://www.youtube.com/results?search_query=%s";
    se->trigger_    = "yt";
    se->iconPath_   = ":youtube";
    se->icon_       = QIcon(se->iconPath_);
    index_.push_back(se);

    se = std::make_shared<SearchEngine>();
    se->enabled_    = true;
    se->name_       = "Amazon";
    se->url_        = "http://www.amazon.de/s/?field-keywords=%s";
    se->trigger_    = "ama";
    se->iconPath_   = ":amazon";
    se->icon_       = QIcon(se->iconPath_);
    index_.push_back(se);

    se = std::make_shared<SearchEngine>();
    se->enabled_    = true;
    se->name_       = "Ebay";
    se->url_        = "http://www.ebay.de/sch/i.html?_nkw=%s";
    se->trigger_    = "eb";
    se->iconPath_   = ":ebay";
    se->icon_       = QIcon(se->iconPath_);
    index_.push_back(se);

    se = std::make_shared<SearchEngine>();
    se->enabled_    = true;
    se->name_       = "Wolfram Alpha";
    se->url_        = "https://www.wolframalpha.com/input/?i=%s";
    se->trigger_    = "=";
    se->iconPath_   = ":default";
    se->icon_       = QIcon(se->iconPath_);
    index_.push_back(se);

    endResetModel();
}



/** ***************************************************************************/
int Websearch::Extension::rowCount(const QModelIndex &) const {
    return static_cast<int>(index_.size());
}



/** ***************************************************************************/
int Websearch::Extension::columnCount(const QModelIndex &) const {
    return COL_COUNT;
}



/** ***************************************************************************/
QVariant Websearch::Extension::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section<0 || COL_COUNT<=section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (role) {
        case Qt::DisplayRole:
            switch (static_cast<Section>(section)) {
            case Section::Name:  return "Name";
            case Section::Trigger:  return "Trigger";
            case Section::URL:  return "URL";
            default: return QVariant();
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
            || index.column() >= static_cast<int>(COL_COUNT))
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return index_[index.row()]->name_;
        case Section::Trigger:  return index_[index.row()]->trigger_;
        case Section::URL:  return index_[index.row()]->url_;
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return index_[index.row()]->name_;
        case Section::Trigger:  return index_[index.row()]->trigger_;
        case Section::URL:  return index_[index.row()]->url_;
        default: return QVariant();
        }
    }
    case Qt::DecorationRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return index_[index.row()]->icon_;
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:  return "Check to enable the search engine";
        default: return "Double click to edit";
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:  return (index_[index.row()]->enabled_)?Qt::Checked:Qt::Unchecked;
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
            || index.column() >= static_cast<int>(COL_COUNT))
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
            index_[index.row()]->name_ = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::Trigger:
            index_[index.row()]->trigger_ = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::URL:
            index_[index.row()]->url_ = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        default:
            return false;
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            index_[index.row()]->enabled_ = value.toBool();
            return true;
        default:
            return false;
        }
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
        return Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
    default:
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable;
    }
}

/** ***************************************************************************/
bool Websearch::Extension::insertRows(int position, int rows, const QModelIndex &) {
    if (position > static_cast<int>(index_.size()))
        return false;

    if (position < 0)
        position = static_cast<int>(index_.size());

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = position; row < rows; ++row){
        shared_ptr<SearchEngine> se = std::make_shared<SearchEngine>();
        se->enabled_ = false;
        se->name_ = "Name";
        se->trigger_ = "Trigger";
        se->url_ = "URL ";
        index_.insert(index_.begin() + row, se);
    }
    endInsertRows();
    return true;
}

/** ***************************************************************************/
bool Websearch::Extension::removeRows(int position, int rows, const QModelIndex &) {
    if (position < 0 || static_cast<int>(index_.size()) <= position)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    index_.erase(index_.begin()+position,index_.begin()+(position+rows));
    endRemoveRows();
    return true;
}
