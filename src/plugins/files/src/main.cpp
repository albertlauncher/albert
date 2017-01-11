// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <vector>
#include "configwidget.h"
#include "file.h"
#include "main.h"
#include "offlineindex.h"
#include "query.h"
#include "queryhandler.h"
using std::vector;
using std::shared_ptr;
using namespace Core;


namespace  {

const char* CFG_PATHS           = "paths";
const char* CFG_FUZZY           = "fuzzy";
const bool  DEF_FUZZY           = false;
const char* CFG_INDEX_AUDIO     = "indexaudio";
const bool  DEF_INDEX_AUDIO     = true;
const char* CFG_INDEX_VIDEO     = "indexvideo";
const bool  DEF_INDEX_VIDEO     = true;
const char* CFG_INDEX_IMAGE     = "indeximage";
const bool  DEF_INDEX_IMAGE     = true;
const char* CFG_INDEX_DOC       = "indexdocs";
const bool  DEF_INDEX_DOC       = true;
const char* CFG_INDEX_DIR       = "indexdirs";
const bool  DEF_INDEX_DIR       = true;
const char* CFG_INDEX_HIDDEN    = "indexhidden";
const bool  DEF_INDEX_HIDDEN    = false;
const char* CFG_FOLLOW_SYMLINKS = "follow_symlinks";
const bool  DEF_FOLLOW_SYMLINKS = false;
const char* CFG_SCAN_INTERVAL   = "scan_interval";
const uint  DEF_SCAN_INTERVAL   = 60;
const char* IGNOREFILE          = ".albertignore";

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Files::FilesPrivate
{
public:
    FilesPrivate(Extension *q) : q(q), abort(false), rerun(false) {}

    Extension *q;

    QPointer<ConfigWidget> widget;
    QStringList rootDirs;

    vector<shared_ptr<File>> index;
    Core::OfflineIndex offlineIndex;
    QFutureWatcher<vector<shared_ptr<File>>> futureWatcher;
    QTimer indexIntervalTimer;
    bool abort;
    bool rerun;

    // Index Properties
    bool indexAudio;
    bool indexVideo;
    bool indexImage;
    bool indexDocs;
    bool indexDirs;
    bool indexHidden;
    bool followSymlinks;

    void finishIndexing();
    void startIndexing();
    vector<shared_ptr<File>> indexFiles() const;
};



/** ***************************************************************************/
void Files::FilesPrivate::startIndexing() {

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
                     std::bind(&FilesPrivate::finishIndexing, this));

    // Restart the timer (Index update may have been started manually)
    if (indexIntervalTimer.interval() != 0)
        indexIntervalTimer.start();

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(this, &FilesPrivate::indexFiles));

    // Notification
    qDebug() << qPrintable(QString("[%1] Start indexing in background thread.").arg(q->Core::Extension::id).toUtf8().constData());
    emit q->statusInfo("Indexing files ...");
}



/** ***************************************************************************/
void Files::FilesPrivate::finishIndexing() {

    if ( rerun ) {
        rerun = false;
        startIndexing();
    }

    // In case of abortion the returned data is invalid, quit
    if ( abort ) {
        abort = false;
        return;
    }

    // Get the thread results
    index = futureWatcher.future().result();

    // Rebuild the offline index
    offlineIndex.clear();
    for (const auto &item : index)
        offlineIndex.add(item);

    // Notification
    qDebug() << qPrintable(QString("[%1] Indexing done (%2 items).").arg(q->Core::Extension::id).arg(index.size()));
    emit q->statusInfo(QString("%1 files indexed.").arg(index.size()));
}



/** ***************************************************************************/
vector<shared_ptr<Files::File>> Files::FilesPrivate::indexFiles() const {

    // Get a new index
    std::vector<shared_ptr<File>> newIndex;
    std::set<QString> indexedDirs;
    QMimeDatabase mimeDatabase;

    // Prepare the iterator properties
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    if (indexHidden)
        filters |= QDir::Hidden;

    // Anonymous function that implemnents the index recursion
    std::function<void(const QFileInfo&)> indexRecursion =
            [this, &mimeDatabase, &newIndex, &indexedDirs, &filters, &indexRecursion](const QFileInfo& fileInfo){

        if (abort) return;

        const QString canonicalPath = fileInfo.canonicalFilePath();

        if (fileInfo.isFile()) {

            // If the file matches the index options, index it
            QMimeType mimetype = mimeDatabase.mimeTypeForFile(canonicalPath);
            const QString mimeName = mimetype.name();
            if ((indexAudio && mimeName.startsWith("audio"))
                    ||(indexVideo && mimeName.startsWith("video"))
                    ||(indexImage && mimeName.startsWith("image"))
                    ||(indexDocs &&
                       (mimeName.startsWith("application") || mimeName.startsWith("text")))) {
                newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));
            }
        } else if (fileInfo.isDir()) {

            emit q->statusInfo(QString("Indexing %1.").arg(canonicalPath));

            // Skip if this dir has already been indexed
            if (indexedDirs.find(canonicalPath)!=indexedDirs.end())
                return;

            // Remember that this dir has been indexed to avoid loops
            indexedDirs.insert(canonicalPath);

            // If the dir matches the index options, index it
            if (indexDirs) {
                QMimeType mimetype = mimeDatabase.mimeTypeForFile(canonicalPath);
                newIndex.push_back(std::make_shared<File>(canonicalPath, mimetype));
            }

            // Ignore ignorefile by default
            std::vector<QRegExp> ignores;
            ignores.push_back(QRegExp(IGNOREFILE, Qt::CaseSensitive, QRegExp::Wildcard));

            // Read the ignore file, see http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
            QFile file(QDir(canonicalPath).filePath(IGNOREFILE));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd())
                    ignores.push_back(QRegExp(in.readLine().trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
                file.close();
            }

            // Index all children in the dir
            QDirIterator dirIterator(canonicalPath, filters, QDirIterator::NoIteratorFlags);
            while (dirIterator.hasNext()) {
                dirIterator.next();
                const QString & fileName = dirIterator.fileName();
                const QFileInfo & fileInfo = dirIterator.fileInfo();

                // Skip if this file matches one of the ignore patterns
                for (const QRegExp& ignore : ignores)
                    if(ignore.exactMatch(fileName))
                        goto SKIP_THIS;

                // Skip if this file is a symlink and we shoud skip symlinks
                if (fileInfo.isSymLink() && !followSymlinks)
                    goto SKIP_THIS;

                // Index this file
                indexRecursion(fileInfo);
                SKIP_THIS:;
            }
        }
    };

    // Start the indexing
    for (const QString &rootDir : rootDirs) {
        indexRecursion(QFileInfo(rootDir));
        if (abort) return vector<shared_ptr<Files::File>>();
    }

    // Serialize data
    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("%1.txt").arg(q->Core::Extension::id)));
    if (file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        qDebug() << qPrintable(QString("[%1] Serializing to %2").arg(q->Core::Extension::id, file.fileName()));
        QTextStream out(&file);
        for (const shared_ptr<File> &item : newIndex)
            out << item->path() << endl << item->mimetype().name() << endl;
    } else
        qWarning() << qPrintable(QString("[%1] Could not write file %2: %3").arg(q->Core::Extension::id, file.fileName(), file.errorString()));

    return newIndex;
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Files::Extension::Extension()
    : Core::Extension("org.albert.extension.files"),
      Core::QueryHandler(Core::Extension::id),
      d(new FilesPrivate(this)) {

    // Load settings
    QSettings s(qApp->applicationName());
    s.beginGroup(Core::Extension::id);
    d->indexAudio = s.value(CFG_INDEX_AUDIO, DEF_INDEX_AUDIO).toBool();
    d->indexVideo = s.value(CFG_INDEX_VIDEO, DEF_INDEX_VIDEO).toBool();
    d->indexImage = s.value(CFG_INDEX_IMAGE, DEF_INDEX_IMAGE).toBool();
    d->indexDocs =  s.value(CFG_INDEX_DOC, DEF_INDEX_DOC).toBool();
    d->indexDirs =  s.value(CFG_INDEX_DIR, DEF_INDEX_DIR).toBool();
    d->indexHidden = s.value(CFG_INDEX_HIDDEN, DEF_INDEX_HIDDEN).toBool();
    d->followSymlinks = s.value(CFG_FOLLOW_SYMLINKS, DEF_FOLLOW_SYMLINKS).toBool();
    d->offlineIndex.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->indexIntervalTimer.setInterval(s.value(CFG_SCAN_INTERVAL, DEF_SCAN_INTERVAL).toInt()*60000); // Will be started in the initial index update
    d->rootDirs = s.value(CFG_PATHS).toStringList();
    if (d->rootDirs.isEmpty())
        restorePaths();
    s.endGroup();

    // Deserialize data
    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("%1.txt").arg(Core::Extension::id)));
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug() << qPrintable(QString("[%1] Deserializing from %2").arg(Core::Extension::id, file.fileName()));
            QTextStream in(&file);
            QMimeDatabase mimedatabase;
            while (!in.atEnd())
                d->index.emplace_back(new File(in.readLine(), mimedatabase.mimeTypeForName(in.readLine())));
            file.close();

            // Build the offline index
            for (const auto &item : d->index)
                d->offlineIndex.add(item);
        } else
            qWarning() << qPrintable(QString("[%1] Could not read from %2: %3").arg(Core::Extension::id, file.fileName(), file.errorString()));
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
    delete d;
}



/** ***************************************************************************/
QWidget *Files::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Paths
        d->widget->ui.listWidget_paths->addItems(d->rootDirs);
        connect(this, &Extension::rootDirsChanged, d->widget->ui.listWidget_paths, &QListWidget::clear);
        connect(this, &Extension::rootDirsChanged, d->widget->ui.listWidget_paths, &QListWidget::addItems);
        connect(d->widget.data(), &ConfigWidget::requestAddPath, this, &Extension::addDir);
        connect(d->widget.data(), &ConfigWidget::requestRemovePath, this, &Extension::removeDir);
        connect(d->widget->ui.pushButton_restore, &QPushButton::clicked, this, &Extension::restorePaths);
        connect(d->widget->ui.pushButton_update, &QPushButton::clicked, this, &Extension::updateIndex, Qt::QueuedConnection);

        // Checkboxes
        d->widget->ui.checkBox_audio->setChecked(indexAudio());
        connect(d->widget->ui.checkBox_audio, &QCheckBox::toggled, this, &Extension::setIndexAudio);

        d->widget->ui.checkBox_video->setChecked(indexVideo());
        connect(d->widget->ui.checkBox_video, &QCheckBox::toggled, this, &Extension::setIndexVideo);

        d->widget->ui.checkBox_image->setChecked(indexImage());
        connect(d->widget->ui.checkBox_image, &QCheckBox::toggled, this, &Extension::setIndexImage);

        d->widget->ui.checkBox_docs->setChecked(indexDocs());
        connect(d->widget->ui.checkBox_docs, &QCheckBox::toggled, this, &Extension::setIndexDocs);

        d->widget->ui.checkBox_dirs->setChecked(indexDirs());
        connect(d->widget->ui.checkBox_dirs, &QCheckBox::toggled, this, &Extension::setIndexDirs);

        d->widget->ui.checkBox_hidden->setChecked(indexHidden());
        connect(d->widget->ui.checkBox_hidden, &QCheckBox::toggled, this, &Extension::setIndexHidden);

        d->widget->ui.checkBox_followSymlinks->setChecked(followSymlinks());
        connect(d->widget->ui.checkBox_followSymlinks, &QCheckBox::toggled, this, &Extension::setFollowSymlinks);

        d->widget->ui.checkBox_fuzzy->setChecked(fuzzy());
        connect(d->widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        d->widget->ui.spinBox_interval->setValue(scanInterval());
        connect(d->widget->ui.spinBox_interval, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &Extension::setScanInterval);

        // Info
        d->widget->ui.label_info->setText(QString("%1 files indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_info, &QLabel::setText);

    }
    return d->widget;
}



/** ***************************************************************************/
void Files::Extension::handleQuery(Core::Query * query) {

    // Skip  short terms since they pollute the output
    if ( query->searchTerm().size() < 3)
        return;

    // Search for matches
    const vector<shared_ptr<Core::Indexable>> &indexables = d->offlineIndex.search(query->searchTerm().toLower());

    // Add results to query
    vector<pair<shared_ptr<Core::Item>,short>> results;
    for (const shared_ptr<Core::Indexable> &item : indexables)
        // TODO `Search` has to determine the relevance. Set to 0 for now
        results.emplace_back(std::static_pointer_cast<File>(item), -1);

    query->addMatches(results.begin(), results.end());
}



/** ***************************************************************************/
void Files::Extension::addDir(const QString &dirPath) {
    QFileInfo fileInfo(dirPath);

    // Get an absolute file path
    QString absPath = fileInfo.absoluteFilePath();

    // Check existance
    if (!fileInfo.exists()) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " does not exist.").exec();
        return;
    }

    // Check type
    if(!fileInfo.isDir()) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " is not a directory.").exec();
        return;
    }

    // Check if there is an identical existing path
    if (d->rootDirs.contains(absPath)) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " has already been indexed.").exec();
        return;
    }

    /* Check if this dir is a sub/superdir of an existing dir. This is fine
       since user may have choosen to ignore some dirs (.albertignore). This is
       more complex but also more flexible. At least inform the user */
    for (const QString &p: d->rootDirs)
        if (absPath.startsWith(p + '/'))
            QMessageBox(QMessageBox::Warning, "Warning", absPath + " is subdirectory of " + p).exec();
    for (const QString &p: d->rootDirs)
        if (p.startsWith(absPath + '/'))
            QMessageBox(QMessageBox::Warning, "Warning", p + " is subdirectory of " + absPath).exec();

    // Add the path to root dirs
    d->rootDirs << absPath;

    // Inform observers
    emit rootDirsChanged(d->rootDirs);
}



/** ***************************************************************************/
void Files::Extension::removeDir(const QString &dirPath) {
    // Get an absolute file path
    QString absPath = QFileInfo(dirPath).absoluteFilePath();

    // Check existance
    if (!d->rootDirs.contains(absPath))
        return;

    // Remove the path
    d->rootDirs.removeAll(absPath);

    // Update the widget, if it is visible atm
    emit rootDirsChanged(d->rootDirs);
}



/** ***************************************************************************/
void Files::Extension::restorePaths() {
    // Add standard paths
    d->rootDirs.clear();
    addDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}



/** ***************************************************************************/
void Files::Extension::updateIndex() {
    d->startIndexing();
}



/** ***************************************************************************/
bool Files::Extension::indexAudio() {
    return d->indexAudio;
}



/** ***************************************************************************/
void Files::Extension::setIndexAudio(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_AUDIO), b);
    d->indexAudio = b;
}



/** ***************************************************************************/
bool Files::Extension::indexVideo() {
    return d->indexVideo;
}



/** ***************************************************************************/
void Files::Extension::setIndexVideo(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_VIDEO), b);
    d->indexVideo = b;
}



/** ***************************************************************************/
bool Files::Extension::indexImage() {
    return d->indexImage;
}



/** ***************************************************************************/
void Files::Extension::setIndexImage(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_IMAGE), b);
    d->indexImage = b;
}



/** ***************************************************************************/
bool Files::Extension::indexDocs() {
    return d->indexDocs;
}



/** ***************************************************************************/
void Files::Extension::setIndexDocs(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_DOC), b);
    d->indexDocs = b;
}



/** ***************************************************************************/
bool Files::Extension::indexDirs() {
    return d->indexDirs;
}



/** ***************************************************************************/
void Files::Extension::setIndexDirs(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_DIR), b);
    d->indexDirs = b;
}



/** ***************************************************************************/
bool Files::Extension::indexHidden() {
    return d->indexHidden;
}



/** ***************************************************************************/
void Files::Extension::setIndexHidden(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_INDEX_HIDDEN), b);
    d->indexHidden = b;
}



/** ***************************************************************************/
bool Files::Extension::followSymlinks() {
    return d->followSymlinks;
}



/** ***************************************************************************/
void Files::Extension::setFollowSymlinks(bool b)  {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FOLLOW_SYMLINKS), b);
    d->followSymlinks = b;
}



/** ***************************************************************************/
unsigned int Files::Extension::scanInterval() {
    return d->indexIntervalTimer.interval()/60000;
}



/** ***************************************************************************/
void Files::Extension::setScanInterval(uint minutes) {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_SCAN_INTERVAL), minutes);
    (minutes == 0) ? d->indexIntervalTimer.stop() : d->indexIntervalTimer.start(minutes*60000);
}



/** ***************************************************************************/
bool Files::Extension::fuzzy() {
    return d->offlineIndex.fuzzy();
}



/** ***************************************************************************/
void Files::Extension::setFuzzy(bool b) {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FUZZY), b);
}
