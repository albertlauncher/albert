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

#include <QDirIterator>
#include <QDebug>
#include <QThread>
#include <vector>
using std::vector;
#include <memory>
using std::shared_ptr;
#include <algorithm>
#include "indexer.h"
#include "application.h"
#include "extension.h"


/** ***************************************************************************/
void Applications::Indexer::run() {

    // Notification
    qDebug("[%s] Start indexing in background thread", extension_->name);
    emit statusInfo("Indexing desktop entries ...");

    // Get a new index [O(n)]
    vector<shared_ptr<Application>> newIndex;

    // Iterate over all desktop files
    for (const QString &path : extension_->rootDirs_) {
        QDirIterator fIt(path, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (fIt.hasNext()) {
            if (abort_) return;
            QString path = fIt.next();

            // Make new entry
            shared_ptr<Application> application;

            // If it is already in the index copy usage
            vector<shared_ptr<Application>>::iterator indexIt =
                    std::find_if(extension_->index_.begin(),
                                 extension_->index_.end(),
                                 [&path](const shared_ptr<Application>& de){ return de->path() == path; });

            if (indexIt != extension_->index_.end())
                application = std::make_shared<Application>(path, (*indexIt)->usageCount());
            else
                application = std::make_shared<Application>(path, 1);

            // Try to read the desktop entry
            if (!application->readDesktopEntry())
                continue;

            // Everthing okay, index it
            newIndex.push_back(application);
        }
    }


    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    extension_->indexAccess_.lock();

    // Set the new index
    extension_->index_ = std::move(newIndex);

    // Reset the offline index
    extension_->searchIndex_.clear();

    // Build the new offline index
    for (const shared_ptr<Application> &de : extension_->index_)
        extension_->searchIndex_.add(de);

    // Unlock the accress
    extension_->indexAccess_.unlock();

    // Finally update the watches (maybe folders changed)
    extension_->watcher_.removePaths(extension_->watcher_.directories());
    for (const QString &path : extension_->rootDirs_) {
        extension_->watcher_.addPath(path);
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            extension_->watcher_.addPath(dit.next());
    }

    /*
     *  ▲ CRITICAL ▲
     */

    // Notification
    qDebug("[%s] Indexing done (%d items)", extension_->name, static_cast<int>(extension_->index_.size()));
    emit statusInfo(QString("Indexed %1 desktop entries").arg(extension_->index_.size()));
}
