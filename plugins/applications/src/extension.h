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
#include <QTimer>
#include <QFileSystemWatcher>
#include "plugininterfaces/extension_if.h"
#include "search/search.h"
#include "application.h"

namespace Applications {

class ConfigWidget;

class Extension final : public QObject, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)

public:
    Extension(){}
    ~Extension(){}

    // GenericPluginInterface
    void initialize() override;
    void finalize() override;
    QWidget *widget() override;

    // ExtensionInterface
    void handleQuery(Query*) override;
    void setFuzzy(bool b = true) override;

    // API special to this extension
    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    void updateIndex();

private:
    static bool getAppInfo(const QString &path, SharedApp appInfo);

    QPointer<ConfigWidget> _widget;
    QStringList            _rootDirs;
    Search<SharedApp>      _searchIndex;
    QList<SharedApp>       _appIndex;
    QFileSystemWatcher     _watcher;
    QTimer                 _updateDelayTimer;

    /* constexpr */
    static constexpr const char* CFG_PATHS      = "Applications/paths";
    static constexpr const char* CFG_FUZZY      = "Applications/fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF  = false;
    static constexpr const char* DATA_FILE      = "applications.dat";
    static constexpr const bool  UPDATE_DELAY   = 60000;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);
};
}
