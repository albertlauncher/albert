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
        return QString::compare(lhs->absolutePath(), rhs->absolutePath(), Qt::CaseInsensitive) < 0;
    }
};



/** ***************************************************************************/
ScanWorker::ScanWorker(QList<File *>** fileIndex, Search *searchIndex, const QStringList& rootPaths, const IndexOptions& indexOptions, QMutex* searchLock) :
    _fileIndex(fileIndex), _searchIndex(searchIndex), _rootDirs(rootPaths), _indexOptions(indexOptions), _mutex(searchLock), _abort(false) {}



/** ***************************************************************************/
void ScanWorker::run() {
    // Get a new index [O(n)]
    QList<File *>* newIndex = new QList<File *>;
    for (const QString& path : _rootDirs)
        indexRecursive(QFileInfo(path), newIndex);

    // Sort the new index  [O(n*log(n))]
    std::sort(newIndex->begin(), newIndex->end(), Comp());

    // Copy the usagecounters  [O(n)]
    int i=0, j=0;
    while (i < (*_fileIndex)->size() && j < newIndex->size()) {
        if ((**_fileIndex)[i]->absolutePath()==(*newIndex)[j]->absolutePath()){
            (*newIndex)[j]->_usage = (**_fileIndex)[i]->_usage;
            ++i;++j;
        } else if ((**_fileIndex)[i]->absolutePath() < (*newIndex)[j]->absolutePath()){
            ++i;
        } else {// if ((*_fileIndex)[i]->absolutePath() > (*newIndex)[j]->absolutePath()){
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
}



/** ***************************************************************************/
void ScanWorker::indexRecursive(const QFileInfo& fi, QList<File *>* result)
{
    File *sf = new File(fi.absoluteFilePath(), _mimeDatabase.mimeTypeForFile(fi));
    QString mimeName = sf->mimetype().name();

    // If is a file and matches index options index it
    if (fi.isFile() && ((_indexOptions.indexAudio && mimeName.startsWith("audio"))
        ||(_indexOptions.indexVideo && mimeName.startsWith("video"))
        ||(_indexOptions.indexImage && mimeName.startsWith("image"))
        ||(_indexOptions.indexDocs && mimeName.startsWith("application")))) {
        result->append(sf);
    } else if (fi.isDir()) {
        emit statusInfo(QString("Indexing %1.").arg(fi.absoluteFilePath()));

        // If is a dir and matches index options index it
        if (_indexOptions.indexDirs)
            result->append(sf);

        // Read .albertignore
        // http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
        QList<QRegExp> ignores;
        QFile file(fi.filePath() + "/.albertignore");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            while (!in.atEnd())
                ignores.append(QRegExp(in.readLine().trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
        }

        //  Ignore ignorefile by default
        ignores.append(QRegExp(".albertignore", Qt::CaseSensitive, QRegExp::Wildcard));

        // Iterate over all files in the dir and do recursion
        QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot|QDir::NoSymLinks;
        if (_indexOptions.indexHidden) filters |= QDir::Hidden;
        QDirIterator dirIterator(fi.absoluteFilePath(), filters);
        while (dirIterator.hasNext() && !_abort) {
            dirIterator.next();

            // Skip if this file matches one of the ignore patterns
            for (QRegExp& ignore : ignores)
                if(ignore.exactMatch(dirIterator.fileName()))
                    goto CONTINUE;

            indexRecursive(dirIterator.fileInfo(), result);
            CONTINUE:
            continue; // gnah does not compile without this ...
        }
    }
}
}
