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

#include <QStandardPaths>
#include <QDirIterator>
#include <QThreadPool>
#include <QFileInfo>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>
#include "extension.h"
#include "configwidget.h"
#include "indexer.h"
#include "bookmark.h"
#include "query.h"

const char* ChromeBookmarks::Extension::CFG_BOOKMARKS  = "bookmarkfile";
const char* ChromeBookmarks::Extension::CFG_FUZZY      = "fuzzy";
const bool  ChromeBookmarks::Extension::DEF_FUZZY      = false;

/** ***************************************************************************/
ChromeBookmarks::Extension::Extension() : IExtension("Chromebookmarks") {
    qDebug("[%s] Initialize extension", name_);

    // Load settings
    QSettings s;
    s.beginGroup(name_);
    searchIndex_.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());

    // Load and set a valid path
    QVariant v = s.value(CFG_BOOKMARKS);
    if (v.isValid() && v.canConvert(QMetaType::QString) && QFileInfo(v.toString()).exists())
        setPath(v.toString());
    else
        restorePath();

    // Deserialize data
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("%1.dat").arg(name_)));
    if (dataFile.exists()) {
        if (dataFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
            qDebug("[%s] Deserializing from %s", name_, dataFile.fileName().toLocal8Bit().data());
            QDataStream in(&dataFile);
            quint64 count;
            for (in >> count ;count != 0; --count){
                shared_ptr<Bookmark> deshrp = std::make_shared<Bookmark>();
                deshrp->deserialize(in);
                index_.push_back(deshrp);
            }
            dataFile.close();

            // Build the offline index
            for (auto &item : index_)
                searchIndex_.add(item);
        } else
            qWarning() << "Could not open file: " << dataFile.fileName();
    }

    // Keep in sync with the bookmarkfile
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, &Extension::updateIndex, Qt::QueuedConnection);
    connect(this, &Extension::pathChanged, this, &Extension::updateIndex, Qt::QueuedConnection);

    // Trigger an initial update
    updateIndex();

    qDebug("[%s] Extension initialized", name_);
}



/** ***************************************************************************/
ChromeBookmarks::Extension::~Extension() {
    qDebug("[%s] Finalize extension", name_);

    /*
     * Stop and wait for background indexer.
     * This should be thread safe since this thread is responisble to start the
     * indexer and, connections to this thread are disconnected in the QObject
     * destructor and all events for a deleted object are removed from the event
     * queue.
     */
    if (!indexer_.isNull()) {
        indexer_->abort();
        QEventLoop loop;
        connect(indexer_.data(), &Indexer::destroyed, &loop, &QEventLoop::quit);
        loop.exec();
    }

    // Save settings
    QSettings s;
    s.beginGroup(name_);
    s.setValue(CFG_FUZZY, searchIndex_.fuzzy());
    s.setValue(CFG_BOOKMARKS, bookmarksFile_);

    // Serialize data
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("%1.dat").arg(name_)));
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug("[%s] Serializing to %s", name_, dataFile.fileName().toLocal8Bit().data());
        QDataStream out( &dataFile );
        out << static_cast<quint64>(index_.size());
        for (auto &item : index_)
            item->serialize(out);
        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug("[%s] Extension finalized", name_);
}



/** ***************************************************************************/
QWidget *ChromeBookmarks::Extension::widget(QWidget *parent) {
    if (widget_.isNull()){
        widget_ = new ConfigWidget(parent);

        // Paths
        widget_->ui.lineEdit_path->setText(bookmarksFile_);
        connect(widget_.data(), &ConfigWidget::requestEditPath, this, &Extension::setPath);
        connect(this, &Extension::pathChanged, widget_->ui.lineEdit_path, &QLineEdit::setText);

        // Fuzzy
        widget_->ui.checkBox_fuzzy->setChecked(fuzzy());
        connect(widget_->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Info
        widget_->ui.label_info->setText(QString("%1 bookmarks indexed.").arg(index_.size()));
        connect(this, &Extension::statusInfo, widget_->ui.label_info, &QLabel::setText);

        // If indexer is active connect its statusInfo to the infoLabel
        if (!indexer_.isNull())
            connect(indexer_.data(), &Indexer::statusInfo, widget_->ui.label_info, &QLabel::setText);
    }
    return widget_;
}



/** ***************************************************************************/
void ChromeBookmarks::Extension::handleQuery(shared_ptr<Query> query) {
    // Search for matches. Lock memory against indexer
    indexAccess_.lock();
    vector<shared_ptr<IIndexable>> indexables = searchIndex_.search(query->searchTerm());
    indexAccess_.unlock();

    // Add results to query. This cast is safe since index holds files only
    for (shared_ptr<IIndexable> obj : indexables)
        // TODO `Search` has to determine the relevance. Set to 0 for now
        query->addMatch(std::static_pointer_cast<Bookmark>(obj), 0);
}



/** ***************************************************************************/
const QString &ChromeBookmarks::Extension::path() {
    return bookmarksFile_;
}



/** ***************************************************************************/
void ChromeBookmarks::Extension::setPath(const QString &path) {
    QFileInfo fi(path);

    if (!(fi.exists() && fi.isFile()))
        return;

    bookmarksFile_ = path;

    emit pathChanged(path);
}



/** ***************************************************************************/
void ChromeBookmarks::Extension::restorePath() {
    // Find a bookmark file (Take first one)
    for (QString browser : {"chromium","google-chrome"}){
        QString root = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath(browser);
        QDirIterator it(root, {"Bookmarks"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            setPath(it.next());
            return;
        }
    }
}



/** ***************************************************************************/
void ChromeBookmarks::Extension::updateIndex() {
    qDebug() << "[ChromeBookmarks] Index update triggered";

    // If thread is running, stop it and start this functoin after termination
    if (!indexer_.isNull()) {
        indexer_->abort();
        if (!widget_.isNull())
            widget_->ui.label_info->setText("Waiting for indexer to shut down ...");
        connect(indexer_.data(), &Indexer::destroyed, this, &Extension::updateIndex, Qt::QueuedConnection);
    } else {
        // Create a new scanning runnable for the threadpool
        indexer_ = new Indexer(this);

        //  Run it
        QThreadPool::globalInstance()->start(indexer_);

        // If widget is visible show the information in the status bat
        if (!widget_.isNull())
            connect(indexer_.data(), &Indexer::statusInfo, widget_->ui.label_info, &QLabel::setText);
    }
}



/** ***************************************************************************/
bool ChromeBookmarks::Extension::fuzzy() {
    return searchIndex_.fuzzy();
}



/** ***************************************************************************/
void ChromeBookmarks::Extension::setFuzzy(bool b) {
    indexAccess_.lock();
    searchIndex_.setFuzzy(b);
    indexAccess_.unlock();
}

