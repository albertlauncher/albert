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
#include "configwidget.h"
#include "extension.h"
#include "indexer.h"
#include "bookmark.h"
#include "query.h"


const QString ChromeBookmarks::Extension::EXT_NAME       = "chromebookmarks";
const QString ChromeBookmarks::Extension::CFG_BOOKMARKS  = "bookmarkfile";
const QString ChromeBookmarks::Extension::CFG_FUZZY      = "fuzzy";
const bool    ChromeBookmarks::Extension::DEF_FUZZY      = false;


/** ***************************************************************************/
ChromeBookmarks::Extension::Extension() {
    qDebug() << "[ChromeBookmarks] Initialize extension";

    // Load settings
    QSettings s;
    s.beginGroup(EXT_NAME);
    searchIndex_.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());

    // Load and set a valid path
    QVariant v = s.value(CFG_BOOKMARKS);
    if (v.isValid() && v.canConvert(QMetaType::QString) && QFileInfo(v.toString()).exists())
        setPath(v.toString());
    else
        restorePath();

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.exists()) {
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug() << "[ChromeBookmarks] Deserializing from" << dataFile.fileName();
            QDataStream in(&dataFile);
            quint64 size;
            QString name, url;
            short usage;
            in >> size;
            for (quint64 i = 0; i < size; ++i) {
                in >> name >> url >> usage;
                index_.push_back(std::make_shared<Bookmark>(name, url , usage));
            }
            dataFile.close();
        } else
            qWarning() << "Could not open file: " << dataFile.fileName();
    }

    // Rebuild the offline search index
    searchIndex_.clear();
    for (auto &i : index_)
        searchIndex_.add(i);

    // Keep in sync with the bookmarkfile
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, &Extension::updateIndex);
    connect(this, &Extension::pathChanged, this, &Extension::updateIndex);

    // Trigger an initial update
    updateIndex();

    qDebug() << "[ChromeBookmarks] Extension initialized";
}



/** ***************************************************************************/
ChromeBookmarks::Extension::~Extension() {
    qDebug() << "[ChromeBookmarks] Finalize extension";

    // Stop and wait for background indexer
    if (!indexer_.isNull()) {
        indexer_->abort();
        disconnect(indexer_.data(), &Indexer::destroyed, this, &Extension::updateIndex);
        QEventLoop loop;
        connect(indexer_.data(), &Indexer::destroyed, &loop, &QEventLoop::quit);
        loop.exec();
    }

    // Save settings
    QSettings s;
    s.beginGroup(EXT_NAME);
    s.setValue(CFG_FUZZY, searchIndex_.fuzzy());
    s.setValue(CFG_BOOKMARKS, bookmarksFile_);

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[ChromeBookmarks] Serializing to" << dataFile.fileName();
        QDataStream out( &dataFile );

        // Lock index against indexer
        QMutexLocker locker(&indexAccess_);

        // Serialize
        out << static_cast<quint64>(index_.size());
        for (shared_ptr<Bookmark> b : index_)
            out << b->name_ << b->url_ << b->usage_;

        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "[ChromeBookmarks] Extension finalized";
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
        connect(indexer_.data(), &Indexer::destroyed, this, &Extension::updateIndex);
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

