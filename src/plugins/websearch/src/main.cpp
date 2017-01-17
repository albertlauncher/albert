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

#include <QByteArray>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QStandardPaths>
#include <QUrl>
#include <map>
#include <vector>
#include "main.h"
#include "configwidget.h"
#include "item.h"
#include "standarditem.h"
#include "standardaction.h"
#include "query.h"
using std::shared_ptr;
using std::vector;
using namespace Core;


namespace {

enum class Section{ Enabled, Name, Trigger, URL, Count };

std::map<QString,QIcon> iconCache;

struct SearchEngine {
    bool    enabled;
    QString name;
    QString url;
    QString trigger;
    QString iconPath;
};

shared_ptr<Core::Item> buildWebsearchItem(const SearchEngine &se, const QString &searchterm) {

    QString urlString = QString(se.url).replace("%s", QUrl::toPercentEncoding(searchterm));
    QUrl url = QUrl(urlString);
    QString desc = QString("Start %1 search in default browser").arg(se.name);

    std::shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(desc);
    action->setAction([=](){ QDesktopServices::openUrl(url); });

    std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(se.name);
    item->setText(se.name);
    item->setSubtext(desc);
    item->setIconPath(se.iconPath);

    item->setActions({action});

    return item;
}
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Websearch::WebsearchPrivate
{
public:
    WebsearchPrivate(Extension *q) : q(q) {}

    bool deserialize();
    bool serialize();
    void restoreDefaults();

    QPointer<ConfigWidget> widget;
    std::vector<SearchEngine> searchEngines;

    Extension *q;
};



/** ***************************************************************************/
bool Websearch::WebsearchPrivate::deserialize() {

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
               .filePath(QString("%1.json").arg(q->Core::Extension::id)));

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << QString("[%1] Could not open file: %2").arg(q->Core::Extension::id,file.fileName()).toLocal8Bit().data();
        return false;
    }

    QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();

    for ( const QJsonValue& value : array) {
        QJsonObject object = value.toObject();
        searchEngines.emplace_back(SearchEngine{object["enabled"].toBool(),
                                                object["name"].toString(),
                                                object["url"].toString(),
                                                object["trigger"].toString(),
                                                object["iconPath"].toString()});
    }

    return true;
}



/** ***************************************************************************/
bool Websearch::WebsearchPrivate::serialize() {

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
               .filePath(QString("%1.json").arg(q->Core::Extension::id)));

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << QString("[%1] Could not open file: %2").arg(q->Core::Extension::id,file.fileName()).toLocal8Bit().data();
        return false;
    }

    QJsonArray array;

    for ( const SearchEngine& searchEngine : searchEngines ) {
        QJsonObject object;
        object["name"]     = searchEngine.name;
        object["url"]      = searchEngine.url;
        object["trigger"]  = searchEngine.trigger;
        object["iconPath"] = searchEngine.iconPath;
        object["enabled"]  = searchEngine.enabled;
        array.append(object);
    }

    file.write(QJsonDocument(array).toJson());

    return true;
}



/** ***************************************************************************/
void Websearch::WebsearchPrivate::restoreDefaults() {
    /* Init std searches */
    searchEngines.clear();

    searchEngines.emplace_back(SearchEngine{true, "Google", "https://www.google.com/search?q=%s", "gg ", ":google"});
    searchEngines.emplace_back(SearchEngine{true, "Youtube", "https://www.youtube.com/results?search_query=%s", "yt ", ":youtube"});
    searchEngines.emplace_back(SearchEngine{true, "Amazon", "http://www.amazon.com/s/?field-keywords=%s", "ama ", ":amazon"});
    searchEngines.emplace_back(SearchEngine{true, "Ebay", "http://www.ebay.com/sch/i.html?_nkw=%s", "eb ", ":ebay"});
    searchEngines.emplace_back(SearchEngine{true, "GitHub", "https://github.com/search?utf8=✓&q=%s", "gh ", ":github"});
    searchEngines.emplace_back(SearchEngine{true, "Wolfram Alpha", "https://www.wolframalpha.com/input/?i=%s", "=", ":wolfram"});

    serialize();

    if (!widget.isNull())
        widget->ui.tableView_searches->reset();
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Websearch::Extension::Extension()
    : Core::Extension("org.albert.extension.websearch"),
      Core::QueryHandler(Core::Extension::id),
      d(new WebsearchPrivate(this)) {

    QString writableLocation =  QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFile dataFile(QDir(writableLocation).filePath(QString("%1.dat").arg(Core::Extension::id)));
    QFile jsonFile(QDir(writableLocation).filePath(QString("%1.json").arg(Core::Extension::id)));

    // If there is an old file
    if (dataFile.exists()) {

        // Deserialize binary data
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug("[%s] Porting from binary file: %s", Core::Extension::id.toUtf8().constData(), dataFile.fileName().toLocal8Bit().data());
            QDataStream in(&dataFile);
            quint64 size;
            in >> size;
            SearchEngine se;
            bool enabled;
            QString name, trigger, url, iconPath;
            for (quint64 i = 0; i < size; ++i) {
                in >> enabled >> url >> name >> trigger >> iconPath;
                d->searchEngines.emplace_back(SearchEngine{enabled, name, url, trigger, iconPath});
            }
            dataFile.close();
        } else
            qWarning() << "Could not open file: " << dataFile.fileName();

        // Whatever remove it
        if ( !dataFile.remove() )
            qWarning() << "Could not remove file: " << dataFile.fileName();

        // Hmm what to do?

        // Serialize in json format
        d->serialize();

    } else if (!d->deserialize())
        d->restoreDefaults();
}



/** ***************************************************************************/
Websearch::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Websearch::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);
        d->widget->ui.tableView_searches->setModel(this);
        // TODO Fix all data() if least supported Qt supports its omittance
        connect(d->widget.data(), &ConfigWidget::restoreDefaults,
                std::bind(&WebsearchPrivate::restoreDefaults, std::ref(d)));
    }
    return d->widget;
}



/** ***************************************************************************/
void Websearch::Extension::handleQuery(Core::Query * query) {
    for (const SearchEngine &se : d->searchEngines)
        if (query->searchTerm().startsWith(se.trigger))
            query->addMatch(buildWebsearchItem(se, query->searchTerm().mid(se.trigger.size())), SHRT_MAX);
}



/** ***************************************************************************/
vector<shared_ptr<Core::Item>> Websearch::Extension::fallbacks(const QString & searchterm) {
    vector<shared_ptr<Core::Item>> res;
    for (const SearchEngine &se : d->searchEngines)
        if (se.enabled)
            res.push_back(buildWebsearchItem(se, searchterm));
    return res;
}



/** ***************************************************************************/
int Websearch::Extension::rowCount(const QModelIndex &) const {
    return static_cast<int>(d->searchEngines.size());
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
            || index.row() >= static_cast<int>(d->searchEngines.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return d->searchEngines[index.row()].name;
        case Section::Trigger:  return d->searchEngines[index.row()].trigger;
        case Section::URL:  return d->searchEngines[index.row()].url;
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return d->searchEngines[index.row()].name;
        case Section::Trigger:  return d->searchEngines[index.row()].trigger;
        case Section::URL:  return d->searchEngines[index.row()].url;
        default: return QVariant();
        }
    }
    case Qt::DecorationRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:{
            // Resizing request thounsands of repaints. Creating an icon for
            // ever paint event is to expensive. Therefor maintain an icon cache
            QString &iconPath = d->searchEngines[index.row()].iconPath;
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
        case Section::Enabled:  return (d->searchEngines[index.row()].enabled)?Qt::Checked:Qt::Unchecked;
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
            || index.row() >= static_cast<int>(d->searchEngines.size())
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
            d->searchEngines[index.row()].name = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            d->serialize();
            return true;
        case Section::Trigger:
            d->searchEngines[index.row()].trigger = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            d->serialize();
            return true;
        case Section::URL:
            d->searchEngines[index.row()].url = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            d->serialize();
            return true;
        default:
            return false;
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            d->searchEngines[index.row()].enabled = value.toBool();
            dataChanged(index, index, QVector<int>({Qt::CheckStateRole}));
            d->serialize();
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
                            .arg(d->searchEngines[index.row()].name)
                            .arg(i++)
                            .arg(fileInfo.suffix()));
        // Copy the file into cache dir
        } while (!QFile::copy(fileInfo.filePath(), newFilePath));
        // Set the copied file as icon
        d->searchEngines[index.row()].iconPath = newFilePath;
        dataChanged(index, index, QVector<int>({Qt::DecorationRole}));
        d->serialize();
        iconCache.clear();
        return true;
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
    if (position<0 || rows<1 || static_cast<int>(d->searchEngines.size())<position)
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = position; row < position + rows; ++row){
        d->searchEngines.insert(d->searchEngines.begin() + row,
                              SearchEngine({false,
                                            "<name>",
                                            "<http://url/containing/the/?query=%s>",
                                            "<trigger>",
                                            ":default"}));
    }
    endInsertRows();

    d->serialize();

    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::removeRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(d->searchEngines.size())<position+rows)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows-1);
    d->searchEngines.erase(d->searchEngines.begin()+position,d->searchEngines.begin()+(position+rows));
    endRemoveRows();

    d->serialize();

    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::moveRows(const QModelIndex &src, int srcRow, int cnt, const QModelIndex &dst, int dstRow) {
    if (srcRow<0 || cnt<1 || dstRow<0
            || static_cast<int>(d->searchEngines.size())<srcRow+cnt-1
            || static_cast<int>(d->searchEngines.size())<dstRow
            || ( srcRow<=dstRow && dstRow<srcRow+cnt) ) // If its inside the source do nothing
        return false;

    vector<SearchEngine> tmp;
    beginMoveRows(src, srcRow, srcRow+cnt-1, dst, dstRow);
    tmp.insert(tmp.end(), make_move_iterator(d->searchEngines.begin()+srcRow), make_move_iterator(d->searchEngines.begin() + srcRow+cnt));
    d->searchEngines.erase(d->searchEngines.begin()+srcRow, d->searchEngines.begin() + srcRow+cnt);
    const size_t finalDst = dstRow > srcRow ? dstRow - cnt : dstRow;
    d->searchEngines.insert(d->searchEngines.begin()+finalDst , make_move_iterator(tmp.begin()), make_move_iterator(tmp.end()));
    endMoveRows();

    d->serialize();

    return true;
}

