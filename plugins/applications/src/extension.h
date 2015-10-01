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
#include <QMutex>
#include <QPointer>
#include <QTimer>
#include <QFileSystemWatcher>
#include <map>
#include "interfaces/iextension.h"
#include "utils/search/search.h"

namespace Applications {

class Application;
class ConfigWidget;
class Indexer;

class Extension final : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(IExtension)

    friend class Indexer;

public:
    Extension();
    ~Extension();

    // GenericPluginInterface
    QWidget *widget() override;

    // IExtension
    void initialize(/*CoreApi *coreApi*/) override;
    void finalize() override;
    void setupSession() override;
    void teardownSession() override;
    void handleQuery(shared_ptr<Query> query) override;

    // API special to this extension
    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    void updateIndex();

    // Properties
    bool fuzzy();
    void setFuzzy(bool b = true);

private:
    QPointer<ConfigWidget> _widget;
    vector<shared_ptr<Application>> _appIndex;
    Search _searchIndex;
    QMutex _indexAccess;
    QPointer<Indexer> _indexer;
    QFileSystemWatcher _watcher;
    QTimer _updateDelayTimer;
    QStringList _rootDirs;

    /* constexpr */
    static constexpr const char* EXT_NAME       = "applications";
    static constexpr const char* CFG_PATHS      = "applications/paths";
    static constexpr const char* CFG_FUZZY      = "applications/fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF  = false;
    static constexpr const bool  UPDATE_DELAY   = 60000;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);
};
}
