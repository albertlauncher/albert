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
#include <QThreadPool>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <memory>
#include "extension.h"
#include "configwidget.h"
#include "desktopentry.h"
#include "indexer.h"
#include "query.h"

const char* Applications::Extension::CFG_PATHS    = "paths";
const char* Applications::Extension::CFG_FUZZY    = "fuzzy";
const bool  Applications::Extension::DEF_FUZZY    = false;
const char* Applications::Extension::CFG_TERM     = "terminal";
const char* Applications::Extension::DEF_TERM     = "xterm -e %1";
const bool  Applications::Extension::UPDATE_DELAY = 60000;

/** ***************************************************************************/
Applications::Extension::Extension() : IExtension("Applications") {
    qDebug("[%s] Initialize extension", name_);

    // Add the userspace icons dir which is not covered in the specs
    QFileInfo userSpaceIconsPath(QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath("icons"));
    if (userSpaceIconsPath.exists() && userSpaceIconsPath.isDir())
        QIcon::setThemeSearchPaths(QStringList(userSpaceIconsPath.absoluteFilePath()) << QIcon::themeSearchPaths());

    // Load settings
    QSettings s;
    s.beginGroup(name_);
    searchIndex_.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        rootDirs_ = v.toStringList();
    else
        restorePaths();

    // Set terminal emulator
    v = s.value(CFG_TERM);
    if (v.isValid() && v.canConvert(QMetaType::QString))
        DesktopEntry::terminal = v.toString();
    else{
        DesktopEntry::terminal = getenv("TERM");
        if (DesktopEntry::terminal.isEmpty())
            DesktopEntry::terminal = DEF_TERM;
        else
            DesktopEntry::terminal.append(" -e %1");
    }

    // Keep the Applications in sync with the OS
    updateDelayTimer_.setInterval(UPDATE_DELAY);
    updateDelayTimer_.setSingleShot(true);
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, &updateDelayTimer_, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(this, &Extension::rootDirsChanged, &updateDelayTimer_, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(&updateDelayTimer_, &QTimer::timeout, this, &Extension::updateIndex, Qt::QueuedConnection);

    // Deserialize data
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
                   .filePath(QString("%1.dat").arg(name_)));
    if (dataFile.exists()) {
        if (dataFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
            qDebug("[%s] Deserializing from %s", name_, dataFile.fileName().toLocal8Bit().data());
            QDataStream in(&dataFile);
            quint64 count;
            for (in >> count ;count != 0; --count){
                shared_ptr<DesktopEntry> deshrp = std::make_shared<DesktopEntry>();
                deshrp->deserialize(in);
                index_.push_back(deshrp);
            }
            dataFile.close();

            // Build the offline index
            for (const auto &item : index_)
                searchIndex_.add(item);
        } else
            qWarning() << "Could not open file: " << dataFile.fileName();
    }

    // Trigger initial update
    updateIndex();

    qDebug("[%s] Extension initialized", name_);
}



/** ***************************************************************************/
Applications::Extension::~Extension() {
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
    s.setValue(CFG_PATHS, rootDirs_);
    s.setValue(CFG_TERM, Applications::DesktopEntry::terminal);

    // Serialize data
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("%1.dat").arg(name_)));
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug("[%s] Serializing to %s", name_, dataFile.fileName().toLocal8Bit().data());
        QDataStream out( &dataFile );
        out << static_cast<quint64>(index_.size());
        for (const auto &item : index_)
            item->serialize(out);
        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug("[%s] Extension finalized", name_);
}



/** ***************************************************************************/
QWidget *Applications::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        // Paths
        widget_->ui.listWidget_paths->addItems(rootDirs_);
        connect(this, &Extension::rootDirsChanged, widget_->ui.listWidget_paths, &QListWidget::clear);
        connect(this, &Extension::rootDirsChanged, widget_->ui.listWidget_paths, &QListWidget::addItems);
        connect(widget_.data(), &ConfigWidget::requestAddPath, this, &Extension::addDir);
        connect(widget_.data(), &ConfigWidget::requestRemovePath, this, &Extension::removeDir);
        connect(widget_->ui.pushButton_restorePaths, &QPushButton::clicked, this, &Extension::restorePaths);

        // Fuzzy
        widget_->ui.checkBox_fuzzy->setChecked(searchIndex_.fuzzy());
        connect(widget_->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Info
        widget_->ui.label_info->setText(QString("%1 Applications indexed.").arg(index_.size()));
        connect(this, &Extension::statusInfo, widget_->ui.label_info, &QLabel::setText);

        // If indexer is active connect its statusInfo to the infoLabel
        if (!indexer_.isNull())
            connect(indexer_.data(), &Indexer::statusInfo, widget_->ui.label_info, &QLabel::setText);
    }
    return widget_;
}



/** ***************************************************************************/
void Applications::Extension::handleQuery(shared_ptr<Query> query) {
    // Search for matches. Lock memory against scanworker
    indexAccess_.lock();
    vector<shared_ptr<IIndexable>> indexables = searchIndex_.search(query->searchTerm());
    indexAccess_.unlock();

    // Add results to query. This cast is safe since index holds files only
    for (const shared_ptr<IIndexable> &obj : indexables)
        // TODO `Search` has to determine the relevance. Set to 0 for now
        query->addMatch(std::static_pointer_cast<DesktopEntry>(obj), 0);
}



/** ***************************************************************************/
void Applications::Extension::addDir(const QString & dirPath) {
    qDebug() << "[Applications] Adding dir" << dirPath;

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
    if (rootDirs_.contains(absPath)) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " has already been indexed.").exec();
        return;
    }

    // Check if this dir is a subdir of an existing dir
    for (const QString &p: rootDirs_)
        if (absPath.startsWith(p + '/')) {
            QMessageBox(QMessageBox::Critical, "Error", absPath + " is subdirectory of " + p).exec();
            return;
        }

    // Check if this dir is a superdir of an existing dir, in case delete subdir
    for (QStringList::iterator it = rootDirs_.begin(); it != rootDirs_.end();)
        if (it->startsWith(absPath + '/')) {
            QMessageBox(QMessageBox::Warning, "Warning",
                        (*it) + " is subdirectory of " + absPath + ". " + (*it) + " will be removed.").exec();
            it = rootDirs_.erase(it);
        } else ++it;

    // Add the path to root dirs
    rootDirs_ << absPath;

    // Inform observers
    emit rootDirsChanged(rootDirs_);
}



/** ***************************************************************************/
void Applications::Extension::removeDir(const QString &dirPath) {
    qDebug() << "[Applications] Removing path" << dirPath;

    // Get an absolute file path
    QString absPath = QFileInfo(dirPath).absoluteFilePath();

    // Check existance
    if (!rootDirs_.contains(absPath))
        return;

    // Remove the path
    rootDirs_.removeAll(absPath);

    // Update the widget, if it is visible atm
    emit rootDirsChanged(rootDirs_);
}



/** ***************************************************************************/
void Applications::Extension::restorePaths() {
    qDebug() << "[Applications] Restore paths to defaults";

    // Add standard paths
    rootDirs_.clear();

    //  Add standard paths
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        if (QFileInfo(path).exists())
            addDir(path);
}



/** ***************************************************************************/
void Applications::Extension::updateIndex() {
    qDebug() << "[Applications] Index update triggered";

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
bool Applications::Extension::fuzzy() {
    return searchIndex_.fuzzy();
}



/** ***************************************************************************/
void Applications::Extension::setFuzzy(bool b) {
    searchIndex_.setFuzzy(b);
}


