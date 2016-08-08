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

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <functional>
#include <vector>
#include <memory>
#include "indexer.h"
#include "bookmark.h"
#include "extension.h"
using std::shared_ptr;
using std::vector;


/** ***************************************************************************/
void ChromeBookmarks::Extension::Indexer::run() {

    // Notification
    qDebug("[%s] Start indexing in background thread.", extension_->id);
    emit statusInfo("Indexing bookmarks ...");

    // Build a new index
    vector<shared_ptr<Bookmark>> newIndex;

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
            // TODO ADD THE FOLDERS to the indexKeywords
            newIndex.push_back(
                    std::make_shared<Bookmark>(
                            json["id"].toString(),
                            json["name"].toString(),
                            json["url"].toString()
                    )
            );
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

    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    QMutexLocker locker(&extension_->indexAccess_);

    // Abortion requested while block
    if (abort_)
        return;

    // Set the new index (use swap to shift destruction out of critical area)
    std::swap(extension_->index_, newIndex);

    // Rebuild the offline index
    extension_->offlineIndex_.clear();
    for (const auto &item : extension_->index_)
        extension_->offlineIndex_.add(item);

    /*
     * Finally update the watches (maybe folders changed)
     * Note that QFileSystemWatcher stops monitoring files once they have been
     * renamed or removed from disk, and directories once they have been removed
     * from disk.
     * Chromium seems to mv the file (inode change), removing is not necessary.
     */
    if(!extension_->watcher_.addPath(extension_->bookmarksFile_)) // No clue why this should happen
        qCritical() << extension_->bookmarksFile_
                    <<  "could not be watched. Changes in this path will not be noticed.";

    // Notification
    qDebug("[%s] Indexing done (%d items)", extension_->id, static_cast<int>(extension_->index_.size()));
   emit statusInfo(QString("Indexed %1 bookmarks").arg(extension_->index_.size()));
}
