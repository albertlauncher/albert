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
#include <QFileSystemWatcher>
#include "interfaces/iextension.h"
#include "utils/search/search.h"

namespace Applications {

class App;
class ConfigWidget;

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
    bool fuzzy();
    void setFuzzy(bool b = true);

private:
    static bool getAppInfo(const QString &path, App *app);

    IExtensionManager      *_manager;
    QPointer<ConfigWidget> _widget;
    QList<App*>            _appIndex;
    Search                 _searchIndex;

    QStringList            _rootDirs;
    QFileSystemWatcher     _watcher;
    QTimer                 _updateDelayTimer;
    bool                   _updateOnTearDown;

    /* constexpr */
    static constexpr const char* EXT_NAME       = "applications";
    static constexpr const char* CFG_PATHS      = "Applications/paths";
    static constexpr const char* CFG_FUZZY      = "Applications/fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF  = false;
    static constexpr const bool  UPDATE_DELAY   = 60000;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);
};
}
