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

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <functional>
#include <vector>
#include <memory>
using std::shared_ptr;
#include <vector>
using std::vector;
#include "indexer.h"
#include "bookmark.h"
#include "extension.h"


/** ***************************************************************************/
void ChromeBookmarks::Indexer::run() {

    // Notification
    QString msg("Indexing bookmarks ...");
    emit extension_->statusInfo(msg);
    qDebug() << "[ChromeBookmarks]" << msg;

    // Build a new index
    vector<shared_ptr<AlbertItem>> newIndex;

    // Define a recursive bookmark indexing lambda
    std::function<void(const QJsonObject &json)> rec_bmsearch =
            [&rec_bmsearch, &newIndex](const QJsonObject &json) {
        QJsonValue type = json["type"];
        if (type == QJsonValue::Undefined)
            return;
        if (type.toString() == "folder"){
            QJsonArray jarr = json["children"].toArray();
            for (const QJsonValue &i : jarr)
                rec_bmsearch(i.toObject());
        }
        if (type.toString() == "url") {
            // TODO ADD THE FOLDERS to the aliases
            newIndex.push_back(std::make_shared<Bookmark>(
                                 json["name"].toString(), json["url"].toString(), 0));
        }
    };

    QFile f(extension_->bookmarksFile_);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open " << extension_->bookmarksFile_;
        return;
    }

    QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
    QJsonObject roots = json.value("roots").toObject();
    for (const QJsonValue &i : roots)
        if (i.isObject())
            rec_bmsearch(i.toObject());

    f.close();

    // Sort the new index for linear usage copy [O(n*log(n))]
    std::sort(newIndex.begin(), newIndex.end(),
              [&](const shared_ptr<AlbertItem> lhs, const shared_ptr<AlbertItem> rhs) {
                  return QString::compare(std::static_pointer_cast<Bookmark>(lhs)->url(),
                                          std::static_pointer_cast<Bookmark>(rhs)->url(),
                                          Qt::CaseInsensitive) < 0;
              });

    // Copy the usagecounters  [O(n)]
    size_t i=0, j=0;
    while (i < extension_->index_.size() && j < newIndex.size()) {

        shared_ptr<Bookmark> oldBookmark
                = std::static_pointer_cast<Bookmark>(extension_->index_[i]);
        shared_ptr<Bookmark> newBookmark
                = std::static_pointer_cast<Bookmark>(newIndex[j]);

        if (oldBookmark->url_ == newBookmark->url_) {
            newBookmark->usage_ = oldBookmark->usage_;
            ++i;++j;
        } else if (oldBookmark->url_ < newBookmark->url_ ) {
            ++i;
        } else {// if (oldBookmark->url_ > newBookmark->url_ ) {
            ++j;
        }
    }

    // ▼ CRITICAL: Set the new index ▼
    extension_->indexAccess_.lock();
    extension_->index_ = std::move(newIndex);
    extension_->indexAccess_.unlock();
    // ▲ CRITICAL: Set the new index ▲

    emit extension_->staticItemsChanged(extension_);

    // Finally update the watches (maybe folders changed)
    extension_->watcher_.removePaths(extension_->watcher_.files());
    if(!extension_->watcher_.addPath(extension_->bookmarksFile_)) // No clue why this should happen
        qCritical() << extension_->bookmarksFile_
                    <<  "could not be watched. Changes in this path will not be noticed.";

    // Notification
    msg = QString("Indexed %1 bookmarks.").arg(extension_->index_.size());
    emit extension_->statusInfo(msg);
    qDebug() << "[ChromeBookmarks]" << msg;
}
