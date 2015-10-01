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
    QString msg("Indexing desktop entries ...");
    emit statusInfo(msg);
    qDebug() << "[Applications]" << msg;


    // Get a new index [O(n)]
    vector<shared_ptr<Application>> newIndex;

    // Iterate over all desktop files
    for (const QString &path : _extension->_rootDirs) {
        QDirIterator fIt(path, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (fIt.hasNext()) {
            if (_abort) return;
            QString path = fIt.next();

            // Make new entry
            shared_ptr<Application> application;

            // If it is already in the index copy usage
            vector<shared_ptr<Application>>::iterator indexIt =
                    std::find_if(_extension->_appIndex.begin(),
                                _extension->_appIndex.end(),
                                [&path](const shared_ptr<Application>& de){
                                    return de->path()== path;
                                });
            if (indexIt != _extension->_appIndex.end())
                application = std::make_shared<Application>(path, (*indexIt)->usage());
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
    _extension->_indexAccess.lock();

    // Set the new index
    _extension->_appIndex = std::move(newIndex);

    // Reset the offline index
    _extension->_searchIndex.clear();

    // Build the new offline index
    for (const shared_ptr<Application> &de : _extension->_appIndex)
        _extension->_searchIndex.add(de);

    // Unlock the accress
    _extension->_indexAccess.unlock();

    /*
     *  ▲ CRITICAL ▲
     */


    // Finally update the watches (maybe folders changed)
    _extension->_watcher.removePaths(_extension->_watcher.directories());
    for (const QString &path : _extension->_rootDirs) {
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            _extension->_watcher.addPath(dit.next());
    }


    // Notification
    msg = QString("Indexed %1 desktop entries.").arg(_extension->_appIndex.size());
    emit statusInfo(msg);
    qDebug() << "[Applications]" << msg;
}
