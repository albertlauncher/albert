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



/** ***************************************************************************/
ChromeBookmarks::Extension::Extension()
    : IExtension("org.albert.extension.chromebookmarks",
                 tr("Chrome Bookmarks"),
                 tr("Access your Google Chrome/Chromium bookmarks via albert")) {
    qDebug() << "Initialize extension:" << id;

    // Load settings
    QSettings s;
    s.beginGroup(id);

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.exists())
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

    // Load and set a valid path (Updates the bookmarks)
    QVariant v = s.value(CFG_BOOKMARKS);
    if (v.isValid() && v.canConvert(QMetaType::QString) && QFileInfo(v.toString()).exists())
        setPath(v.toString());
    else
        restorePath();

    // Keep in sync with the bookmarkfile
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, &Extension::updateIndex);

    // Get a generic favicon
    Bookmark::icon_ = QIcon::fromTheme("favorites", QIcon(":favicon"));

    qDebug() << "Initialization done:" << id;
}



/** ***************************************************************************/
ChromeBookmarks::Extension::~Extension() {
    qDebug() << "Finalize extension:" << id;

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
    s.beginGroup(id);
    s.setValue(CFG_BOOKMARKS, bookmarksFile_);

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[ChromeBookmarks] Serializing to" << dataFile.fileName();
        QDataStream out( &dataFile );

        // ▼ CRITICAL: Serialize ▼
        QMutexLocker locker(&indexAccess_);
        out << static_cast<quint64>(index_.size());
        for (shared_ptr<AlbertItem> b : index_)
            out << std::static_pointer_cast<Bookmark>(b)->name_
                << std::static_pointer_cast<Bookmark>(b)->url_
                << std::static_pointer_cast<Bookmark>(b)->usage_;
        // ▲ CRITICAL: Serialize ▲

        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "Finalization done:" << id;
}



/** ***************************************************************************/
QWidget *ChromeBookmarks::Extension::widget(QWidget *parent) {
    if (widget_.isNull()){
        widget_ = new ConfigWidget(parent);

        // Paths
        widget_->ui.lineEdit_path->setText(bookmarksFile_);
        connect(widget_.data(), &ConfigWidget::requestEditPath, this, &Extension::setPath);
        connect(this, &Extension::pathChanged, widget_->ui.lineEdit_path, &QLineEdit::setText);

        // Info
        widget_->ui.label_info->setText(QString("%1 bookmarks indexed.").arg(index_.size()));
        connect(this, &Extension::statusInfo, widget_->ui.label_info, &QLabel::setText);
    }
    return widget_;
}

vector<shared_ptr<AlbertItem> > ChromeBookmarks::Extension::staticItems() const {
    // ▼ CRITICAL: Copy index ▼
    QMutexLocker locker(&indexAccess_);
    return index_;
    // ▲ CRITICAL: Copy index ▲
}



/** ***************************************************************************/
const QString &ChromeBookmarks::Extension::path() {
    return bookmarksFile_;
}



/** ***************************************************************************/
void ChromeBookmarks::Extension::setPath(const QString &s) {
    QFileInfo fi(s);
    // Only let _existing_ _files_ in
    if (!(fi.exists() && fi.isFile()))
        return;

    if(!watcher_.addPath(s)) // No clue why this should happen
        qCritical() << s <<  "could not be watched. Changes in this path will not be noticed.";

    bookmarksFile_ = s;
    updateIndex();

    // And update the widget, if it is visible atm
    emit pathChanged(s);
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
        widget_->ui.label_info->setText("Waiting for indexer to shut down ...");
        connect(indexer_.data(), &Indexer::destroyed, this, &Extension::updateIndex);
    } else {
        // Create a new scanning runnable for the threadpool
        indexer_ = new Indexer(this);

        //  Run it
        QThreadPool::globalInstance()->start(indexer_);
    }
}
