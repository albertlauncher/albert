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
#include <QSet>
#include <QHash>
#include <QList>
#include <QIcon>
#include <QTimer>
#include <QString>
#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QFileSystemWatcher>
#include "settings.h"
#include "fuzzysearch.h"
#include "prefixsearch.h"
#include "configwidget.h"
#include "extensioninterface.h"

#define DATA_FILE "applauncher.dat"

struct AppInfo {
    QString path;
    QString name;
	QString altName;
	QString iconName;
	QString exec;
	uint    usage;
};

class AppLauncher final : public QObject, public ExtensionInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
	Q_INTERFACES(ExtensionInterface)

    typedef QHash<QString, AppInfo> AppIndex;

public:
    explicit AppLauncher() : _search(nullptr) {}
    ~AppLauncher() {if (_search) delete _search;}

    bool addPath(const QString &);
    bool removePath(const QString &);
    void restorePaths();
    void setFuzzy(bool b = true);


    /*
     * GenericPluginInterface
     */
    QWidget*    widget() override;
    void        initialize() override;
    void        finalize() override;

	/*
	 * ExtensionInterface
     */
	void        setupSession() override;
	void        teardownSession() override;
	void        handleQuery(Query*) override;    
    const QIcon &icon     (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) override;
    void        action    (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) override;
    QString     titleText (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const override;
    QString     infoText  (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const override;
    QString     actionText(const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const override;

private:
    QStringList             _paths;
    bool                    _fuzzy;

    QPointer<ConfigWidget>  _widget;
    QFileSystemWatcher      _watcher;
    AppIndex                _index;
    AbstractSearch<AppInfo> *_search;
    QHash<QString, QIcon>   _iconCache;
    QTimer                  _timer;
    QStringList             _toBeUpdated;

    void onFileSystemChange(const QString &);
    void update(const QString &);
    void clean();

    static bool getAppInfo(const QString &path, AppInfo *appInfo);
    static QIcon getIcon(const QString &iconName);

    static constexpr const char* CFG_PATHS      = "AppLauncher/paths";
    static constexpr const char* CFG_FUZZY      = "AppLauncher/fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF  = true;
    static constexpr const uint  UPDATE_TIMEOUT = 1000;
};
