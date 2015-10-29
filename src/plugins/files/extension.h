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
#include <QString>
#include <QPointer>
#include <QTimer>
#include <QMutex>
#include <vector>
#include "iextension.h"
using std::vector;

namespace Files {

class File;
class ConfigWidget;
class Indexer;

class Extension final : public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")
    Q_INTERFACES(IExtension)

    friend class Indexer;

public:
    Extension();
    ~Extension();

    // IExtension
    QWidget *widget(QWidget *parent = nullptr) override;
    void teardownSession() override;
    vector<shared_ptr<AlbertItem>> staticItems() const override;

    // API special to this extension
    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    void updateIndex();

    // Properties
    inline bool indexAudio() { return indexAudio_; }
    inline void setIndexAudio(bool b = true)  { indexAudio_ = b; }

    inline bool indexVideo() { return indexVideo_; }
    inline void setIndexVideo(bool b = true)  { indexVideo_ = b; }

    inline void setIndexImage(bool b = true)  { indexImage_ = b; }
    inline bool indexImage() { return indexImage_; }

    inline bool indexDocs() { return indexDocs_; }
    inline void setIndexDocs(bool b = true)  { indexDocs_ = b; }

    inline bool indexDirs() { return indexDirs_; }
    inline void setIndexDirs(bool b = true)  { indexDirs_ = b; }

    inline bool indexHidden() { return indexHidden_; }
    inline void setIndexHidden(bool b = true)  { indexHidden_ = b; }

    inline bool followSymlinks() { return followSymlinks_; }
    inline void setFollowSymlinks(bool b = true)  { followSymlinks_ = b; }

    inline unsigned int scanInterval() { return scanInterval_; }
    void setScanInterval(uint minutes);

private:
    QPointer<ConfigWidget> widget_;
    vector<shared_ptr<AlbertItem>> index_;
    mutable QMutex indexAccess_;
    QPointer<Indexer> indexer_;
    QTimer minuteTimer_;
    unsigned int minuteCounter_;

    // Index Properties
    QStringList rootDirs_;
    bool indexAudio_;
    bool indexVideo_;
    bool indexImage_;
    bool indexDocs_;
    bool indexDirs_;
    bool indexHidden_;
    bool followSymlinks_;
    unsigned int scanInterval_;

    /* constexpr */
    static constexpr const char* CFG_PATHS               = "paths";
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
    static constexpr const char* IGNOREFILE              = ".albertignore";

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);

private slots:
    void onMinuteTick();
};
}
