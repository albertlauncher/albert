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
#include <QList>
#include <QString>
#include <QPointer>
#include <QTimer>
#include <QMutex>
#include "interfaces/iextension.h"
#include "utils/search/search.h"


namespace Files {

class File;
class ConfigWidget;
class ScanWorker;

struct IndexOptions {
    bool indexAudio;
    bool indexVideo;
    bool indexImage;
    bool indexDocs;
    bool indexDirs;
    bool indexHidden;
    bool followSymlinks;
};

class Extension final : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(IExtension)

public:
    Extension();
    ~Extension();

    // GenericPluginInterface
    QWidget *widget() override;

    // IExtension
    void initialize(IExtensionManager *em) override;
    void finalize() override;
    void teardownSession();
    void handleQuery(IQuery*) override;

    // API special to this extension
    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    void updateIndex();

    // Properties
    inline bool indexAudio() { return _indexOptions.indexAudio; }
    inline void setIndexAudio(bool b = true)  { _indexOptions.indexAudio = b; }

    inline bool indexVideo() { return _indexOptions.indexVideo; }
    inline void setIndexVideo(bool b = true)  { _indexOptions.indexVideo = b; }

    inline void setIndexImage(bool b = true)  { _indexOptions.indexImage = b; }
    inline bool indexImage() { return _indexOptions.indexImage; }

    inline bool indexDocs() { return _indexOptions.indexDocs; }
    inline void setIndexDocs(bool b = true)  { _indexOptions.indexDocs = b; }

    inline bool indexDirs() { return _indexOptions.indexDirs; }
    inline void setIndexDirs(bool b = true)  { _indexOptions.indexDirs = b; }

    inline bool indexHidden() { return _indexOptions.indexHidden; }
    inline void setIndexHidden(bool b = true)  { _indexOptions.indexHidden = b; }

    inline bool followSymlinks() { return _indexOptions.followSymlinks; }
    inline void setFollowSymlinks(bool b = true)  { _indexOptions.followSymlinks = b; }

    inline unsigned int scanInterval() { return _scanInterval; }
    void setScanInterval(uint minutes);

    bool fuzzy();
    void setFuzzy(bool b = true);

private:
    IExtensionManager      *_manager;
    QPointer<ConfigWidget> _widget;
    QList<File*>*          _fileIndex;
    Search                 _searchIndex;

    QStringList            _rootDirs;
    IndexOptions           _indexOptions;
    QTimer                 _minuteTimer;
    unsigned int           _minuteCounter;
    unsigned int           _scanInterval;
    QPointer<ScanWorker>   _scanWorker;
    QMutex                 _mutex;

    /* constexpr */
    static constexpr const char* EXT_NAME                = "files";
    static constexpr const char* CFG_PATHS               = "paths";
    static constexpr const char* CFG_FUZZY               = "fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF           = false;
    static constexpr const char* CFG_INDEX_AUDIO         = "index_audio";
    static constexpr const bool  CFG_INDEX_AUDIO_DEF     = false;
    static constexpr const char* CFG_INDEX_VIDEO         = "index_video";
    static constexpr const bool  CFG_INDEX_VIDEO_DEF     = false;
    static constexpr const char* CFG_INDEX_IMAGE         = "index_image";
    static constexpr const bool  CFG_INDEX_IMAGE_DEF     = false;
    static constexpr const char* CFG_INDEX_DOC           = "index_docs";
    static constexpr const bool  CFG_INDEX_DOC_DEF       = false;
    static constexpr const char* CFG_INDEX_DIR           = "index_dirs";
    static constexpr const bool  CFG_INDEX_DIR_DEF       = false;
    static constexpr const char* CFG_INDEX_HIDDEN        = "index_hidden";
    static constexpr const bool  CFG_INDEX_HIDDEN_DEF    = false;
    static constexpr const char* CFG_FOLLOW_SYMLINKS     = "follow_symlinks";
    static constexpr const bool  CFG_FOLLOW_SYMLINKS_DEF = true;
    static constexpr const char* CFG_SCAN_INTERVAL       = "scan_interval";
    static constexpr const uint  CFG_SCAN_INTERVAL_DEF   = 60;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);

private slots:
    void onMinuteTick();
};
}
