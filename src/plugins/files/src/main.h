// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QPointer>
#include <QTimer>
#include <QMutex>
#include <vector>
#include <memory>
#include "extension.h"
#include "queryhandler.h"
#include "offlineindex.h"
using std::vector;
using std::shared_ptr;

namespace Files {

class File;
class ConfigWidget;

class Extension final :
        public QObject,
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

    class Indexer;

public:

    Extension();
    ~Extension();

    /*
     * Implementation of extension interface
     */

    QString name() const override { return "Files"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) override;

    /*
     * Extension specific members
     */

    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    void updateIndex();

    // Properties
    inline bool indexAudio() { return indexAudio_; }
    inline void setIndexAudio(bool b = true);

    inline bool indexVideo() { return indexVideo_; }
    inline void setIndexVideo(bool b = true);

    inline bool indexImage() { return indexImage_; }
    inline void setIndexImage(bool b = true);

    inline bool indexDocs() { return indexDocs_; }
    inline void setIndexDocs(bool b = true);

    inline bool indexDirs() { return indexDirs_; }
    inline void setIndexDirs(bool b = true);

    inline bool indexHidden() { return indexHidden_; }
    inline void setIndexHidden(bool b = true);

    inline bool followSymlinks() { return followSymlinks_; }
    inline void setFollowSymlinks(bool b = true);

    inline unsigned int scanInterval() { return indexIntervalTimer_.interval()/60000; }
    void setScanInterval(uint minutes);

    bool fuzzy();
    void setFuzzy(bool b = true);

private:
    QPointer<ConfigWidget> widget_;
    vector<shared_ptr<File>> index_;
    Core::OfflineIndex offlineIndex_;
    QMutex indexAccess_;
    QPointer<Indexer> indexer_;
    QTimer indexIntervalTimer_;

    // Index Properties
    QStringList rootDirs_;
    bool indexAudio_;
    bool indexVideo_;
    bool indexImage_;
    bool indexDocs_;
    bool indexDirs_;
    bool indexHidden_;
    bool followSymlinks_;

    /* const */
    static const char* CFG_PATHS;
    static const char* CFG_FUZZY;
    static const bool  DEF_FUZZY;
    static const char* CFG_INDEX_AUDIO;
    static const bool  DEF_INDEX_AUDIO;
    static const char* CFG_INDEX_VIDEO;
    static const bool  DEF_INDEX_VIDEO;
    static const char* CFG_INDEX_IMAGE;
    static const bool  DEF_INDEX_IMAGE;
    static const char* CFG_INDEX_DOC;
    static const bool  DEF_INDEX_DOC;
    static const char* CFG_INDEX_DIR;
    static const bool  DEF_INDEX_DIR;
    static const char* CFG_INDEX_HIDDEN;
    static const bool  DEF_INDEX_HIDDEN;
    static const char* CFG_FOLLOW_SYMLINKS;
    static const bool  DEF_FOLLOW_SYMLINKS;
    static const char* CFG_SCAN_INTERVAL;
    static const uint  DEF_SCAN_INTERVAL;
    static const char* IGNOREFILE;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);

private slots:
    void onMinuteTick();
};
}
