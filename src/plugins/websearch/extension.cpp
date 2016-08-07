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
#include "searchenginesmodel.h"
#include "query.h"



/** ***************************************************************************/
Websearch::Extension::Extension() : AbstractExtension("org.albert.extension.websearch")  {

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
            SearchEngine se;
            for (quint64 i = 0; i < size; ++i) {
                se.deserialize(in);
                searchEngines_.push_back(se);
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
        out << static_cast<quint64>(searchEngines_.size());
        for (const SearchEngine &se : searchEngines_)
            se.serialize(out);
        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();
}



/** ***************************************************************************/
QWidget *Websearch::Extension::widget(QWidget *parent) {
    if (widget_.isNull()){
        widget_ = new ConfigWidget(parent);
        SearchEnginesModel *sem = new SearchEnginesModel(searchEngines_, widget_->ui.tableView_searches);
        widget_->ui.tableView_searches->setModel(sem);
        connect(sem, &SearchEnginesModel::fallBackChanged, [this](){
            emit fallBacksChanged();
        });
        // TODO Fix all *.data if least supported Qt supports its omittance
        QObject::connect(widget_.data(), &ConfigWidget::restoreDefaults,
                this, &Extension::restoreDefaults);
    }
    return widget_;
}



/** ***************************************************************************/
QStringList Websearch::Extension::triggers() const {
    QStringList triggers;
    for (const auto &searchEngine : searchEngines_)
        triggers.push_back(searchEngine.trigger());
    return triggers;
}


/** ***************************************************************************/
void Websearch::Extension::handleQuery(Query query) {
    for (const SearchEngine &se : searchEngines_)
        if (query.searchTerm().startsWith(se.trigger()))
            query.addMatch(se.buildWebsearchItem(query.searchTerm().mid(se.trigger().size())));
}



/** ***************************************************************************/
vector<SharedItem> Websearch::Extension::fallbacks(QString searchterm) const {
    vector<SharedItem> res;
    for (const SearchEngine &se : searchEngines_)
        if (se.enabled())
            res.push_back(se.buildWebsearchItem(searchterm));
    return res;
}



/** ***************************************************************************/
void Websearch::Extension::restoreDefaults() {
    /* Init std searches */
    searchEngines_.clear();

    searchEngines_.emplace_back("Google", "https://www.google.com/search?q=%s", "gg ", ":google");
    searchEngines_.emplace_back("Youtube", "https://www.youtube.com/results?search_query=%s", "yt ", ":youtube");
    searchEngines_.emplace_back("Amazon", "http://www.amazon.com/s/?field-keywords=%s", "ama ", ":amazon");
    searchEngines_.emplace_back("Ebay", "http://www.ebay.com/sch/i.html?_nkw=%s", "eb ", ":ebay");
    searchEngines_.emplace_back("GitHub", "https://github.com/search?utf8=✓&q=%s", "gh ", ":github");
    searchEngines_.emplace_back("Wolfram Alpha", "https://www.wolframalpha.com/input/?i=%s", "=", ":wolfram");

    if (!widget_.isNull())
        widget_->ui.tableView_searches->reset();
}
