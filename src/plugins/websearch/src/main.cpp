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

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QUrl>
#include <map>
#include <vector>
#include "main.h"
#include "configwidget.h"
#include "enginesmodel.h"
#include "standarditem.h"
#include "standardaction.h"
#include "query.h"
using std::shared_ptr;
using std::vector;
using namespace Core;


namespace {

struct SearchEngine {
    bool    enabled;
    QString name;
    QString trigger;
    QString iconPath;
    QString url;
};

std::vector<Websearch::SearchEngine> defaultSearchEngines = {
    {true, "Google",        "gg ",  ":google",    "https://www.google.com/search?q=%s"},
    {true, "Youtube",       "yt ",  ":youtube",   "https://www.youtube.com/results?search_query=%s"},
    {true, "Amazon",        "ama ", ":amazon",    "http://www.amazon.com/s/?field-keywords=%s"},
    {true, "Ebay",          "eb ",  ":ebay",      "http://www.ebay.com/sch/i.html?_nkw=%s"},
    {true, "GitHub",        "gh ",  ":github",    "https://github.com/search?utf8=✓&q=%s"},
    {true, "Wikipedia",     "wp ",  ":wikipedia", "https://wikipedia.org/w/index.php?search=%s"},
    {true, "Wolfram Alpha", "=",    ":wolfram",   "https://www.wolframalpha.com/input/?i=%s"}
};

shared_ptr<Core::Item> buildWebsearchItem(const Websearch::SearchEngine &se, const QString &searchterm) {

    QString urlString = QString(se.url).replace("%s", QUrl::toPercentEncoding(searchterm));
    QUrl url = QUrl(urlString);
    QString desc = QString("Start %1 search in your browser").arg(se.name);

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
struct Websearch::Internal
{
    QPointer<ConfigWidget> widget;
    std::vector<SearchEngine> searchEngines;
};



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Websearch::Extension::Extension()
    : Core::Extension("org.albert.extension.websearch"),
      Core::QueryHandler(Core::Extension::id),
      d(new Internal) {

    QString writableLocation =  QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFile dataFile(QDir(writableLocation).filePath(QString("%1.dat").arg(Core::Extension::id)));
    QFile jsonFile(QDir(writableLocation).filePath(QString("%1.json").arg(Core::Extension::id)));

    // If there is an old file
    if (dataFile.exists()) {

        // Deserialize binary data
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug() << "Porting websearches from old format";
            QDataStream in(&dataFile);
            quint64 size;
            in >> size;
            SearchEngine se;
            for (quint64 i = 0; i < size; ++i) {
                in >> se.enabled >> se.url >> se.name >> se.trigger >> se.iconPath;
                d->searchEngines.push_back(se);
            }
            dataFile.close();
        } else
            qWarning() << qPrintable(QString("Could not open file '%1'").arg(dataFile.fileName()));

        // Whatever remove it
        if ( !dataFile.remove() )
            qWarning() << qPrintable(QString("Could not remove file '%1'").arg(dataFile.fileName()));

        // Hmm what to do?

        // Serialize in json format
        serialize();

    } else if (!deserialize())
        restoreDefaults();
}



/** ***************************************************************************/
Websearch::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Websearch::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){

        d->widget = new ConfigWidget(parent);
        EnginesModel *enginesModel = new EnginesModel(d->searchEngines,
                                                     d->widget->ui.tableView_searches);
        d->widget->ui.tableView_searches->setModel(enginesModel);

        // Serialize engines if anything changed
        connect(enginesModel, &EnginesModel::dataChanged,
                this, &Extension::serialize);
        connect(enginesModel, &EnginesModel::rowsInserted,
                this, &Extension::serialize);
        connect(enginesModel, &EnginesModel::rowsRemoved,
                this, &Extension::serialize);
        connect(enginesModel, &EnginesModel::rowsMoved,
                this, &Extension::serialize);

        // TODO Fix all data() if least supported Qt supports its omittance
        connect(d->widget.data(), &ConfigWidget::restoreDefaults,
                this, &Extension::restoreDefaults);
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
bool Websearch::Extension::deserialize() {

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
               .filePath(QString("%1.json").arg(Core::Extension::id)));

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << qPrintable(QString("Could not open file: '%1'.").arg(file.fileName()));
        return false;
    }

    QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();

    SearchEngine searchEngine;
    for ( const QJsonValue& value : array) {
        QJsonObject object = value.toObject();
        searchEngine.enabled  = object["enabled"].toBool();
        searchEngine.name     = object["name"].toString();
        searchEngine.trigger  = object["trigger"].toString();
        searchEngine.iconPath = object["iconPath"].toString();
        searchEngine.url      = object["url"].toString();
        d->searchEngines.push_back(searchEngine);
    }

    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::serialize() {

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
               .filePath(QString("%1.json").arg(Core::Extension::id)));

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << qPrintable(QString("Could not open file: '%1'.").arg(file.fileName()));
        return false;
    }

    QJsonArray array;

    for ( const SearchEngine& searchEngine : d->searchEngines ) {
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
void Websearch::Extension::restoreDefaults() {
    /* Init std searches */
    d->searchEngines = defaultSearchEngines;
    serialize();

    if (!d->widget.isNull())
        d->widget->ui.tableView_searches->reset();
}
