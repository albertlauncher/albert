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
#include <set>
#include <functional>
#include "indexer.h"
#include "file.h"
#include "extension.h"


/** ***************************************************************************/
void Files::Indexer::run() {

    // Notification
    QString msg("Indexing files ...");
    emit extension_->statusInfo(msg);
    qDebug() << "[Files]" << msg;


    // Prepare the iterator properties
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    if (extension_->indexHidden_)
        filters |= QDir::Hidden;
    QDirIterator::IteratorFlags flags;
    if (extension_->followSymlinks_)
        flags = QDirIterator::FollowSymlinks;


    // Get a new index
    std::vector<shared_ptr<AlbertItem>> newIndex;
    std::set<QString> indexedDirs;


    // Anonymous function that implemnents the index recursion
    std::function<void(const QFileInfo&)> indexRecursion =
            [this, &newIndex, &indexedDirs, &filters, &flags, &indexRecursion](const QFileInfo& fileInfo){
        if (abort_) return;

        QString canonicalPath = fileInfo.canonicalFilePath();


        if (fileInfo.isFile()) {

            // If the file matches the index options, index it
            QMimeType mimetype = mimeDatabase_.mimeTypeForFile(canonicalPath);
            QString mimeName = mimetype.name();
            if ((extension_->indexAudio_ && mimeName.startsWith("audio"))
                    ||(extension_->indexVideo_ && mimeName.startsWith("video"))
                    ||(extension_->indexImage_ && mimeName.startsWith("image"))
                    ||(extension_->indexDocs_ &&
                       (mimeName.startsWith("application") || mimeName.startsWith("text")))) {
                newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));
            }
        } else if (fileInfo.isDir()) {

            emit extension_->statusInfo(QString("Indexing %1.").arg(canonicalPath));

            // Skip if this dir has already been indexed
            if (indexedDirs.find(canonicalPath)!=indexedDirs.end()){
                return;
            }

            // If the dir matches the index options, index it
            if (extension_->indexDirs_) {
                QMimeType mimetype = mimeDatabase_.mimeTypeForFile(canonicalPath);
                newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));
            }

            // Ignore ignorefile by default
            std::vector<QRegExp> ignores;
            ignores.push_back(QRegExp(extension_->IGNOREFILE, Qt::CaseSensitive, QRegExp::Wildcard));

            // Read the ignore file, see http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
            QFile file(QDir(canonicalPath).filePath(extension_->IGNOREFILE));
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
    for (const QString& rootDir : extension_->rootDirs_) {
        indexRecursion(QFileInfo(rootDir));
        if (abort_) return;
    }

    // Sort the new index for linear usage copy [O(n*log(n))]
    std::sort(newIndex.begin(), newIndex.end(),
              [](const shared_ptr<AlbertItem> &lhs, const shared_ptr<AlbertItem> &rhs) {
                  return QString::compare(std::static_pointer_cast<File>(lhs)->path(),
                                          std::static_pointer_cast<File>(rhs)->path(),
                                          Qt::CaseInsensitive) < 0;
              });

    // Copy the usagecounters  [O(n)]
    size_t i=0, j=0;
    while (i < extension_->index_.size() && j < newIndex.size()) {

        shared_ptr<File> oldFile
                = std::static_pointer_cast<File>(extension_->index_[i]);
        shared_ptr<File> newFile
                = std::static_pointer_cast<File>(newIndex[j]);

        if (oldFile->path_ == newFile->path_) {
            newFile->usage_ = oldFile->usage_;
            ++i;++j;
        } else if (oldFile->path_ < newFile->path_ ) {
            ++i;
        } else {// if (oldFile->path_ > newFile->path_ ) {
            ++j;
        }
    }

    // ▼ CRITICAL: Set the new index ▼
    extension_->indexAccess_.lock();
    extension_->index_ = std::move(newIndex);
    extension_->indexAccess_.unlock();
    // ▲ CRITICAL: Set the new index ▲

    emit extension_->staticItemsChanged(extension_);

    // Notification
    msg = QString("Indexed %1 files.").arg(extension_->index_.size());
    emit extension_->statusInfo(msg);
    qDebug() << "[Files]" << msg;
}
