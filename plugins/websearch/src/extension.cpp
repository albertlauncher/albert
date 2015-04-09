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

#include "extension.h"
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QDesktopServices>
//#include <QClipboard>
//#include <QGuiApplication>
#include <QFile>

#include "query.h"
#include "configwidget.h"

/** ***************************************************************************/
void Extension::restoreDefaults()
{
    /* Init std searches */
    _index.clear();

    SharedSearchPtr se = std::make_shared<SearchEngine>(this);
    se->_name       = "Google";
    se->_url        = "https://www.google.de/#q=%s";
    se->_trigger    = "gg";
    se->_iconPath   = ":google";
    se->_icon       = QIcon(se->_iconPath);
    _index.push_back(se);

    se = std::make_shared<SearchEngine>(this);
    se->_name       = "Youtube";
    se->_url        = "https://www.youtube.com/results?search_query=%s";
    se->_trigger    = "yt";
    se->_iconPath   = ":youtube";
    se->_icon       = QIcon(se->_iconPath);
    _index.push_back(se);

    se = std::make_shared<SearchEngine>(this);
    se->_name       = "Amazon";
    se->_url        = "http://www.amazon.de/s/?field-keywords=%s";
    se->_trigger    = "ama";
    se->_iconPath   = ":amazon";
    se->_icon       = QIcon(se->_iconPath);
    _index.push_back(se);

    se = std::make_shared<SearchEngine>(this);
    se->_name       = "Ebay";
    se->_url        = "http://www.ebay.de/sch/i.html?_nkw=%s";
    se->_trigger    = "eb";
    se->_iconPath   = ":ebay";
    se->_icon       = QIcon(se->_iconPath);
    _index.push_back(se);

    se = std::make_shared<SearchEngine>(this);
    se->_name       = "Wolfram Alpha";
    se->_url        = "https://www.wolframalpha.com/input/?i=%s";
    se->_trigger    = "=";
    se->_iconPath   = ":wolfram";
    se->_icon       = QIcon(se->_iconPath);
    _index.push_back(se);
}


///**************************************************************************/
//void WebSearch::queryFallback(const QString &req, QVector<Service::Item *> *res) const
//{
//	for (Item *w : _searchEngines){
//		w->_searchTerm = req;
//		res->push_back(w);
//	}
//}






/******************************************************************************/
/*                          INTERFACE IMPLEMENTATION                          */
/******************************************************************************/

/** ***************************************************************************/
QWidget *Extension::widget()
{
    if (_widget.isNull()){
        _widget = new ConfigWidget;
        _widget->ui.tableView_searches->setModel(_adapter);
        _widget->ui.tableView_searches->resizeColumnsToContents();
        _widget->ui.tableView_searches->resizeRowsToContents();
        _widget->ui.tableView_searches->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);
    }
    return _widget;
}

/** ***************************************************************************/
void Extension::initialize()
{
    qDebug() << "Initialize extension 'WebSearch'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    /* Deserialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "Deserializing from" << f.fileName();
        QDataStream in(&f);
        quint64 size;
        in >> size;
        for (quint64 i = 0; i < size; ++i) {
            SharedSearchPtr se(new SearchEngine(this));
            in >> se->_url
               >> se->_name
               >> se->_trigger
               >> se->_iconPath
               >> se->_usage;
            se->_icon = QIcon(se->_iconPath);
            _index.push_back(se);
        }
        f.close();
    } else {
        qWarning() << "Could not open file: " << f.fileName();
        restoreDefaults();
    }
}

/** ***************************************************************************/
void Extension::finalize()
{
    qDebug() << "Finalize extension 'WebSearch'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    /* Serialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "Serializing to " << f.fileName();
        QDataStream out( &f );
        out << static_cast<quint64>(_index.size());
        for (SharedSearchPtr se : _index)
            out << se->_url
                << se->_name
                << se->_trigger
                << se->_iconPath
                << se->_usage;
        f.close();
    } else
        qCritical() << "Could not write to " << f.fileName();
}

/** ***************************************************************************/
void Extension::handleQuery(Query *q)
{
    QString trigger = q->searchTerm().section(' ',0,0);
    for (SharedSearchPtr se : _index) {
        if ( (trigger==se->_trigger) || q->searchTerm().startsWith(se->_name) ) {
            // Make a new instance per query
            SharedSearchPtr newSe = std::make_shared<SearchEngine>(*se.get()); //
            newSe->_searchTerm = q->searchTerm().section(' ', 1, -1, QString::SectionSkipEmpty);
            q->addResult(newSe);
        }
    }
}

/** ***************************************************************************/
void Extension::action(const SearchEngine &se, const Query &, Qt::KeyboardModifiers) const
{
    QDesktopServices::openUrl(QUrl(QString(se._url).replace("%s", se._searchTerm)));
    //    QGuiApplication::clipboard()->setText(QString(_url).replace("%s", _searchTerm));
}

/** ***************************************************************************/
QString Extension::actionText(const SearchEngine &se, const Query &, Qt::KeyboardModifiers) const
{
    return QString("Visit '%1'").arg(QString(se._url).replace("%s", se._searchTerm));
    //    return QString("Copy '%1' to clipboard.").arg(QString(_url).replace("%s", _searchTerm));
}

/** ***************************************************************************/
QString Extension::titleText(const SearchEngine &se, const Query &) const
{
    return QString("Search '%1' in %2")
            .arg(((se._searchTerm.isEmpty()) ? "..." : se._searchTerm), se._name);
}

/** ***************************************************************************/
QString Extension::infoText(const SearchEngine &se, const Query &) const
{
    return QString(se._url).replace("%s", se._searchTerm);
}



/******************************************************************************/
/*               SearchEnginesAdapter IMPLEMENTATION                          */
/******************************************************************************/

/** ***************************************************************************/
QVariant IndexAdapter::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case Qt::DisplayRole:
    {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return _ref->value(index.row())->_name;
        case Section::Trigger:  return _ref->value(index.row())->_trigger;
        case Section::URL:  return _ref->value(index.row())->_url;
        default: return QVariant();
        }
    }
    case Qt::DecorationRole:
    {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return _ref->value(index.row())->_icon;
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole:
    {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return QVariant();
        case Section::Trigger:  return QVariant();
        case Section::URL:  return QVariant();
        default: return QVariant();
        }
    }
    case Qt::CheckStateRole:
    {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return QVariant();
        case Section::Trigger:  return QVariant();
        case Section::URL:  return QVariant();
        default: return QVariant();
        }
    }
    default:
        return QVariant();
    }
}

/** ***************************************************************************/
bool IndexAdapter::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    switch (role)
    {
    case Qt::EditRole:
    {
        if ( !value.canConvert(QMetaType::QString) )
            return false;
        QString s = value.toString();
        switch (static_cast<Section>(index.column())) {
        case Section::Name:
            _ref->value(index.row())->_name = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::Trigger:
            _ref->value(index.row())->_trigger = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        case Section::URL:
            qDebug() << _ref->value(index.row())->_url;
            _ref->value(index.row())->_url = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            qDebug() << _ref->value(index.row())->_url;
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
QVariant IndexAdapter::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal){
        switch (role)
        {
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
Qt::ItemFlags IndexAdapter::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable
            |Qt::ItemIsEnabled
            |Qt::ItemIsEditable
            |Qt::ItemIsDragEnabled;
//            |Qt::ItemIsDropEnabled;
//    Qt::ItemIsUserCheckable	| Qt::ItemIsTristate
}

/** ***************************************************************************/
int IndexAdapter::rowCount(const QModelIndex &) const
{
    return _ref->size();
}

/** ***************************************************************************/
int IndexAdapter::columnCount(const QModelIndex &) const
{
    return 3;
}
