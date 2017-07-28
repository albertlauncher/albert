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
#include <QRegularExpression>
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
#include "extension.h"
#include "core/query.h"
#include "util/offlineindex.h"
#include "util/standarditem.h"
#include "util/standardaction.h"
using std::pair;
using std::shared_ptr;
using std::vector;
using namespace Core;


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
const uint  DEF_SCAN_INTERVAL   = 60;
const char* IGNOREFILE          = ".albertignore";

struct IndexSettings {
    QStringList rootDirs;
    QStringList filters;
    bool indexHidden;
    bool followSymlinks;
};

enum class PatternType {
    Include,
    Exclude
};

struct IgnoreEntry {
    IgnoreEntry(QRegularExpression regex, PatternType type) : regex(regex), type(type) {}
    QRegularExpression regex;
    PatternType type;
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

    vector<shared_ptr<File>> index;
    Core::OfflineIndex offlineIndex;
    QFutureWatcher<vector<shared_ptr<File>>> futureWatcher;
    QTimer indexIntervalTimer;
    bool abort;
    bool rerun;

    IndexSettings indexSettings;

    void finishIndexing();
    void startIndexing();
    vector<shared_ptr<File>> indexFiles(const IndexSettings &indexSettings) const;
};



/** ***************************************************************************/
void Files::Private::startIndexing() {

    // Abort and rerun
    if ( futureWatcher.future().isRunning() ) {
        emit q->statusInfo("Waiting for indexer to shut down ...");
        abort = true;
        rerun = true;
        return;
    }

    // Run finishIndexing when the indexing thread finished
    futureWatcher.disconnect();
    QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<File>>>::finished,
                     std::bind(&Private::finishIndexing, this));

    // Restart the timer (Index update may have been started manually)
    if (indexIntervalTimer.interval() != 0)
        indexIntervalTimer.start();

    // Run the indexer thread
    qInfo() << "Start indexing files.";
    futureWatcher.setFuture(QtConcurrent::run(this, &Private::indexFiles, indexSettings));

    // Notification
    emit q->statusInfo("Indexing files ...");
}



/** ***************************************************************************/
void Files::Private::finishIndexing() {

    // In case of abortion the returned data is invalid
    if ( !abort ) {
        // Get the thread results
        index = futureWatcher.future().result();

        // Rebuild the offline index
        offlineIndex.clear();
        for (const auto &item : index)
            offlineIndex.add(item);

        // Notification
        qInfo() << qPrintable(QString("Indexed %1 files.").arg(index.size()));
        emit q->statusInfo(QString("%1 files indexed.").arg(index.size()));
    }

    abort = false;

    if ( rerun ) {
        rerun = false;
        startIndexing();
    }
}



/** ***************************************************************************/
vector<shared_ptr<Files::File>>
Files::Private::indexFiles(const IndexSettings &indexSettings) const {

    // Get a new index
    std::vector<shared_ptr<File>> newIndex;
    std::set<QString> indexedDirs;
    QMimeDatabase mimeDatabase;
    std::vector<QRegExp> mimeFilters;
    for (const QString &re : indexSettings.filters)
        mimeFilters.emplace_back(re, Qt::CaseInsensitive, QRegExp::Wildcard);

    // Prepare the iterator properties
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    if (indexSettings.indexHidden)
        filters |= QDir::Hidden;

    // Anonymous function that implemnents the index recursion
    std::function<void(const QFileInfo&, vector<IgnoreEntry>)> indexRecursion =
            [&](const QFileInfo& fileInfo, const vector<IgnoreEntry> &ignoreEntries){

        if (abort) return;

        const QString canonicalPath = fileInfo.canonicalFilePath();
        const QMimeType mimetype = mimeDatabase.mimeTypeForFile(canonicalPath);
        const QString mimeName = mimetype.name();

        // If the file matches the index options, index it
        if ( std::any_of(mimeFilters.begin(), mimeFilters.end(),
                         [&](const QRegExp &re){ return re.exactMatch(mimeName); }) )
            newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));

        if (fileInfo.isDir()) {

            emit q->statusInfo(QString("Indexing %1.").arg(canonicalPath));

            // Skip if this dir has already been indexed
            if ( indexedDirs.find(canonicalPath) != indexedDirs.end() )
                return;

            // Remember that this dir has been indexed to avoid loops
            indexedDirs.insert(canonicalPath);

            // Read the ignore file, see http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
            vector<IgnoreEntry> localIgnoreEntries = ignoreEntries;
            QFile file(QDir(canonicalPath).filePath(IGNOREFILE));
            if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
                QTextStream in(&file);
                while ( !in.atEnd() ) {
                    QString pattern = QDir::cleanPath(in.readLine());

                    if ( pattern.isEmpty() || pattern.startsWith("#") )
                        continue;

                    // Replace ** and * by their regex analogons
                    pattern.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)"), "[^\\/]*");
                    pattern.replace(QRegularExpression("\\*{2,}"), ".*");

                    // Determine pattern type
                    PatternType patternType = PatternType::Exclude;
                    if ( pattern.startsWith('!') ) {
                        patternType = PatternType::Include;
                        pattern = pattern.mid(1, -1);
                    }

                    // Respect files beginning with excalmation mark
                    if ( pattern.startsWith("\\!") )
                        pattern = pattern.mid(1, -1);

                    if ( pattern.startsWith("/") ) {
                        pattern = QString("^%1$").arg(QDir(fileInfo.filePath()).filePath(pattern.mid(1, -1)));
                        localIgnoreEntries.emplace_back(QRegularExpression(pattern), patternType);
                    } else {
                        pattern = QString("%1$").arg(pattern);
                        localIgnoreEntries.emplace_back(QRegularExpression(pattern), patternType);
                    }
                }
                file.close();
            }

            // Index all children in the dir
            QDirIterator dirIterator(canonicalPath, filters, QDirIterator::NoIteratorFlags);
            while ( dirIterator.hasNext() ) {
                dirIterator.next();
                const QFileInfo & fileInfo = dirIterator.fileInfo();

                // Skip if this file depending on ignore patterns
                PatternType patternType = PatternType::Include;
                for ( const IgnoreEntry &ignoreEntry : localIgnoreEntries )
                    if ( ignoreEntry.regex.match(fileInfo.filePath()).hasMatch() )
                        patternType = ignoreEntry.type;
                if ( patternType == PatternType::Exclude )
                    continue;

                // Skip if this file is a symlink and we should skip symlinks
                if ( fileInfo.isSymLink() && !indexSettings.followSymlinks )
                    continue;

                // Index this file
                indexRecursion(fileInfo, localIgnoreEntries);
            }
        }
    };

    // Start the indexing
    for ( const QString &rootDir : indexSettings.rootDirs ) {
        // Index rootdir, ignore ignorefile by default
        vector<IgnoreEntry> ignores = {IgnoreEntry(
                                       QRegularExpression(QString("%1$").arg(IGNOREFILE)),
                                       PatternType::Exclude)};
        indexRecursion(QFileInfo(rootDir), ignores);
        if ( abort )
            return vector<shared_ptr<Files::File>>();
    }

    // Serialize data
    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).
                   filePath(QString("%1.txt").arg(q->Core::Extension::id)));
    if ( file.open(QIODevice::WriteOnly|QIODevice::Text) ) {
        qInfo() << qPrintable(QString("Serializing files to '%1'").arg(file.fileName()));
        QTextStream out(&file);
        for (const shared_ptr<File> &item : newIndex)
            out << item->path() << endl << item->mimetype().name() << endl;
    } else
        qWarning() << qPrintable(QString("Could not write to file '%1': %2").arg(file.fileName(), file.errorString()));

    return newIndex;
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Files::Extension::Extension()
    : Core::Extension("org.albert.extension.files"),
      Core::QueryHandler(Core::Extension::id),
      d(new Private(this)) {

    // Load settings
    QSettings s(qApp->applicationName());
    s.beginGroup(Core::Extension::id);
    d->indexSettings.filters =  s.value(CFG_FILTERS, DEF_FILTERS).toStringList();
    d->indexSettings.indexHidden = s.value(CFG_INDEX_HIDDEN, DEF_INDEX_HIDDEN).toBool();
    d->indexSettings.followSymlinks = s.value(CFG_FOLLOW_SYMLINKS, DEF_FOLLOW_SYMLINKS).toBool();
    d->offlineIndex.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->indexIntervalTimer.setInterval(s.value(CFG_SCAN_INTERVAL, DEF_SCAN_INTERVAL).toInt()*60000); // Will be started in the initial index update
    d->indexSettings.rootDirs = s.value(CFG_PATHS).toStringList();
    if (d->indexSettings.rootDirs.isEmpty())
        restorePaths();
    s.endGroup();

    // Deserialize data
    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).
                   filePath(QString("%1.txt").arg(Core::Extension::id)));
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qInfo() << qPrintable(QString("Deserializing files from '%1'.").arg(file.fileName()));
            QTextStream in(&file);
            QMimeDatabase mimedatabase;
            while (!in.atEnd())
                d->index.emplace_back(new File(in.readLine(), mimedatabase.mimeTypeForName(in.readLine())));
            file.close();

            // Build the offline index
            for (const auto &item : d->index)
                d->offlineIndex.add(item);
        } else
            qWarning() << qPrintable(QString("Could not read from file '%1': %2").arg(file.fileName(), file.errorString()));
    }

    // Index timer
    connect(&d->indexIntervalTimer, &QTimer::timeout, this, &Extension::updateIndex);

    // If the root dirs change write it to the settings
    connect(this, &Extension::rootDirsChanged, [this](const QStringList& dirs){
        QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_PATHS), dirs);
    });

    // Trigger an initial update
    updateIndex();
}



/** ***************************************************************************/
Files::Extension::~Extension() {

    // The indexer thread has sideeffects wait for termination
    d->abort = true;
    d->rerun = false;
    d->futureWatcher.waitForFinished();
}



/** ***************************************************************************/
QWidget *Files::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(this, parent);
    return d->widget;
}



/** ***************************************************************************/
void Files::Extension::handleQuery(Core::Query * query) {


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
                    query->addMatch(std::make_shared<File>(fileinfo.filePath(), mimetype),
                                    static_cast<short>(SHRT_MAX * static_cast<float>(queryFileInfo.fileName().size()) / fileinfo.fileName().size()));
                }
            }
        }
    }
    else
    {
        if ( QString("albert scan files").startsWith(query->searchTerm()) ) {
            shared_ptr<StandardItem> standardItem = std::make_shared<StandardItem>("org.albert.extension.files.action.index");
            standardItem->setText("albert scan files");
            standardItem->setSubtext("Update the file index");
            standardItem->setIconPath(":app_icon");

            shared_ptr<StandardAction> standardAction = std::make_shared<StandardAction>();
            standardAction->setText("Update the file index");
            standardAction->setAction([this](){ this->updateIndex(); });

            standardItem->setActions({standardAction});

            query->addMatch(standardItem);
        }

        // Search for matches
        const vector<shared_ptr<Core::Indexable>> &indexables = d->offlineIndex.search(query->searchTerm().toLower());

        // Add results to query
        vector<pair<shared_ptr<Core::Item>,short>> results;
        for (const shared_ptr<Core::Indexable> &item : indexables)
            // TODO `Search` has to determine the relevance. Set to 0 for now
            results.emplace_back(std::static_pointer_cast<File>(item), -1);

        query->addMatches(results.begin(), results.end());
    }
}



/** ***************************************************************************/
const QStringList &Files::Extension::paths() const {
    return d->indexSettings.rootDirs;
}



/** ***************************************************************************/
void Files::Extension::setPaths(const QStringList &paths) {

    d->indexSettings.rootDirs.clear();

    // Check sanity and add path
    for ( const QString& path : paths ) {

        QFileInfo fileInfo(path);
        QString absPath = fileInfo.absoluteFilePath();

        if (d->indexSettings.rootDirs.contains(absPath)) {
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

        d->indexSettings.rootDirs << absPath;
    }

    // Store to settings
    QSettings(qApp->applicationName())
            .setValue(QString("%1/%2").arg(Core::Extension::id, CFG_PATHS), d->indexSettings.rootDirs);
}



/** ***************************************************************************/
void Files::Extension::restorePaths() {
    // Add standard path
    d->indexSettings.rootDirs.clear();
    d->indexSettings.rootDirs << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}



/** ***************************************************************************/
void Files::Extension::updateIndex() {
    d->startIndexing();
}



/** ***************************************************************************/
bool Files::Extension::indexHidden() const {
    return d->indexSettings.indexHidden;
}



/** ***************************************************************************/
void Files::Extension::setIndexHidden(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_HIDDEN), b);
    d->indexSettings.indexHidden = b;
}



/** ***************************************************************************/
bool Files::Extension::followSymlinks() const {
    return d->indexSettings.followSymlinks;
}



/** ***************************************************************************/
void Files::Extension::setFollowSymlinks(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FOLLOW_SYMLINKS), b);
    d->indexSettings.followSymlinks = b;
}



/** ***************************************************************************/
unsigned int Files::Extension::scanInterval() const {
    return d->indexIntervalTimer.interval()/60000;
}



/** ***************************************************************************/
void Files::Extension::setScanInterval(uint minutes) {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_SCAN_INTERVAL), minutes);
    (minutes == 0) ? d->indexIntervalTimer.stop() : d->indexIntervalTimer.start(minutes*60000);
}



/** ***************************************************************************/
bool Files::Extension::fuzzy() const {
    return d->offlineIndex.fuzzy();
}



/** ***************************************************************************/
void Files::Extension::setFuzzy(bool b) {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FUZZY), b);
}



/** ***************************************************************************/
const QStringList &Files::Extension::filters() const {
    return d->indexSettings.filters;
}



/** ***************************************************************************/
void Files::Extension::setFilters(const QStringList &filters) {
    QSettings(qApp->applicationName())
            .setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FILTERS), filters);
    d->indexSettings.filters = filters;
}
