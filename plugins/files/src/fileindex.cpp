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

#include "fileindex.h"
#include <QDebug>
#include <QSettings>
#include <QFileInfo>
#include <functional>

namespace Files{
/** ***************************************************************************/
FileIndex::FileIndex()
{
    // Check if SQLite driver is available
    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
        qFatal("SQLite 3.x driver unavailabe. Check Qt build!");

    /* Initialize the database */
    _db = QSqlDatabase::addDatabase("QSQLITE");
    if (!_db.isValid())
        qFatal("Could not add the database!");

    _db.setDatabaseName(
                QString("%1/%2").arg(
                            QStandardPaths::writableLocation(QStandardPaths::DataLocation),"files.db3"));
    _mutex.lock();
    if (!_db.open())
        qFatal("Could not open database: %s", _db.lastError().text().toLocal8Bit().constData());
    _mutex.unlock();
    createTableIfNotExists();

    // Load settings
    QSettings s;
    _indexOptions.indexAudio = s.value(CFG_INDEX_AUDIO, CFG_INDEX_AUDIO).toBool();
    _indexOptions.indexVideo = s.value(CFG_INDEX_VIDEO, CFG_INDEX_VIDEO).toBool();
    _indexOptions.indexImage = s.value(CFG_INDEX_IMAGE, CFG_INDEX_IMAGE).toBool();
    _indexOptions.indexDocs  = s.value(CFG_INDEX_DOC, CFG_INDEX_DOC).toBool();
    _indexOptions.indexDirs  = s.value(CFG_INDEX_DIR, CFG_INDEX_DIR).toBool();
    _indexOptions.indexHidden= s.value(CFG_INDEX_HIDDEN, CFG_INDEX_HIDDEN).toBool();

    /* Create index*/
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        _rootDirs = v.toStringList();
    else
        restorePaths();
}


/** ***************************************************************************/
FileIndex::~FileIndex()
{
    /* Close database */
    _db.close();

    /* Save settings */
    QSettings s;
    s.setValue(CFG_PATHS, _rootDirs);
    s.setValue(CFG_INDEX_AUDIO, _indexOptions.indexAudio);
    s.setValue(CFG_INDEX_VIDEO, _indexOptions.indexVideo);
    s.setValue(CFG_INDEX_IMAGE, _indexOptions.indexImage);
    s.setValue(CFG_INDEX_DIR, _indexOptions.indexDirs);
    s.setValue(CFG_INDEX_DOC, _indexOptions.indexDocs);
    s.setValue(CFG_INDEX_HIDDEN,_indexOptions.indexHidden);
}


/** ***************************************************************************
 * @brief FileIndex::addDir
 * @param dirPath
 * @return 0 success, 1 does not exist, 2 is not a dir, 3 already watched,
 * 4 is sub dir of other root
 */
uint FileIndex::addDir(const QString &dirPath)
{
    qDebug() << "Add path" << dirPath;

    QFileInfo fileInfo(dirPath);

    // Check existance && type
    if (!fileInfo.exists()) return 1;
    if(!fileInfo.isDir()) return 2;

    // Get an absolute file path
    QString absPath = fileInfo.absoluteFilePath();

    // Check if there is an identical existing path
    if (_rootDirs.contains(absPath))
        return 3;

    // Check if this dir is a subdir of an existing dir
    for (const QString &p: _rootDirs)
        if (absPath.startsWith(p + '/'))
            return 4;

    // Check if this dir is a superdir of an existing dir
    for (QStringList::iterator it = _rootDirs.begin(); it != _rootDirs.end();)
        (it->startsWith(absPath + '/')) ? it = _rootDirs.erase(it) : ++it;

    // Add the path. This is the only add on rootDirs. And existance has been
    // checked before so neo reason for a set
    _rootDirs << absPath;

    // Exception: Add the root dir to index if allowed
//    if (_indexDirs)
//        _index.push_back(std::make_shared<FileInfo>(this, canonicalRootDirPath,
//                                                     _mimeDatabase.mimeTypeForFile(rootDirFileInfo)));

    // Add this directory tree to the index
//    QMetaObject::invokeMethod(&_fileIndexer, "indexDir",
//                              Q_ARG(QString, absPath),
//                              Q_ARG(bool, true),
//                              Q_ARG(bool, true));
//    indexDir(absPath);

    // Inform observers
    emit rootDirAdded(dirPath);

    return 0;
}


/** ***************************************************************************/
void FileIndex::removeDir(const QString &dirPath)
{
    qDebug() << "Remove path" << dirPath;

    // Get an absolute file path
    QString absPath = QFileInfo(dirPath).absoluteFilePath();

    // Check existance
    if (!_rootDirs.contains(absPath))
        return;

    // Remove the path.
    _rootDirs.removeAll(absPath);

    // Remove apps and watches in this directory tree
//    cleanDir(absPath); // TODO ADD REMOVE DIR FUNC
//    QMetaObject::invokeMethod(&_fileIndexer, "cleanDir", Q_ARG(QString, absPath));

    // Update the widget, if it is visible atm
    emit rootDirRemoved(dirPath);
}


/** ***************************************************************************/
void FileIndex::restorePaths()
{
    qDebug() << "Restore paths to default";

    // Reset paths
    _rootDirs.clear();

    // Reset database
    QSqlQuery query;
    _mutex.lock();
    if(!(query.exec("DROP TABLE files")))
        qFatal("%s: %s", SQLERR, query.lastError().text().toLocal8Bit().constData());
    _mutex.unlock();
    createTableIfNotExists();

    emit indexReset();

    //  Add standard paths
    addDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    addDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    addDir(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    addDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    addDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    addDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}


/** ***************************************************************************/
bool FileIndex::indexOption(IndexOption option) const
{
    switch (option) {
    case IndexOption::Audio: return _indexOptions.indexAudio;
    case IndexOption::Video: return _indexOptions.indexVideo;
    case IndexOption::Image: return _indexOptions.indexImage;
    case IndexOption::Docs:  return _indexOptions.indexDocs;
    case IndexOption::Dirs:  return _indexOptions.indexDirs;
    case IndexOption::Hidden:return _indexOptions.indexHidden;
    default: return true; // Unreachable
    };
}


/** ***************************************************************************/
void FileIndex::setIndexOption(IndexOption option, bool checked)
{
    switch (option) {
    case IndexOption::Audio: _indexOptions.indexAudio=checked;
    case IndexOption::Video: _indexOptions.indexVideo=checked;
    case IndexOption::Image: _indexOptions.indexImage=checked;
    case IndexOption::Docs:  _indexOptions.indexDocs=checked;
    case IndexOption::Dirs:  _indexOptions.indexDirs=checked;
    case IndexOption::Hidden:_indexOptions.indexHidden=checked;
    }

    /*
     * If b is true/false the set of elements is getting bigger/smaller. This
     * means that it is just necessary to index/clean the index. This is an
     * optimization and is only working since the new set is a real super/sub
     * set of the current.
     */
    //

    stopCurrentIndexing();
    for (const QString &s : _rootDirs)
        if (checked)
            queueJob(new Indexer(this, s));
        else
            queueJob(new Cleaner(this, s));
}

/** ***************************************************************************/
SharedFilePtr FileIndex::getFile(uint id) const
{
    QSqlQuery query;
    query.prepare("SELECT path, name, mimetype FROM files WHERE id=:id");
    query.bindValue(":id", id);

    _mutex.lock();
    if (!query.exec())
        qFatal("%s: %s", SQLERR, query.lastError().text().toLocal8Bit().constData());
    _mutex.unlock();

    if(query.next()){
        SharedFilePtr ret = std::make_shared<File>();
        ret->_path = query.value(0).toString();
        ret->_name = query.value(1).toString();
        ret->_mimetype = _mimeDB.mimeTypeForName(query.value(2).toString());
        return ret;
    }
    qFatal("%s: %s", SQLERR_RESULT, query.lastError().text().toLocal8Bit().constData());
}



/** ***************************************************************************/
void FileIndex::completeUpdate()
{
    stopCurrentIndexing();
    for (const QString &s : _rootDirs){
        Cleaner* cleaner = new Cleaner(this, s);
        connect(cleaner, &Cleaner::fileRemoved, this, &FileIndex::fileRemoved);
        queueJob(cleaner);
        Indexer* indexer = new Indexer(this, s);
        connect(indexer, &Indexer::fileAdded, this, &FileIndex::fileAdded);
        queueJob(new Indexer(this, s));
    }
}



/** ***************************************************************************/
void FileIndex::reportAllFilesOnce()
{
    QSqlQuery query;
    query.prepare("SELECT id, path, name FROM files");
    _mutex.lock();
    if (!query.exec())
        qFatal("%s: %s", SQLERR, query.lastError().text().toLocal8Bit().constData());
    _mutex.unlock();

    while(query.next())
        fileAdded(query.value(0).toInt(), QStringList() << query.value(1).toString() << query.value(2).toString());
}



/** ***************************************************************************/
bool FileIndex::checkRequirements(const QFileInfo &fi, const QString &mimeName) const
{
    bool isSubOfRoot=false;
    for (const QString& d : _rootDirs){
        if (fi.absoluteFilePath().startsWith(d)){
            isSubOfRoot=true;
            break;
        }
    }
    return (isSubOfRoot && fi.isDir() && _indexOptions.indexDirs)
            || (fi.isFile()
                && ((_indexOptions.indexAudio && mimeName.startsWith("audio"))
                    ||(_indexOptions.indexVideo && mimeName.startsWith("video"))
                    ||(_indexOptions.indexImage && mimeName.startsWith("image"))
                    ||(_indexOptions.indexDocs && mimeName.startsWith("application"))));
}



/** ***************************************************************************/
void FileIndex::stopCurrentIndexing()
{
    // This is safe since it is decoupled by the local event queue
    while (!_jobQueue.isEmpty()) {
        delete _jobQueue.takeLast();
    }
    if (!_currentJob.isNull())
        _currentJob->abort();
//    QThreadPool::globalInstance()->waitForDone();
}



/** ***************************************************************************/
void FileIndex::queueJob(Job *job)
{
    _jobQueue.append(job);
    startNextInJobQueue();
}



/** ***************************************************************************/
void FileIndex::startNextInJobQueue()
{
    if (_currentJob.isNull() && !_jobQueue.isEmpty()) {
        _currentJob = _jobQueue.takeLast();
        emit statusInfo(_currentJob->stringRep());
        connect(_currentJob, &Job::destroyed, this, &FileIndex::startNextInJobQueue, Qt::QueuedConnection);
        QThreadPool::globalInstance()->start(_currentJob);
    }
    else
        emit statusInfo("Done.");
}



/** ***************************************************************************/
void FileIndex::createTableIfNotExists()
{
    QSqlQuery query;
    // TODO DB VERSIONING
    _mutex.lock();
    if (!query.exec("CREATE TABLE IF NOT EXISTS files"
                    "("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "path VARCHAR(255),"
                    "name VARCHAR(255),"
                    "mimetype VARCHAR(255) ,"
                    "usage INTEGER DEFAULT 0 ,"
                    "UNIQUE (path, name)"
                    ");"))
            qFatal("%s: %s", SQLERR, query.lastError().text().toLocal8Bit().constData());
    _mutex.unlock();
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Job::Job(FileIndex *parent) : _parent(parent)
{
    _abort = false;
}



/** ***************************************************************************/
void Job::abort()
{
    _abort = true;
}



/** ***************************************************************************/
Indexer::Indexer(FileIndex *parent, const QString &absolutePath, bool updateRecursive, bool indexRecursive)
    : Job(parent)
{
    _absolutePath = absolutePath;
    _updateRecursive = updateRecursive;
    _indexRecursive = indexRecursive;
}



/** ***************************************************************************/
void Indexer::run()
{
    indexDir(_absolutePath, _updateRecursive, _indexRecursive);
}



/** ***************************************************************************/
QString Indexer::stringRep()
{
   return QString("Indexing %1.").arg(_absolutePath);
}



/** ****************************************************************************
 * @brief Put the contents of a directory toFileIndex::indexDir
 * ! ! ! ! ONLY PASS EXISTING AND ABSOLUTE PATHS ! ! ! !
 * @param dir The directory to index
 * @param updateRecursive Precesses indexed dirs recursively.
 * @param indexRecursive Precesses _not_ indexed dirs recursively.
 */
void Indexer::indexDir(const QString &absolutePath, bool updateRecursive, bool indexRecursive)
{
    // TODO confirurable follow symlinks

    // define an anonymous function that checks if a file exists in the database
    std::function<bool(const QString&, const QString&)> existsInDB =
            [this](const QString& path, const QString& name){
        QSqlQuery query;
        query.prepare("SELECT EXISTS(SELECT 1 FROM files WHERE path=:path AND name=:name LIMIT 1);");
        query.bindValue(":path", path);
        query.bindValue(":name", name);
        _parent->_mutex.lock();
        if (!query.exec())
            qFatal("%s: %s", _parent->SQLERR, query.lastError().text().toLocal8Bit().constData());
        _parent->_mutex.unlock();
        if(!query.next())
            qFatal("%s: %s", _parent->SQLERR_RESULT, query.lastError().text().toLocal8Bit().constData());
        return query.value(0).toInt() == 1;
    };

    // define an anonymous function that adds a file to the database
    std::function<void(const QString&, const QString&, const QString&)> addtoDB =
            [this](const QString &path, const QString &name, const QString &mimetype){
        QSqlQuery query;
        query.prepare("INSERT INTO files (path, name, mimetype) VALUES (:path, :name, :mimetype);");
        query.bindValue(":path", path);
        query.bindValue(":name", name);
        query.bindValue(":mimetype", mimetype);
        _parent->_mutex.lock();
        if (!query.exec())
            qFatal("%s: %s", _parent->SQLERR, query.lastError().text().toLocal8Bit().constData());
        _parent->_mutex.unlock();
        qDebug() << name;
        emit fileAdded(query.lastInsertId().toInt(), QStringList() << name << path);
    };

    // Update all file entries
    QDir::Filters filters = QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot;
    if (_parent->_indexOptions.indexHidden) filters |= QDir::Hidden;
    QDirIterator fit(absolutePath, filters);
    while (fit.hasNext() && !_abort) {
        fit.next();
        QFileInfo &&fi = fit.fileInfo();
        QString path = fi.absolutePath();
        QString name = fi.fileName();
        QString mimetype = _mimeDB.mimeTypeForFile(fi).name();

        // Check for configurations
        if (fi.isDir() && _parent->_indexOptions.indexDirs) {
            // If the file is not in the index, index it
            if (!existsInDB(path, name)){
                addtoDB(path, name, mimetype);
                if (indexRecursive)
                    indexDir(fi.absoluteFilePath(), updateRecursive, indexRecursive);
            } else if (updateRecursive)
                indexDir(fi.absoluteFilePath(), updateRecursive, indexRecursive);
        }
        else if (fi.isFile() && _parent->checkRequirements(fi, mimetype)){
            // If the file is not in the index, index it
            if (!existsInDB(path, name)){
                addtoDB(path, name, mimetype);
            }
        }
     }
}



/** ***************************************************************************/
Cleaner::Cleaner(FileIndex *parent, const QString &absolutePath, bool recursive)
    : Job(parent)
{
    _absolutePath = absolutePath;
    _recursive = recursive;
}



/** ***************************************************************************/
void Cleaner::run()
{
    cleanDir(_absolutePath, _recursive);
}



/** ***************************************************************************/
QString Cleaner::stringRep()
{
   return QString("Cleaning %1.").arg(_absolutePath);
}



/** ***************************************************************************/
void Cleaner::cleanDir(const QString &absolutePath, bool recursive)
{
    // Get the entries in the path and if "recursive" all subpaths too.
    QSqlQuery query;
    if (recursive){
        query.prepare("SELECT id, path, name, mimetype FROM files WHERE path=:path OR path LIKE :path1");
        query.bindValue(":path", absolutePath);
        query.bindValue(":path1", QString(absolutePath).append("/%"));
    } else {
        query.prepare("SELECT id, path, name, mimetype FROM files WHERE path=:p");
        query.bindValue(":p", absolutePath);
    }
    _parent->_mutex.lock();
    if (!query.exec())
        qFatal("%s: %s", _parent->SQLERR, query.lastError().text().toLocal8Bit().constData());
    _parent->_mutex.unlock();

    // Iterate over the results and delete the file from the database if the
    // the contraints are not hold
    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM files WHERE id=:id;");
    while(query.next() && !_abort){
        QFileInfo fi(QString("%1/%2").arg(query.value(1).toString(),query.value(2).toString()));
        if (!fi.exists() || !_parent->checkRequirements(fi, query.value(3).toString()) ) {
            uint id = query.value(0).toInt();
            deleteQuery.bindValue(":id ", id);
            _parent->_mutex.lock();
            if (!deleteQuery.exec())
                qFatal("%s: %s", _parent->SQLERR, query.lastError().text().toLocal8Bit().constData());
            _parent->_mutex.unlock();
            emit fileRemoved(id);
        }
    }

}
}















///** ***************************************************************************/
//void FileDB::incrementUsage(uint id) noexcept
//{
//    QSqlQuery query;
//    query.prepare("UPDATE files SET usage=usage+1 WHERE id=:id");
//    query.bindValue(":id", id);
//    _mutex.lock();
//    if (!query.exec())
//        qFatal("%s: %s", SQLERR, query.lastError().text().toLocal8Bit().constData());
//    _mutex.unlock();
//}















///** ***************************************************************************/
///** ***************************************************************************/
///** ***************************************************************************/
//ScanIntervallFileIndex::ScanIntervallFileIndex()
//{
//    _timer.start(SCAN_INTERVAL);
////    connect(&_timer, &QTimer::timeout, this, &ScanIntervallFileIndex::indexAll);
//    connect(&_timer, SIGNAL(timeout()), &_fileIndexer, SLOT(cleanAll()));
//    connect(&_timer, SIGNAL(timeout()), &_fileIndexer, SLOT(indexAll()));
//}

///** ***************************************************************************/
//ScanIntervallFileIndex::~ScanIntervallFileIndex()
//{
//    _timer.stop();
//}
//}











/////////////////////  WATCHER //////////////////////

//// And update the widget, if it is visible atm
//if (!_widget.isNull())
//    _widget->ui.label_info->setText(QString("%1 files.").arg(_index.size()));

//// All filesystemwatches have to be in/be sub of _paths
//for (const QString &w : _watcher.directories()){
//    for (sit = _paths.begin(); sit != _paths.end(); ++sit)
//        if (w.startsWith(*sit + '/') || w == *sit)
//            break;
//    if (sit == _paths.end())
//        if (!_watcher.removePath(w)) // No clue why this should happen
//            qCritical() <<  "Could not remove watch from:" << w;
//}

//// Watch if not watched already
//if (!_watcher.directories().contains(absPath))
//    if(!_watcher.addPath(absPath)) // No clue why this should happen
//        qCritical() << absPath <<  "could not be watched. Changes in this path will not be noticed.";

//// Update unwatched subdirectories
//filters = QDir::Dirs|QDir::NoDotAndDotDot;
//if (_indexHidden) filters |= QDir::Hidden;
//QDirIterator dit(absPath, filters);
//while (dit.hasNext()){
//    dit.next();
//    if (!_watcher.directories().contains(dit.filePath()))
//        updateDirectory(dit.filePath());
//}

//// And update the widget, if it is visible atm
//if (!_widget.isNull())
//    _widget->ui.label_info->setText(QString("%1 files.").arg(_index.size()));


//if (!_watcher.directories().isEmpty())
//    _watcher.removePaths(_watcher.directories());
//if (!_watcher.files().isEmpty())
//    _watcher.removePaths(_watcher.files());



///* Keep the applications in sync with the OS */
//_timer.setInterval(UPDATE_TIMEOUT);
//_timer.setSingleShot(true);

//// If some files changed stort poth and wait a while since often multiple
//// access takes place in shot time intervals
//connect(&_watcher, &QFileSystemWatcher::directoryChanged,
//        [&](const QString &path){
//    qDebug() << path << "changed! Starting timer.";
//    if (!_toBeUpdated.contains(path))
//        _toBeUpdated << path;
//    _timer.start();
//});

//// If the timer timed out update the files
//connect(&_timer, &QTimer::timeout,
//        [this](){
//    qDebug() << "Timeout! Updating paths " << _toBeUpdated;
//    for (const QString &s: _toBeUpdated)
//        this->updateDirectory(s);
//    _toBeUpdated.clear();
//    this->clean();
//});



/////////////////////  WORKER //////////////////////



//    QPair<QString,bool> job(dirPath,recIndexUnindexed);
//    if (!_workerCleanQueue.contains(job))
    //        _workerCleanQueue.enqueue(job);





//    _worker = new IndexWorker(this);
//    _worker->moveToThread(&workerThread);





//    QPair<QString,bool> job(dirPath,recIndexUnindexed);
//    if (!_workerIndexQueue.contains(job))
//        _workerIndexQueue.enqueue(job);

//    workerThread.quit();
//    if (!workerThread.wait(THREAD_WAIT)){
//        workerThread.terminate();
//        qCritical("Had to terminate workerThread");
//    }
//    _worker->deleteLater();
