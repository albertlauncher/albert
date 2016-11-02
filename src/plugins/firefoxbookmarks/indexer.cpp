// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
//                    2016 Martin Buergmann
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
#include <QUrl>
#include <QClipboard>
#include <QDesktopServices>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <functional>
#include <vector>
#include <memory>
#include "indexer.h"
#include "extension.h"
#include "standardobjects.h"
#include "albertapp.h"
using std::shared_ptr;
using std::vector;


/** ***************************************************************************/
void FirefoxBookmarks::Extension::Indexer::run() {

    if (!base_.open()) {
        qWarning("Could not open places.sqlite");
        return;
    }

    // Notification
    qDebug("[%s] Start indexing in background thread.", extension_->name().toStdString().c_str());
    emit statusInfo("Indexing bookmarks ...");

    // Build a new index
    vector<SharedStdIdxItem> bookmarks;

    QSqlQuery result(base_);
    if (!result.exec("SELECT b.id,b.title,p.url FROM moz_bookmarks b JOIN moz_places p ON b.fk = p.id")) {
        qWarning() << result.lastError().text();
        return;
    }

    while (result.next()) {
        QString id = result.value("id").toString();
        QString title = result.value("title").toString();
        QString urlstr = result.value("url").toString();

        if (title.isEmpty()) continue;  // This is a folder or something else

        SharedStdIdxItem ssii  = std::make_shared<StandardIndexItem>(id);
        ssii->setText(title);
        ssii->setSubtext(urlstr);
        ssii->setIconPath(":favicon");

        std::vector<IIndexable::WeightedKeyword> weightedKeywords;
        QUrl url(urlstr);
        QString host = url.host();
        weightedKeywords.emplace_back(title, USHRT_MAX);
        weightedKeywords.emplace_back(host.left(host.size()-url.topLevelDomain().size()), USHRT_MAX/2);
        ssii->setIndexKeywords(std::move(weightedKeywords));

        std::vector<SharedAction> actions;
        SharedStdAction action = std::make_shared<StandardAction>();
        action->setText("Open in default browser");
        action->setAction([urlstr](ExecutionFlags*){
            QDesktopServices::openUrl(QUrl(urlstr));
        });
        actions.push_back(std::move(action));

        action = std::make_shared<StandardAction>();
        action->setText("Copy url to clipboard");
        action->setAction([urlstr](ExecutionFlags*){
            QApplication::clipboard()->setText(urlstr);
        });
        actions.push_back(std::move(action));

        ssii->setActions(std::move(actions));

        bookmarks.push_back(std::move(ssii));

        if (abort_) {
            base_.close();
            return;
        }
    }

    base_.close();

    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    QMutexLocker locker(&extension_->indexAccess_);

    // Abortion requested while block
    if (abort_)
        return;

    // Set the new index (use swap to shift destruction out of critical area)
    std::swap(extension_->index_, bookmarks);

    // Rebuild the offline index
    extension_->offlineIndex_.clear();
    for (const auto &item : extension_->index_)
        extension_->offlineIndex_.add(item);

    // Notification
    qDebug("[%s] Indexing done (%d items)", extension_->id.toUtf8().constData(), static_cast<int>(extension_->index_.size()));
    emit statusInfo(QString("Indexed %1 bookmarks").arg(extension_->index_.size()));
}
