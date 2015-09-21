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

#pragma once
#include <QRunnable>
#include <QMimeDatabase>
#include <QMutex>
#include <QList>
#include "utils/search/search.h"
#include "file.h"

namespace Files{

class IndexOptions;

class ScanWorker final : public QObject, public QRunnable {

    Q_OBJECT

public:
    ScanWorker(QList<File *>** fileIndex, Search* searchIndex, const QStringList& rootPaths, const IndexOptions& indexOptions, QMutex* searchLock);
    void run() override;
    inline void abort(){ _abort = true; }

private:
    void scan(const QFileInfo& fi, QList<File *>* result);

    QMimeDatabase       _mimeDatabase;
    QList<File*>        **_fileIndex;
    Search              *_searchIndex;
    const QStringList   &_rootDirs;
    const IndexOptions  &_indexOptions;
    QMutex              *_mutex;
    bool _abort;

    static constexpr const char* IGNOREFILE = ".albertignore";

signals:
    void statusInfo(const QString&);
};
}
