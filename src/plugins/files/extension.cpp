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
Files::Extension::Extension() {
    _minuteTimer.setInterval(60000);
}



/** ***************************************************************************/
Files::Extension::~Extension() {
}



/** ***************************************************************************/
QWidget *Files::Extension::widget() {
    if (_widget.isNull()) {
        _widget = new ConfigWidget;

        // Paths
        _widget->ui.listWidget_paths->addItems(_rootDirs);
        _widget->ui.label_info->setText(QString("%1 files indexed.").arg(_fileIndex.size()));
        connect(this, &Extension::rootDirsChanged, _widget->ui.listWidget_paths, &QListWidget::clear);
        connect(this, &Extension::rootDirsChanged, _widget->ui.listWidget_paths, &QListWidget::addItems);
        connect(_widget, &ConfigWidget::requestAddPath, this, &Extension::addDir);
        connect(_widget, &ConfigWidget::requestRemovePath, this, &Extension::removeDir);
        connect(_widget->ui.pushButton_restore, &QPushButton::clicked, this, &Extension::restorePaths);
        connect(_widget->ui.pushButton_update, &QPushButton::clicked, this, &Extension::updateIndex);

        // Checkboxes
        _widget->ui.checkBox_audio->setChecked(indexAudio());
        connect(_widget->ui.checkBox_audio, &QCheckBox::toggled, this, &Extension::setIndexAudio);

        _widget->ui.checkBox_video->setChecked(indexVideo());
        connect(_widget->ui.checkBox_video, &QCheckBox::toggled, this, &Extension::setIndexVideo);

        _widget->ui.checkBox_image->setChecked(indexImage());
        connect(_widget->ui.checkBox_image, &QCheckBox::toggled, this, &Extension::setIndexImage);

        _widget->ui.checkBox_docs->setChecked(indexDocs());
        connect(_widget->ui.checkBox_docs, &QCheckBox::toggled, this, &Extension::setIndexDocs);

        _widget->ui.checkBox_dirs->setChecked(indexDirs());
        connect(_widget->ui.checkBox_dirs, &QCheckBox::toggled, this, &Extension::setIndexDirs);

        _widget->ui.checkBox_hidden->setChecked(indexHidden());
        connect(_widget->ui.checkBox_hidden, &QCheckBox::toggled, this, &Extension::setIndexHidden);

        _widget->ui.checkBox_followSymlinks->setChecked(followSymlinks());
        connect(_widget->ui.checkBox_followSymlinks, &QCheckBox::toggled, this, &Extension::setFollowSymlinks);

        _widget->ui.checkBox_fuzzy->setChecked(fuzzy());
        connect(_widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        _widget->ui.spinBox_interval->setValue(scanInterval());
        connect(_widget->ui.spinBox_interval, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &Extension::setScanInterval);

        // Info
        connect(this, &Extension::statusInfo, _widget->ui.label_info, &QLabel::setText);
    }
    return _widget;
}



/** ***************************************************************************/
void Files::Extension::initialize() {
    qDebug() << "[Files] Initialize extension";

    // Load settings
    QSettings s;
    s.beginGroup(EXT_NAME);
    _indexAudio = s.value(CFG_INDEX_AUDIO, CFG_INDEX_AUDIO_DEF).toBool();
    _indexVideo = s.value(CFG_INDEX_VIDEO, CFG_INDEX_VIDEO_DEF).toBool();
    _indexImage = s.value(CFG_INDEX_IMAGE, CFG_INDEX_IMAGE_DEF).toBool();
    _indexDocs = s.value(CFG_INDEX_DOC, CFG_INDEX_DOC_DEF).toBool();
    _indexDirs = s.value(CFG_INDEX_DIR, CFG_INDEX_DIR_DEF).toBool();
    _indexHidden = s.value(CFG_INDEX_HIDDEN, CFG_INDEX_HIDDEN_DEF).toBool();
    _followSymlinks = s.value(CFG_FOLLOW_SYMLINKS, CFG_FOLLOW_SYMLINKS_DEF).toBool();
    _searchIndex.setFuzzy(s.value(CFG_FUZZY, CFG_FUZZY_DEF).toBool());

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        _rootDirs = v.toStringList();
    else
        restorePaths();

    // Deserialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
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
            _fileIndex.push_back(std::make_shared<File>(path, db.mimeTypeForName(mimename), usage));
        }
        dataFile.close();
    } else
        qWarning() << "Could not open file: " << dataFile.fileName();

    // scan interval timer
    connect(&_minuteTimer, &QTimer::timeout, this, &Extension::onMinuteTick);
    setScanInterval(s.value(CFG_SCAN_INTERVAL, CFG_SCAN_INTERVAL_DEF).toUInt());

    // Initial update
    updateIndex();

    s.endGroup();
    qDebug() << "[Files] Extension initialized";
}



/** ***************************************************************************/
void Files::Extension::finalize() {
    qDebug() << "[Files] Finalize extension";

    _minuteTimer.stop();

    // Save settings
    QSettings s;
    s.beginGroup(EXT_NAME);
    s.setValue(CFG_FUZZY, _searchIndex.fuzzy());
    s.setValue(CFG_PATHS, _rootDirs);
    s.setValue(CFG_INDEX_AUDIO, _indexAudio);
    s.setValue(CFG_INDEX_VIDEO, _indexVideo);
    s.setValue(CFG_INDEX_IMAGE, _indexImage);
    s.setValue(CFG_INDEX_DIR, _indexDirs);
    s.setValue(CFG_INDEX_DOC, _indexDocs);
    s.setValue(CFG_INDEX_HIDDEN,_indexHidden);
    s.setValue(CFG_FOLLOW_SYMLINKS,_followSymlinks);
    s.setValue(CFG_SCAN_INTERVAL,_scanInterval);
    s.endGroup();

    // Serialize data
    QFile dataFile(
                QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                filePath(QString("%1.dat").arg(EXT_NAME))
                );
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "[Files] Serializing to " << dataFile.fileName();
        QDataStream out( &dataFile );

        // Lock index against indexer
        QMutexLocker locker(&_indexAccess);

        // Serialize
        out	<< static_cast<quint64>(_fileIndex.size());
        for (shared_ptr<File> f : _fileIndex)
            out << f->path_ << f->mimetype_.name() << f->usage_;

        dataFile.close();
    } else
        qCritical() << "Could not write to " << dataFile.fileName();

    qDebug() << "[Files] Extension finalized";
}



/** ***************************************************************************/
void Files::Extension::setupSession() {

}



/** ***************************************************************************/
void Files::Extension::teardownSession() {
    File::clearIconCache();
}



/** ***************************************************************************/
void Files::Extension::handleQuery(shared_ptr<Query> query) {
    // Search for matches. Lock memory against indexer
    _indexAccess.lock();
    vector<shared_ptr<IIndexable>> indexables = _searchIndex.search(query->searchTerm());
    _indexAccess.unlock();

    // Add results to query. This cast is safe since index holds files only
    for (shared_ptr<IIndexable> obj : indexables)
        query->addMatch(std::static_pointer_cast<File>(obj),
                        std::static_pointer_cast<File>(obj)->usage());
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
    if (_rootDirs.contains(absPath)) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " has already been indexed.").exec();
        return;
    }

    /* Check if this dir is a sub/superdir of an existing dir. This is fine
       since user may have choosen to ignore some dirs (.albertignore). This is
       more complex but also more flexible. At least inform the user */
    for (const QString &p: _rootDirs)
        if (absPath.startsWith(p + '/'))
            QMessageBox(QMessageBox::Warning, "Warning", absPath + " is subdirectory of " + p).exec();
    for (const QString &p: _rootDirs)
        if (p.startsWith(absPath + '/'))
            QMessageBox(QMessageBox::Warning, "Warning", p + " is subdirectory of " + absPath).exec();

    // Add the path to root dirs
    _rootDirs << absPath;

    // Inform observers
    emit rootDirsChanged(_rootDirs);
}



/** ***************************************************************************/
void Files::Extension::removeDir(const QString &dirPath) {
    qDebug() << "[Files] Removing path" << dirPath;

    // Get an absolute file path
    QString absPath = QFileInfo(dirPath).absoluteFilePath();

    // Check existance
    if (!_rootDirs.contains(absPath))
        return;

    // Remove the path
    _rootDirs.removeAll(absPath);

    // Update the widget, if it is visible atm
    emit rootDirsChanged(_rootDirs);
}



/** ***************************************************************************/
void Files::Extension::restorePaths() {
    qDebug() << "[Files] Restore paths to defaults";

    // Add standard paths
    _rootDirs.clear();
    addDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}



/** ***************************************************************************/
void Files::Extension::updateIndex() {
    qDebug() << "[Files] Index update triggered";

    // If thread is running, stop it and start this functoin after termination
    if (!_indexer.isNull()) {
        _indexer->abort();
        _widget->ui.label_info->setText("Waiting for indexer to shut down ...");
        connect(_indexer, &Indexer::destroyed, this, &Extension::updateIndex);
    } else {
        // Create a new scanning runnable for the threadpool
        _indexer = new Indexer(this);

        //  Run it
        QThreadPool::globalInstance()->start(_indexer);

        // Reset the timer
        _minuteCounter = 0;
        _minuteTimer.start();

        // If widget is visible show the information in the status bat
        if (!_widget.isNull())
            connect(_indexer, &Indexer::statusInfo, _widget->ui.label_info, &QLabel::setText);
    }
}



/** ***************************************************************************/
void Files::Extension::setScanInterval(uint minutes) {
    _scanInterval=minutes;
    _minuteCounter=0;
    (minutes == 0) ? _minuteTimer.stop() : _minuteTimer.start();
    qDebug() << "[Files] Scan interval set to" << _scanInterval << "minutes.";
}



/** ***************************************************************************/
bool Files::Extension::fuzzy() {
    return _searchIndex.fuzzy();
}



/** ***************************************************************************/
void Files::Extension::setFuzzy(bool b) {
    _indexAccess.lock();
    _searchIndex.setFuzzy(b);
    _indexAccess.unlock();
}



/** ***************************************************************************/
void Files::Extension::onMinuteTick() {
    ++_minuteCounter;
    if (_minuteCounter == _scanInterval)
        updateIndex(); // resets _minuteCounter
}
