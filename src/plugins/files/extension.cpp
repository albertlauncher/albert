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

#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QThreadPool>
#include <QDir>
#include <memory>
#include "configwidget.h"
#include "extension.h"
#include "indexer.h"
#include "file.h"
#include "query.h"


/** ***************************************************************************/
Files::Extension::Extension()
    : IExtension("org.albert.extension.files",
                 tr("Files"),
                 tr("Access your files via albert")) {
    qDebug() << "Initialize extension:" << id;

    minuteTimer_.setInterval(60000);

    // Load settings
    QSettings s;
    s.beginGroup(id);
    indexAudio_ = s.value(CFG_INDEX_AUDIO, CFG_INDEX_AUDIO_DEF).toBool();
    indexVideo_ = s.value(CFG_INDEX_VIDEO, CFG_INDEX_VIDEO_DEF).toBool();
    indexImage_ = s.value(CFG_INDEX_IMAGE, CFG_INDEX_IMAGE_DEF).toBool();
    indexDocs_ = s.value(CFG_INDEX_DOC, CFG_INDEX_DOC_DEF).toBool();
    indexDirs_ = s.value(CFG_INDEX_DIR, CFG_INDEX_DIR_DEF).toBool();
    indexHidden_ = s.value(CFG_INDEX_HIDDEN, CFG_INDEX_HIDDEN_DEF).toBool();
    followSymlinks_ = s.value(CFG_FOLLOW_SYMLINKS, CFG_FOLLOW_SYMLINKS_DEF).toBool();

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        rootDirs_ = v.toStringList();
    else
        restorePaths();

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.exists())
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug() << "[Files] Deserializing from" << dataFile.fileName();
            QDataStream in(&dataFile);
            QMimeDatabase db;
            QString path, mimename;
            short usage;
            quint64 size;
            in >> size;
            for (quint64 i = 0; i < size; ++i) {
                in >> path >> mimename >> usage;
                index_.push_back(std::make_shared<File>(path, db.mimeTypeForName(mimename), usage));
            }
            dataFile.close();
        } else
            qWarning() << "Could not open file: " << dataFile.fileName();

    // scan interval timer
    connect(&minuteTimer_, &QTimer::timeout, this, &Extension::onMinuteTick);
    setScanInterval(s.value(CFG_SCAN_INTERVAL, CFG_SCAN_INTERVAL_DEF).toUInt());

    // Initial update
    updateIndex();

    qDebug() << "Initialization done:" << id;
}



/** ***************************************************************************/
Files::Extension::~Extension() {
    qDebug() << "Finalize extension:" << id;

    // Stop and wait for background indexer
    minuteTimer_.stop();
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
    s.setValue(CFG_INDEX_AUDIO, indexAudio_);
    s.setValue(CFG_INDEX_VIDEO, indexVideo_);
    s.setValue(CFG_INDEX_IMAGE, indexImage_);
    s.setValue(CFG_INDEX_DIR, indexDirs_);
    s.setValue(CFG_INDEX_DOC, indexDocs_);
    s.setValue(CFG_INDEX_HIDDEN,indexHidden_);
    s.setValue(CFG_FOLLOW_SYMLINKS,followSymlinks_);
    s.setValue(CFG_SCAN_INTERVAL,scanInterval_);
    s.endGroup();

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(id))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[Files] Serializing to " << dataFile.fileName();
        QDataStream out( &dataFile );

        // ▼ CRITICAL: Serialize ▼
        QMutexLocker locker(&indexAccess_);
        out	<< static_cast<quint64>(index_.size());
        for (shared_ptr<AlbertItem> f : index_)
            out << std::static_pointer_cast<File>(f)->path_
                << std::static_pointer_cast<File>(f)->mimetype_.name()
                << std::static_pointer_cast<File>(f)->usage_;
        // ▲ CRITICAL: Serialize ▲

        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "Finalization done:" << id;
}



/** ***************************************************************************/
QWidget *Files::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        // Paths
        widget_->ui.listWidget_paths->addItems(rootDirs_);
        connect(this, &Extension::rootDirsChanged, widget_->ui.listWidget_paths, &QListWidget::clear);
        connect(this, &Extension::rootDirsChanged, widget_->ui.listWidget_paths, &QListWidget::addItems);
        connect(widget_.data(), &ConfigWidget::requestAddPath, this, &Extension::addDir);
        connect(widget_.data(), &ConfigWidget::requestRemovePath, this, &Extension::removeDir);
        connect(widget_->ui.pushButton_restore, &QPushButton::clicked, this, &Extension::restorePaths);
        connect(widget_->ui.pushButton_update, &QPushButton::clicked, this, &Extension::updateIndex);

        // Checkboxes
        widget_->ui.checkBox_audio->setChecked(indexAudio());
        connect(widget_->ui.checkBox_audio, &QCheckBox::toggled, this, &Extension::setIndexAudio);

        widget_->ui.checkBox_video->setChecked(indexVideo());
        connect(widget_->ui.checkBox_video, &QCheckBox::toggled, this, &Extension::setIndexVideo);

        widget_->ui.checkBox_image->setChecked(indexImage());
        connect(widget_->ui.checkBox_image, &QCheckBox::toggled, this, &Extension::setIndexImage);

        widget_->ui.checkBox_docs->setChecked(indexDocs());
        connect(widget_->ui.checkBox_docs, &QCheckBox::toggled, this, &Extension::setIndexDocs);

        widget_->ui.checkBox_dirs->setChecked(indexDirs());
        connect(widget_->ui.checkBox_dirs, &QCheckBox::toggled, this, &Extension::setIndexDirs);

        widget_->ui.checkBox_hidden->setChecked(indexHidden());
        connect(widget_->ui.checkBox_hidden, &QCheckBox::toggled, this, &Extension::setIndexHidden);

        widget_->ui.checkBox_followSymlinks->setChecked(followSymlinks());
        connect(widget_->ui.checkBox_followSymlinks, &QCheckBox::toggled, this, &Extension::setFollowSymlinks);

        widget_->ui.spinBox_interval->setValue(scanInterval());
        connect(widget_->ui.spinBox_interval, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &Extension::setScanInterval);

        // Info
        widget_->ui.label_info->setText(QString("%1 files indexed.").arg(index_.size()));
        connect(this, &Extension::statusInfo, widget_->ui.label_info, &QLabel::setText);
    }
    return widget_;
}



/** ***************************************************************************/
void Files::Extension::teardownSession() {
    File::clearIconCache();
}



/** ***************************************************************************/
vector<shared_ptr<AlbertItem> > Files::Extension::staticItems() const {
    // ▼ CRITICAL: Copy index ▼
    QMutexLocker locker(&indexAccess_);
    return index_;
    // ▲ CRITICAL: Copy index ▲
}



/** ***************************************************************************/
void Files::Extension::addDir(const QString &dirPath) {
    qDebug() << "[Files] Adding dir" << dirPath;

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

    /* Check if this dir is a sub/superdir of an existing dir. This is fine
       since user may have choosen to ignore some dirs (.albertignore). This is
       more complex but also more flexible. At least inform the user */
    for (const QString &p: rootDirs_)
        if (absPath.startsWith(p + '/'))
            QMessageBox(QMessageBox::Warning, "Warning", absPath + " is subdirectory of " + p).exec();
    for (const QString &p: rootDirs_)
        if (p.startsWith(absPath + '/'))
            QMessageBox(QMessageBox::Warning, "Warning", p + " is subdirectory of " + absPath).exec();

    // Add the path to root dirs
    rootDirs_ << absPath;

    // Inform observers
    emit rootDirsChanged(rootDirs_);
}



/** ***************************************************************************/
void Files::Extension::removeDir(const QString &dirPath) {
    qDebug() << "[Files] Removing path" << dirPath;

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
void Files::Extension::restorePaths() {
    qDebug() << "[Files] Restore paths to defaults";

    // Add standard paths
    rootDirs_.clear();
    addDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}



/** ***************************************************************************/
void Files::Extension::updateIndex() {
    qDebug() << "[Files] Index update triggered";

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

        // Reset the timer
        minuteCounter_ = 0;
        minuteTimer_.start();
    }
}



/** ***************************************************************************/
void Files::Extension::setScanInterval(uint minutes) {
    scanInterval_=minutes;
    minuteCounter_=0;
    (minutes == 0) ? minuteTimer_.stop() : minuteTimer_.start();
    qDebug() << "[Files] Scan interval set to" << scanInterval_ << "minutes.";
}


/** ***************************************************************************/
void Files::Extension::onMinuteTick() {
    ++minuteCounter_;
    if (minuteCounter_ == scanInterval_)
        updateIndex(); // resets _minuteCounter
}
