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
#include <QtPlugin>
#include <QList>
#include <QString>
#include <QIcon>
#include <QTimer>
#include <QWidget>
#include <QFileSystemWatcher>

#include <memory>
using std::shared_ptr;

#include "fuzzysearch.h"
#include "prefixsearch.h"
#include "extensioninterface.h"

class AppInfo;
class ConfigWidget;

/** ***************************************************************************/
class Extension final : public QObject, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)

    typedef shared_ptr<AppInfo> SharedAppPtr;
    typedef QList<SharedAppPtr> SharedAppPtrList;
    typedef AbstractSearch<SharedAppPtrList> AppSearch;

public:
    explicit Extension() : _search(nullptr) {}
    ~Extension() {if (_search) delete _search;}

    bool addPath(const QString &);
    bool removePath(const QString &);
    void restorePaths();
    void setFuzzy(bool b = true);

    /*
     * Item management
     */
    void        action    (const AppInfo&, const Query&, Qt::KeyboardModifiers mods) const;
    QString     actionText(const AppInfo&, const Query&, Qt::KeyboardModifiers mods) const;
    QString     titleText (const AppInfo&, const Query&) const;
    QString     infoText  (const AppInfo&, const Query&) const;
    const QIcon &icon     (const AppInfo&) const;

    /*
     * ExtensionInterface
     */
    void        handleQuery(Query*) override;

    /*
     * GenericPluginInterface
     */
    QWidget*    widget() override;
    void        initialize() override;
    void        finalize() override;

private:
    void        update(const QString &);
    void        clean();
    static bool getAppInfo(const QString &path, AppInfo *appInfo);

    /* Configurable */
    QStringList _paths;
    bool        _fuzzy;
    AppSearch*  _search;

    /* Core elements */
    SharedAppPtrList       _index;
    QPointer<ConfigWidget> _widget;
    QFileSystemWatcher     _watcher;
    QTimer                 _timer;
    QStringList            _toBeUpdated;

    /* constexpr */
    static constexpr const char* CFG_PATHS      = "AppLauncher/paths";
    static constexpr const char* CFG_FUZZY      = "AppLauncher/fuzzy";
    static constexpr const char* DATA_FILE      = "applauncher.dat";
    static constexpr const bool  CFG_FUZZY_DEF  = true;
    static constexpr const uint  UPDATE_TIMEOUT = 1000;
};

/** ***************************************************************************/
class AppInfo final : public ItemInterface
{
    friend class Extension;

public:
    AppInfo() = delete;
    explicit AppInfo(Extension *ext) : _extension(ext) {}
    ~AppInfo(){}

    void         action    (const Query &q, Qt::KeyboardModifiers mods) override { ++_usage; _extension->action(*this, q, mods); }
    QString      actionText(const Query &q, Qt::KeyboardModifiers mods) const override { return _extension->actionText(*this, q, mods); }
    QString      titleText (const Query &q) const override { return _extension->titleText(*this, q); }
    QString      infoText  (const Query &q) const override { return _extension->infoText(*this, q); }
    const QIcon  &icon     () const override { return _extension->icon(*this); }
    uint         usage     () const override { return _usage; }

private:
    QString    _path;
    QString    _name;
    QString    _altName;
    QString    _exec;
    QIcon      _icon;
    uint       _usage;
    Extension* _extension; // Should never be invalid since the extension must not unload
};
