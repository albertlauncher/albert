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
#include <map>
#include <functional>
#include "indexer.h"
#include "file.h"
#include "extension.h"


/** ***************************************************************************/
void Files::Indexer::run() {

    // Notification
    QString msg("Indexing files ...");
    emit statusInfo(msg);
    qDebug() << "[Files]" << msg;


    // Prepare the iterator properties
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    if (_extension->_indexHidden)
        filters |= QDir::Hidden;
    QDirIterator::IteratorFlags flags;
    if (_extension->_followSymlinks)
        flags = QDirIterator::FollowSymlinks;


    // Get a new index [O(n)]
    std::vector<shared_ptr<File>> newIndex;
    std::set<QString> indexedDirs;


    // Anonymous function that implemnents the index recursion
    std::function<void(const QFileInfo&)> indexRecursion = [&](const QFileInfo& fileInfo){
        if (_abort) return;

        QString canonicalPath = fileInfo.canonicalFilePath();


        if (fileInfo.isFile()) {

            // If the file matches the index options, index it
            QMimeType mimetype = _mimeDatabase.mimeTypeForFile(canonicalPath);
            QString mimeName = mimetype.name();
            if ((_extension->_indexAudio && mimeName.startsWith("audio"))
                    ||(_extension->_indexVideo && mimeName.startsWith("video"))
                    ||(_extension->_indexImage && mimeName.startsWith("image"))
                    ||(_extension->_indexDocs && mimeName.startsWith("application"))) {
                newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));
            }


        } else if (fileInfo.isDir()) {

            emit statusInfo(QString("Indexing %1.").arg(canonicalPath));


            // Skip if this dir has already been indexed
            if (indexedDirs.find(canonicalPath)!=indexedDirs.end()){
                return;
            }

            // If the dir matches the index options, index it
            if (_extension->_indexDirs) {
                QMimeType mimetype = _mimeDatabase.mimeTypeForFile(canonicalPath);
                newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));
            }

            // Ignore ignorefile by default
            std::vector<QRegExp> ignores;
            ignores.push_back(QRegExp(_extension->IGNOREFILE, Qt::CaseSensitive, QRegExp::Wildcard));

            // Read the ignore file, see http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
            QFile file(QDir(canonicalPath).filePath(_extension->IGNOREFILE));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd())
                    ignores.push_back(QRegExp(in.readLine().trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
                file.close();
            }

            // Index all children in the dir
            QDirIterator dirIterator(canonicalPath, filters, flags);
            while (dirIterator.hasNext()) {
                dirIterator.next();

                // Skip if this file matches one of the ignore patterns
                for (QRegExp& ignore : ignores){
                    QString s = dirIterator.fileName(); // This is insane works only if its a lvalue
                    if(ignore.exactMatch(s))
                        goto SKIP_THIS;
                }

                // Index this file
                indexRecursion(dirIterator.fileInfo());
                SKIP_THIS:;
            }

            // Remember that this dir has been indexed to avoid loops
            indexedDirs.insert(canonicalPath);
        }
    };


    // Start the indexing
    for (const QString& rootDir : _extension->_rootDirs) {
        indexRecursion(QFileInfo(rootDir));
        if (_abort) return;
    }


    // Sort the new index for linear usage copy [O(n*log(n))]
    emit statusInfo("Sorting ... ");
    std::sort(newIndex.begin(), newIndex.end(), [&](const shared_ptr<File> lhs, const shared_ptr<File> rhs) {
                  return QString::compare(lhs->path(), rhs->path(), Qt::CaseInsensitive) < 0;
              });


    // Copy the usagecounters  [O(n)]
    emit statusInfo("Copy usage statistics ... ");
    size_t i=0, j=0;
    while (i < _extension->_fileIndex.size() && j < newIndex.size()) {
        if (_extension->_fileIndex[i]->path_ == newIndex[j]->path_) {
            newIndex[j]->usage_ = _extension->_fileIndex[i]->usage_;
            ++i;++j;
        } else if (_extension->_fileIndex[i]->path_ < newIndex[j]->path_) {
            ++i;
        } else {// if ((*_fileIndex)[i]->path > (*newIndex)[j]->path) {
            ++j;
        }
    }

    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    _extension->_indexAccess.lock();

    // Set the new index
    _extension->_fileIndex = std::move(newIndex);

    // Reset the offline index
    emit statusInfo("Build offline index... ");
    _extension->_searchIndex.clear();

    // Build the new offline index
    for (shared_ptr<IIndexable> i : _extension->_fileIndex)
        _extension->_searchIndex.add(i);

    // Unlock the accress
    _extension->_indexAccess.unlock();

    /*
     *  ▲ CRITICAL ▲
     */


    // Notification
    msg = QString("Indexed %1 files.").arg(_extension->_fileIndex.size());
    emit statusInfo(msg);
    qDebug() << "[Files]" << msg;
}
