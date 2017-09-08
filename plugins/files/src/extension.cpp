// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
#include <QDir>
#include <QDirIterator>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QThreadPool>
#include <QTimer>
#include <memory>
#include <functional>
#include <vector>
#include <set>
#include "configwidget.h"
#include "file.h"
#include "standardfile.h"
#include "extension.h"
#include "indextreenode.h"
#include "core/query.h"
#include "util/offlineindex.h"
#include "util/standarditem.h"
#include "util/standardaction.h"
using namespace Core;
using namespace std;


namespace {

const char* CFG_PATHS           = "paths";
const char* CFG_FILTERS         = "filters";
const QStringList DEF_FILTERS   = { "inode/directory", "application/*" };
const char* CFG_FUZZY           = "fuzzy";
const bool  DEF_FUZZY           = false;
const char* CFG_INDEX_HIDDEN    = "indexhidden";
const bool  DEF_INDEX_HIDDEN    = false;
const char* CFG_FOLLOW_SYMLINKS = "follow_symlinks";
const bool  DEF_FOLLOW_SYMLINKS = false;
const char* CFG_SCAN_INTERVAL   = "scan_interval";
const uint  DEF_SCAN_INTERVAL   = 15;



class OfflineIndexBuilderVisitor : public Files::Visitor {
    Core::OfflineIndex &offlineIndex;
public:
    OfflineIndexBuilderVisitor(Core::OfflineIndex &offlineIndex)
        : offlineIndex(offlineIndex) { }

    void visit(Files::IndexTreeNode *node) override {
        for ( const shared_ptr<Files::File> &item : node->items() )
            offlineIndex.add(item);
    }
};


class CounterVisitor : public Files::Visitor {
public:
    uint itemCount = 0;
    uint dirCount = 0;
    void visit(Files::IndexTreeNode *node) override {
        ++dirCount;
        itemCount += node->items().size();
    }
};

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Files::Private
{
public:
    Private(Extension *q) : q(q), abort(false), rerun(false) {}

    Extension *q;

    QPointer<ConfigWidget> widget;

    QStringList indexRootDirs;
    IndexSettings indexSettings;
    map<QString, shared_ptr<IndexTreeNode>> indexTrees;
    unique_ptr<QFutureWatcher<Core::OfflineIndex*>> futureWatcher;
    Core::OfflineIndex offlineIndex;
    QTimer indexIntervalTimer;
    bool abort;
    bool rerun;


    void finishIndexing();
    void startIndexing();
    Core::OfflineIndex *indexFiles();
};



/** ***************************************************************************/
void Files::Private::startIndexing() {

    // Abort and rerun
    if ( futureWatcher ) {
        emit q->statusInfo("Waiting for indexer to shut down ...");
        abort = true;
        rerun = true;
        return;
    }

    // Run finishIndexing when the indexing thread finished
    futureWatcher.reset(new QFutureWatcher<Core::OfflineIndex*>);
    QObject::connect(futureWatcher.get(), &QFutureWatcher<Core::OfflineIndex*>::finished,
                     [this](){ this->finishIndexing(); });

    // Restart the timer (Index update may have been started manually)
    if (indexIntervalTimer.interval() != 0)
        indexIntervalTimer.start();

    // Run the indexer thread
    qInfo() << "Start indexing files.";
    futureWatcher->setFuture(QtConcurrent::run(this, &Private::indexFiles));

    // Notification
    emit q->statusInfo("Indexing files ...");
}



/** ***************************************************************************/
void Files::Private::finishIndexing() {

    // In case of abortion the returned data is invalid
    if ( !abort ) {
        OfflineIndex *retval = futureWatcher->future().result();
        if (retval) {
            offlineIndex = std::move(*retval);
            delete retval;
        }

        // Notification
        CounterVisitor counterVisitor;
        for (const auto & kv : indexTrees )
            kv.second->accept(counterVisitor);
        qInfo() << qPrintable(QString("Indexed %1 files in %2 directories.")
                              .arg(counterVisitor.itemCount).arg(counterVisitor.dirCount));
        emit q->statusInfo(QString("Indexed %1 files in %2 directories.")
                           .arg(counterVisitor.itemCount).arg(counterVisitor.dirCount));
    }

    futureWatcher.reset();
    abort = false;

    if ( rerun ) {
        rerun = false;
        startIndexing();
    }
}



/** ***************************************************************************/
OfflineIndex* Files::Private::indexFiles() {

    // Remove the subtrees not wanted anymore
    for (auto& kv : this->indexTrees) {
        if ( !indexRootDirs.contains(kv.first) ) {
            kv.second->removeDownlinks();
            this->indexTrees.erase(kv.first);
        }
    }

    // Start the indexing
    for ( const QString &rootDir : indexRootDirs ) {

        qDebug() << qPrintable(QString("Indexing %1…").arg(rootDir));
        emit q->statusInfo(QString("Indexing %1…").arg(rootDir));

        // If this root dir does not exist create it
        if ( !indexTrees.count(rootDir) )
            indexTrees.emplace(rootDir, make_shared<IndexTreeNode>(rootDir));

        // Update the root dir
        indexTrees[rootDir]->update(abort, indexSettings);

        if ( abort )
            return nullptr;
    }

    indexSettings.setSettingsChangedSinceLastUpdate(false);


    // Serialize data
    qDebug() << "Serializing index data…";
    emit q->statusInfo("Serializing index data…");
    //    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).
    //                   filePath(QString("%1.txt").arg(q->Core::Plugin::id())));
    //    if ( file.open(QIODevice::WriteOnly|QIODevice::Text) ) {
    //        qInfo() << qPrintable(QString("Serializing files to '%1'").arg(file.fileName()));
    //        QTextStream out(&file);
    //        for (const shared_ptr<File> &item : newIndex)
    //            out << item->path() << endl << item->mimetype().name() << endl;
    //    } else
    //        qWarning() << qPrintable(QString("Could not write to file '%1': %2").arg(file.fileName(), file.errorString()));


    // Build offline index
    qDebug() << "Building offline index…";
    emit q->statusInfo("Building offline index…");
    Core::OfflineIndex *offline = new Core::OfflineIndex;
    OfflineIndexBuilderVisitor visitor(*offline);
    for (auto& kv : this->indexTrees)
        kv.second->accept(visitor);
    return offline;
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Files::Extension::Extension()
    : Core::Extension("org.albert.extension.files"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)) {

    // Load settings
    d->indexSettings.setFilters(settings().value(CFG_FILTERS, DEF_FILTERS).toStringList());
    d->indexSettings.setIndexHidden(settings().value(CFG_INDEX_HIDDEN, DEF_INDEX_HIDDEN).toBool());
    d->indexSettings.setFollowSymlinks(settings().value(CFG_FOLLOW_SYMLINKS, DEF_FOLLOW_SYMLINKS).toBool());
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->indexIntervalTimer.setInterval(settings().value(CFG_SCAN_INTERVAL, DEF_SCAN_INTERVAL).toInt()*60000); // Will be started in the initial index update
    d->indexRootDirs = settings().value(CFG_PATHS, QDir::homePath()).toStringList();
//    if (d->indexRootDirs.isEmpty())
//        restorePaths();

//    // Deserialize data
//    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).
//                   filePath(QString("%1.txt").arg(Core::Plugin::id())));
//    if (file.exists()) {
//        if (file.open(QIODevice::ReadOnly| QIODevice::Text)) {
//            qInfo() << qPrintable(QString("Deserializing files from '%1'.").arg(file.fileName()));
//            QTextStream in(&file);
//            QMimeDatabase mimedatabase;
//            while (!in.atEnd())
//                d->index.emplace_back(new File(in.readLine(), mimedatabase.mimeTypeForName(in.readLine())));
//            file.close();

//            // Build the offline index
//            for (const auto &item : d->index)
//                d->offlineIndex.add(item);
//        } else
//            qWarning() << qPrintable(QString("Could not read from file '%1': %2").arg(file.fileName(), file.errorString()));
//    }

    // Index timer
    connect(&d->indexIntervalTimer, &QTimer::timeout, this, &Extension::updateIndex);

    // If the root dirs change write it to the settings
    connect(this, &Extension::pathsChanged, [this](const QStringList& dirs){
        settings().setValue(CFG_PATHS, dirs);
    });

    // Trigger an initial update
    updateIndex();
}



/** ***************************************************************************/
Files::Extension::~Extension() {

    // The indexer thread has sideeffects wait for termination
    d->abort = true;
    d->rerun = false;
    if ( d->futureWatcher ){
        disconnect(d->futureWatcher.get(), 0, 0, 0);
        d->futureWatcher->waitForFinished();
    }
}



/** ***************************************************************************/
QWidget *Files::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(this, parent);
    return d->widget;
}



/** ***************************************************************************/
void Files::Extension::handleQuery(Core::Query * query) {

    if ( query->searchTerm().isEmpty() )
        return;

    if ( query->searchTerm().startsWith('/') || query->searchTerm().startsWith("~") ) {

        QFileInfo queryFileInfo(query->searchTerm());

        // Substitute tilde
        if ( query->searchTerm()[0] == '~' )
            queryFileInfo.setFile(QDir::homePath()+query->searchTerm().right(query->searchTerm().size()-1));

        // Get all matching files
        QFileInfo pathInfo(queryFileInfo.path());
        if ( pathInfo.exists() && pathInfo.isDir() ) {
            QMimeDatabase mimeDatabase;
            QDir dir(pathInfo.filePath());
            for (const QFileInfo& fileinfo : dir.entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot,
                                                               QDir::DirsFirst|QDir::Name|QDir::IgnoreCase) ) {
                if ( fileinfo.fileName().startsWith(queryFileInfo.fileName()) ) {
                    QMimeType mimetype = mimeDatabase.mimeTypeForFile(fileinfo.filePath());
                    query->addMatch(make_shared<StandardFile>(fileinfo.filePath(), mimetype),
                                    static_cast<uint>(UINT_MAX * static_cast<float>(queryFileInfo.fileName().size()) / fileinfo.fileName().size()));
                }
            }
        }
    }
    else
    {
        if ( QString("albert scan files").startsWith(query->searchTerm()) ) {
            shared_ptr<StandardItem> standardItem = make_shared<StandardItem>("org.albert.extension.files.action.index");
            standardItem->setText("albert scan files");
            standardItem->setSubtext("Update the file index");
            standardItem->setIconPath(":app_icon");

            shared_ptr<StandardAction> standardAction = make_shared<StandardAction>();
            standardAction->setText("Update the file index");
            standardAction->setAction([this](){ this->updateIndex(); });

            standardItem->setActions({standardAction});

            query->addMatch(move(standardItem));
        }

        // Search for matches
        const vector<shared_ptr<IndexableItem>> &indexables =
                d->offlineIndex.search(query->searchTerm());

        // Add results to query
        vector<pair<shared_ptr<Core::Item>,uint>> results;
        for (const shared_ptr<Core::IndexableItem> &item : indexables)
            // TODO `Search` has to determine the relevance. Set to 0 for now
            results.emplace_back(static_pointer_cast<File>(item), 0);

        query->addMatches(make_move_iterator(results.begin()),
                          make_move_iterator(results.end()));
    }
}



/** ***************************************************************************/
const QStringList &Files::Extension::paths() const {
    return d->indexRootDirs;
}



/** ***************************************************************************/
void Files::Extension::setPaths(const QStringList &paths) {

    if (d->indexRootDirs == paths)
        return;

    d->indexRootDirs.clear();

    // Check sanity and add path
    for ( const QString& path : paths ) {

        QFileInfo fileInfo(path);
        QString absPath = fileInfo.absoluteFilePath();

        if (d->indexRootDirs.contains(absPath)) {
            qWarning() << QString("Duplicate paths: %1.").arg(path);
            continue;
        }

        if (!fileInfo.exists()) {
            qWarning() << QString("Path does not exist: %1.").arg(path);
            continue;
        }

        if(!fileInfo.isDir()) {
            qWarning() << QString("Path is not a directory: %1.").arg(path);
            continue;
        }

        d->indexRootDirs << absPath;
    }

    emit pathsChanged(d->indexRootDirs);

    // Store to settings
    settings().setValue(CFG_PATHS, d->indexRootDirs);

}



/** ***************************************************************************/
void Files::Extension::restorePaths() {
    // Add standard path
    d->indexRootDirs.clear();
    d->indexRootDirs << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    emit pathsChanged(d->indexRootDirs);
}



/** ***************************************************************************/
void Files::Extension::updateIndex() {
    d->startIndexing();
}



/** ***************************************************************************/
bool Files::Extension::indexHidden() const {
    return d->indexSettings.indexHidden();
}



/** ***************************************************************************/
void Files::Extension::setIndexHidden(bool b)  {
    settings().setValue(CFG_INDEX_HIDDEN, b);
    d->indexSettings.setIndexHidden(b);
}



/** ***************************************************************************/
bool Files::Extension::followSymlinks() const {
    return d->indexSettings.followSymlinks();
}



/** ***************************************************************************/
void Files::Extension::setFollowSymlinks(bool b)  {
    settings().setValue(CFG_FOLLOW_SYMLINKS, b);
    d->indexSettings.setFollowSymlinks(b);
}



/** ***************************************************************************/
unsigned int Files::Extension::scanInterval() const {
    return static_cast<uint>(d->indexIntervalTimer.interval()/60000);
}



/** ***************************************************************************/
void Files::Extension::setScanInterval(uint minutes) {
    settings().setValue(CFG_SCAN_INTERVAL, minutes);
    (minutes == 0) ? d->indexIntervalTimer.stop()
                   : d->indexIntervalTimer.start(static_cast<int>(minutes*60000));
}



/** ***************************************************************************/
bool Files::Extension::fuzzy() const {
    return d->offlineIndex.fuzzy();
}



/** ***************************************************************************/
void Files::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);
}



/** ***************************************************************************/
QStringList Files::Extension::filters() const {
    QStringList retval;
    for ( auto const & regex : d->indexSettings.filters() )
        retval.push_back(regex.pattern());
    return retval;
}



/** ***************************************************************************/
void Files::Extension::setFilters(const QStringList &filters) {
    settings().setValue(CFG_FILTERS, filters);
    d->indexSettings.setFilters(filters);
}
