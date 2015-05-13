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

#include "extension.h"
#include <QDebug>
#include <QProcess>
#include <QDirIterator>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <memory>
#include <functional>
//#include <QList>
//#include <QTimer>
//#include <QMutex>
//#include <QThread>

#include "configwidget.h"
#include "query.h"
#include "file.h"

namespace Files{
/** ***************************************************************************/
Extension::Extension() {
    qDebug() << "Initialize extension 'Files'";
    _fileSearch.setFuzzy(QSettings().value(CFG_FUZZY, CFG_FUZZY_DEF).toBool());
    connect(&_fileIndex, &FileIndex::fileAdded, _fileSearch, SLOT(&Search<uint>::add);
    connect(&_fileIndex, &FileIndex::fileAdded, [&](uint i, const QStringList& l){_fileSearch.add(i,l);});
    connect(&_fileIndex, &FileIndex::fileRemoved, [&](uint i){_fileSearch.remove(i);});
    connect(&_fileIndex, &FileIndex::indexReset, [&](){_fileSearch.reset();});
    _fileIndex.reportAllFilesOnce();
}



/** ***************************************************************************/
Extension::~Extension() {
    qDebug() << "Finalize extension 'Files'";
    QSettings().setValue(CFG_FUZZY, _fileSearch.fuzzy());
}



/** ***************************************************************************/
void Extension::teardownSession() {
    File::clearIconCache();
}



/** ***************************************************************************/
void Extension::handleQuery(Query *q) {
    QList<uint> ids = _fileSearch.search(q->searchTerm());
    for (uint i : ids)
        q->addResult(_fileIndex.getFile(i));
}



/** ***************************************************************************/
void Extension::setFuzzy(bool b) {
    _fileSearch.setFuzzy(b);
}



/** ***************************************************************************/
void Extension::initialize() {
}



/** ***************************************************************************/
void Extension::finalize() {
}



/** ***************************************************************************/
QWidget *Extension::widget() {
    if (_widget.isNull()){
        _widget = new ConfigWidget;

        // Paths <--
        _widget->ui.listWidget_paths->addItems(_fileIndex.rootDirs());
        connect(&_fileIndex, &FileIndex::rootDirAdded, _widget->ui.listWidget_paths, (void (QListWidget::*)(const QString &))&QListWidget::addItem);
        // Paths -->
        connect(_widget, &ConfigWidget::requestAddPath, &_fileIndex, &FileIndex::addDir);
        connect(_widget, &ConfigWidget::requestRemovePath, &_fileIndex, &FileIndex::removeDir);
        connect(_widget->ui.pushButton_restore, &QPushButton::clicked, &_fileIndex, &FileIndex::restorePaths);
        connect(_widget->ui.pushButton_update, &QPushButton::clicked, &_fileIndex, &FileIndex::completeUpdate);

        // Checkboxes
        _widget->ui.checkBox_fuzzy->setChecked(_fileSearch.fuzzy());
        connect(_widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        _widget->ui.checkBox_audio->setChecked(_fileIndex.indexOption(FileIndex::IndexOption::Audio));
        connect(_widget->ui.checkBox_audio, &QCheckBox::toggled, [this](bool b){_fileIndex.setIndexOption(FileIndex::IndexOption::Audio, b);});

        _widget->ui.checkBox_video->setChecked(_fileIndex.indexOption(FileIndex::IndexOption::Video));
        connect(_widget->ui.checkBox_video, &QCheckBox::toggled, [this](bool b){_fileIndex.setIndexOption(FileIndex::IndexOption::Video, b);});

        _widget->ui.checkBox_image->setChecked(_fileIndex.indexOption(FileIndex::IndexOption::Image));
        connect(_widget->ui.checkBox_image, &QCheckBox::toggled, [this](bool b){_fileIndex.setIndexOption(FileIndex::IndexOption::Image, b);});

        _widget->ui.checkBox_docs->setChecked(_fileIndex.indexOption(FileIndex::IndexOption::Docs));
        connect(_widget->ui.checkBox_docs, &QCheckBox::toggled, [this](bool b){_fileIndex.setIndexOption(FileIndex::IndexOption::Docs, b);});

        _widget->ui.checkBox_dirs->setChecked(_fileIndex.indexOption(FileIndex::IndexOption::Dirs));
        connect(_widget->ui.checkBox_dirs, &QCheckBox::toggled, [this](bool b){_fileIndex.setIndexOption(FileIndex::IndexOption::Dirs, b);});

        _widget->ui.checkBox_hidden->setChecked(_fileIndex.indexOption(FileIndex::IndexOption::Hidden));
        connect(_widget->ui.checkBox_hidden, &QCheckBox::toggled, [this](bool b){_fileIndex.setIndexOption(FileIndex::IndexOption::Hidden, b);});

        // Info
        connect(&_fileIndex, &FileIndex::statusInfo, _widget, &ConfigWidget::setVanishingInfo);
    }
    return _widget;
}
}





///////////////////////////////////////////////////////////////////////////////

//SharedFilePtr FileIndex::getFile(uint id)
//{
//    shared_ptr<File> p = std::make_shared<File>();
//    QString mimetype;
//    _fileDB.getInfoById(id, &p->_path, &p->_name, &mimetype, &p->_usage);
//    p->_mimetype = _mimeDB.mimeTypeForName(mimetype);
//    p->_filesindex = this;
//    return p;
//}
//    /* Deserialze data */
//    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
//    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
//        qDebug() << "Deserializing from" << f.fileName();
//        QDataStream in(&f);
//        quint64 size;
//        in >> size;
//        for (quint64 i = 0; i < size; ++i) {
//            SharedFilePtr p(new FileInfo(this));
//            in >> p->_filePath >> p->_usage;
//            p->_mimeType = _mimeDatabase.mimeTypeForFile(p->_filePath);
//            _index.push_back(p);
//        }
//        f.close();
//    } else
//        qWarning() << "Could not open file: " << f.fileName();
//QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
//if (f.open(QIODevice::ReadWrite| QIODevice::Text)) {
//    qDebug() << "Serializing to " << f.fileName();
//    QDataStream out( &f );
//    out << static_cast<quint64>(_index.size());
//    for (SharedFilePtr p : _index)
//        out << p->_filePath << p->_usage;
//    f.close();
//} else
//    qCritical() << "Could not write to " << f.fileName();

