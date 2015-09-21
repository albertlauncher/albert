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

#include "scanworker.h"
#include <QDirIterator>
#include <functional>
#include <QDebug>
#include "extension.h"


namespace Files{



/** ***************************************************************************/
struct Comp {
    inline bool operator()(const File *lhs, const File *rhs) const {
        return QString::compare(lhs->path, rhs->path, Qt::CaseInsensitive) < 0;
    }
};



/** ***************************************************************************/
ScanWorker::ScanWorker(QList<File *>** fileIndex, Search *searchIndex, const QStringList& rootPaths, const IndexOptions& indexOptions, QMutex* searchLock) :
    _fileIndex(fileIndex), _searchIndex(searchIndex), _rootDirs(rootPaths), _indexOptions(indexOptions), _mutex(searchLock), _abort(false) {}



/** ***************************************************************************/
void ScanWorker::run() {
    qDebug() << "[Files] Scanning files...";
    // Get a new index [O(n)]
    QList<File *>* newIndex = new QList<File *>;
    for (const QString& path : _rootDirs)
        scan(QFileInfo(path), newIndex);

    // Sort the new index  [O(n*log(n))]
    std::sort(newIndex->begin(), newIndex->end(), Comp());

    // Copy the usagecounters  [O(n)]
    int i=0, j=0;
    while (i < (*_fileIndex)->size() && j < newIndex->size()) {
        if ((**_fileIndex)[i]->path==(*newIndex)[j]->path){
            (*newIndex)[j]->usage = (**_fileIndex)[i]->usage;
            ++i;++j;
        } else if ((**_fileIndex)[i]->path < (*newIndex)[j]->path){
            ++i;
        } else {// if ((*_fileIndex)[i]->path > (*newIndex)[j]->path){
            ++j;
        }
    }

    // Set the new index
    _mutex->lock();
    std::swap(*_fileIndex, newIndex);
    _searchIndex->clear();
    for (IIndexable *i : **_fileIndex)
        _searchIndex->add(i);
    _mutex->unlock();

    // Delete the old index
    for (File* f : *newIndex)
        delete f;
    delete newIndex;

    emit statusInfo(QString("Done. Indexed %1 files.").arg((*_fileIndex)->size()));
    qDebug() << "[Files] Scanning files done.";
}



/** ***************************************************************************/
void ScanWorker::scan(const QFileInfo& root, QList<File *>* result) {
    // Prepare
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    if (_indexOptions.indexHidden)
        filters |= QDir::Hidden;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    if (!_indexOptions.followSymlinks)
        flags |= QDirIterator::FollowSymlinks;

    File f;
    QMap<QString, QList<QRegExp>> ignoreMap;

    // Iterate over all files in the dir and do recursion
    QDirIterator dirIt(root.absoluteFilePath(), filters, flags);
    while (dirIt.hasNext() && !_abort) {
        f.path = dirIt.next();

        // Handle the ignore files
        // http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
        if (!ignoreMap.contains(dirIt.path())){
            QList<QRegExp> ignores;
            QFile file(QDir(dirIt.path()).filePath(IGNOREFILE));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
                QTextStream in(&file);
                while (!in.atEnd())
                    ignores.append(QRegExp(in.readLine().trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
            }
            //  Ignore ignorefile by default
            ignores.append(QRegExp(".albertignore", Qt::CaseSensitive, QRegExp::Wildcard));
            ignoreMap.insert(dirIt.path(), ignores);
        }

        // Skip if this file matches one of the ignore patterns
        for (QRegExp& ignore : ignoreMap[dirIt.path()])
            if(ignore.exactMatch(dirIt.fileName()))
                continue;

        // If is a file and matches index options index it
        if (dirIt.fileInfo().isFile()){
            f.mimetype = _mimeDatabase.mimeTypeForFile(f.path);
            QString mimeName = f.mimetype.name();
            if ((_indexOptions.indexAudio && mimeName.startsWith("audio"))
                    ||(_indexOptions.indexVideo && mimeName.startsWith("video"))
                    ||(_indexOptions.indexImage && mimeName.startsWith("image"))
                    ||(_indexOptions.indexDocs && mimeName.startsWith("application"))) {
                result->append(new File(f));
            }
        } else if (dirIt.fileInfo().isDir()) {
            emit statusInfo(QString("Indexing %1.").arg(f.path));
            // If is a dir and matches index options index it
            if (_indexOptions.indexDirs){
                f.mimetype = _mimeDatabase.mimeTypeForFile(f.path);
                result->append(new File(f));
            }
        }
    }
}
}
