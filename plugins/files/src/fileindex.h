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
#include <QtSql>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QMimeDatabase>
#include <QRunnable>
#include "file.h"


namespace Files{
/** ***************************************************************************/
class Job : public QObject, public QRunnable {
    Q_OBJECT
public:
    Job(FileIndex *parent);
    virtual ~Job(){}
    void abort();
    virtual QString stringRep() = 0;
protected:
    FileIndex *_parent;
    bool _abort;
};



/** ***************************************************************************/
class Indexer final : public Job{
    Q_OBJECT
public:
    Indexer(FileIndex *parent, const QString &absolutePath, bool updateRecursive=true, bool indexRecursive = true);
    void run() override;
    QString stringRep() override;
private:
    void indexDir(const QString &absolutePath, bool updateRecursive, bool indexRecursive);
    QString _absolutePath;
    bool _updateRecursive;
    bool _indexRecursive;
    QMimeDatabase _mimeDB;
signals:
    void fileAdded(uint, QStringList);
};



/** ***************************************************************************/
class Cleaner final: public Job {
    Q_OBJECT
public:
    Cleaner(FileIndex *parent, const QString &absolutePath, bool recursive = true);
    void run() override;
    QString stringRep() override;
private:
    void cleanDir(const QString &absolutePath, bool recursive);
    QString _absolutePath;
    bool _recursive;
signals:
    void fileRemoved(uint);
};



/** ***************************************************************************/
class FileIndex : public QObject{
    Q_OBJECT
    friend class Indexer;
    friend class Cleaner;
public:
    struct IndexOptions {
        bool indexAudio;
        bool indexVideo;
        bool indexImage;
        bool indexDocs;
        bool indexDirs;
        bool indexHidden;
//        bool indexFollowInside;
//        bool indexFollowOutside;
    };
    enum class IndexOption {Audio, Video, Image, Docs, Dirs, Hidden};

    FileIndex();
    ~FileIndex();

    uint addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();

    bool indexOption(IndexOption) const;
    void setIndexOption(IndexOption, bool = true);

    SharedFilePtr getFile(uint id) const;
    const QStringList &rootDirs() const { return _rootDirs; }

    void completeUpdate();
    void reportAllFilesOnce();

private:
    bool checkRequirements(const QFileInfo &f, const QString &mimeName) const;
    void stopCurrentIndexing();
    void queueJob(Job *job);
    void startNextInJobQueue();

    QMimeDatabase _mimeDB;

    // Indexing related
    QStringList _rootDirs;
    QStringList _indexedDirs; // QTBUG
    IndexOptions _indexOptions;
    QQueue<Job*> _jobQueue;

    // Threading related
    QThread _indexerThread;
    mutable QMutex _mutex;
    QPointer<Job> _currentJob;

    // SQLITE related, maybe export into seperate class
    QSqlDatabase _db;
    void createTableIfNotExists();

    static const constexpr char *SQLERR = "Executing query failed";
    static const constexpr char *SQLERR_RESULT = "Result is for some reason not as expected";

    static constexpr const char* CFG_PATHS            = "Files/paths";
    static constexpr const char* CFG_INDEX_AUDIO      = "Files/index_audio";
    static constexpr const bool  CFG_INDEX_AUDIO_DEF  = false;
    static constexpr const char* CFG_INDEX_VIDEO      = "Files/index_video";
    static constexpr const bool  CFG_INDEX_VIDEO_DEF  = false;
    static constexpr const char* CFG_INDEX_IMAGE      = "Files/index_image";
    static constexpr const bool  CFG_INDEX_IMAGE_DEF  = false;
    static constexpr const char* CFG_INDEX_DOC        = "Files/index_docs";
    static constexpr const bool  CFG_INDEX_DOC_DEF    = false;
    static constexpr const char* CFG_INDEX_DIR        = "Files/index_dirs";
    static constexpr const bool  CFG_INDEX_DIR_DEF    = false;
    static constexpr const char* CFG_INDEX_HIDDEN     = "Files/index_hidden";
    static constexpr const bool  CFG_INDEX_HIDDEN_DEF = false;


signals:
    void rootDirAdded(const QString&);
    void rootDirRemoved(const QString&);
    void indexReset();
    void fileAdded(uint, QStringList);
    void fileRemoved(uint);
    void statusInfo(const QString&);
};
}















///** ***************************************************************************/
//class ScanIntervallFileIndex final : public FileIndex
//{
//    Q_OBJECT
//public:
//    ScanIntervallFileIndex();
//    ~ScanIntervallFileIndex();
//    QTimer _timer;

//private:
//    static const constexpr unsigned int SCAN_INTERVAL = 3600000; // One hour
//};

///** ***************************************************************************/
//class WatchedFileIndex final : public FileIndex
//{
// Note QTBUG-45673
//public:
//    virtual ~WatchedFileIndex();

//private:
//    QFileSystemWatcher _watcher;
//    QTimer             _timer;
//    QStringList        _toBeUpdated;
//};
//}

//    friend class IndexWorker;
//    void setWatchFilesystem(bool b = true){};
//    QFileSystemWatcher _watcher;
//    QMimeDatabase      _mimeDatabase;
//    QThread workerThread;
//    QPointer
