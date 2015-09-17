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

#pragma once
#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QFileInfo>
#include <QPointer>
#include <QTimer>
#include "interfaces.h"
#include "utils/search/search.h"
#include "file.h"
#include "scanworker.h"

namespace Files {

struct IndexOptions {
    bool indexAudio;
    bool indexVideo;
    bool indexImage;
    bool indexDocs;
    bool indexDirs;
    bool indexHidden;
    // TODO configurable follow symlinks
};
enum IndexOption {Audio, Video, Image, Docs, Dirs, Hidden};
class ConfigWidget;

class Extension final : public QObject, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)


public:
    Extension();
    ~Extension();

    // GenericPluginInterface
    void initialize() override;
    void finalize() override;
    QWidget *widget() override;

    // ExtensionInterface
    void teardownSession();
    void handleQuery(Query*) override;
    void setFuzzy(bool b = true) override;

    // API special to this extension
    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    void updateIndex();

    inline bool indexOptionAudio() { return _indexOptions.indexAudio; }
    inline bool indexOptionVideo() { return _indexOptions.indexVideo; }
    inline bool indexOptionImage() { return _indexOptions.indexImage; }
    inline bool indexOptionDocs() { return _indexOptions.indexDocs; }
    inline bool indexOptionDirs() { return _indexOptions.indexDirs; }
    inline bool indexOptionHidden() { return _indexOptions.indexHidden; }

    inline void setIndexOptionAudio(bool b = true)  { _indexOptions.indexAudio = b; }
    inline void setIndexOptionVideo(bool b = true)  { _indexOptions.indexVideo = b; }
    inline void setIndexOptionImage(bool b = true)  { _indexOptions.indexImage = b; }
    inline void setIndexOptionDocs(bool b = true)  { _indexOptions.indexDocs = b; }
    inline void setIndexOptionDirs(bool b = true)  { _indexOptions.indexDirs = b; }
    inline void setIndexOptionHidden(bool b = true)  { _indexOptions.indexHidden = b; }

    inline void setScanInterval(uint minutes);
    inline uint scanInterval(){ return _intervalTimer.interval()/60000; }

private:
    QPointer<ConfigWidget> _widget;
    QStringList            _rootDirs;
    IndexOptions           _indexOptions;
    QTimer                 _intervalTimer;
    QPointer<ScanWorker>   _scanWorker;
    QMutex                 _mutex;
    Search                 _searchIndex;
    QList<File*>*          _fileIndex;

    /* constexpr */
    static constexpr const char* CFG_GROUP             = "Files";
    static constexpr const char* CFG_PATHS             = "paths";
    static constexpr const char* CFG_FUZZY             = "fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF         = false;
    static constexpr const char* CFG_INDEX_AUDIO       = "index_audio";
    static constexpr const bool  CFG_INDEX_AUDIO_DEF   = false;
    static constexpr const char* CFG_INDEX_VIDEO       = "index_video";
    static constexpr const bool  CFG_INDEX_VIDEO_DEF   = false;
    static constexpr const char* CFG_INDEX_IMAGE       = "index_image";
    static constexpr const bool  CFG_INDEX_IMAGE_DEF   = false;
    static constexpr const char* CFG_INDEX_DOC         = "index_docs";
    static constexpr const bool  CFG_INDEX_DOC_DEF     = false;
    static constexpr const char* CFG_INDEX_DIR         = "index_dirs";
    static constexpr const bool  CFG_INDEX_DIR_DEF     = false;
    static constexpr const char* CFG_INDEX_HIDDEN      = "index_hidden";
    static constexpr const bool  CFG_INDEX_HIDDEN_DEF  = false;
    static constexpr const char* CFG_SCAN_INTERVAL     = "scan_interval";
    static constexpr const uint  CFG_SCAN_INTERVAL_DEF = 60;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);
};
}
