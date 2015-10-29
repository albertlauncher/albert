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
#include "application.h"
#include "indexer.h"
#include "query.h"



/** ***************************************************************************/
Applications::Extension::Extension()
    : IExtension("org.albert.extension.applications",
                 tr("Applications"),
                 tr("Acces your desktop applications via albert")) {
    qDebug() << "Initialize extension:" << id;

    // Add the userspace icons dir which is not covered in the specs
    QFileInfo userSpaceIconsPath(QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath("icons"));
    if (userSpaceIconsPath.exists() && userSpaceIconsPath.isDir())
        QIcon::setThemeSearchPaths(QStringList(userSpaceIconsPath.absoluteFilePath()) << QIcon::themeSearchPaths());

    // Load settings
    QSettings s;
    s.beginGroup(id);

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        rootDirs_ = v.toStringList();
    else
        restorePaths();

    // Set terminal emulator
    v = s.value(CFG_TERM);
    if (v.isValid() && v.canConvert(QMetaType::QString))
        Application::terminal = v.toString();
    else{
        Application::terminal = getenv("TERM");
        if (Application::terminal.isEmpty())
            Application::terminal = CFG_TERM_DEF;
        else
            Application::terminal.append(" -e %1");
    }

    // Keep the Applications in sync with the OS
    updateDelayTimer_.setInterval(UPDATE_DELAY);
    updateDelayTimer_.setSingleShot(true);
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, &updateDelayTimer_, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(this, &Extension::rootDirsChanged, &updateDelayTimer_, static_cast<void(QTimer::*)()>(&QTimer::start));
    connect(&updateDelayTimer_, &QTimer::timeout, this, &Extension::updateIndex);

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.exists())
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug() << "[Applications] Deserializing from" << dataFile.fileName();
            QDataStream in(&dataFile);
            quint64 size;
            in >> size;
            QString path;
            short usage;
            for (quint64 i = 0; i < size; ++i) {
                in >> path >> usage;
                // index is updated after this
                index_.push_back(std::make_shared<Application>(path, usage));
            }
            dataFile.close();
        } else
            qWarning() << "Could not open file: " << dataFile.fileName();

    // Initial update
    updateIndex();

    qDebug() << "Initialization done:" << id;
}



/** ***************************************************************************/
Applications::Extension::~Extension() {
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
    s.setValue(CFG_PATHS, rootDirs_);
    s.setValue(CFG_TERM, Applications::Application::terminal);

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[Applications] Serializing to" << dataFile.fileName();
        QDataStream out( &dataFile );

        // ▼ CRITICAL: Serialize ▼
        QMutexLocker locker(&indexAccess_);
        out << static_cast<quint64>(index_.size());
        for (shared_ptr<AlbertItem> &ai : index_){
            shared_ptr<Application> app = std::static_pointer_cast<Application>(ai);
            out << app->path() << app->usage();
        }
        // ▲ CRITICAL: Serialize ▲

        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "Finalization done:" << id;
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

        // Info
        widget_->ui.label_info->setText(QString("%1 Applications indexed.").arg(index_.size()));
        connect(this, &Extension::statusInfo, widget_->ui.label_info, &QLabel::setText);
    }
    return widget_;
}



/** ***************************************************************************/
vector<shared_ptr<AlbertItem> > Applications::Extension::staticItems() const {
    // ▼ CRITICAL: Copy index ▼
    QMutexLocker locker(&indexAccess_);
    return index_;
    // ▲ CRITICAL: Copy index ▲
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
        widget_->ui.label_info->setText("Waiting for indexer to shut down ...");
        connect(indexer_.data(), &Indexer::destroyed, this, &Extension::updateIndex);
    } else {
        // Create a new scanning runnable for the threadpool
        indexer_ = new Indexer(this);

        //  Run it
        QThreadPool::globalInstance()->start(indexer_);
    }
}


