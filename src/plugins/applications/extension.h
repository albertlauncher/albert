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
#include <QFileSystemWatcher>
#include <QPointer>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QTimer>
#include <QList>
#include <vector>
#include "iextension.h"
#include "search/search.h"

namespace Applications {

class Application;
class ConfigWidget;
class Indexer;

class Extension final : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")
    Q_INTERFACES(IExtension)

    friend class Indexer;

public:
    Extension();
    ~Extension();

    // GenericPluginInterface
    QWidget *widget(QWidget *parent = nullptr) override;

    // IExtension
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
    std::vector<shared_ptr<Application>> _appIndex;
    Search _searchIndex;
    QMutex _indexAccess;
    QPointer<Indexer> _indexer;
    QFileSystemWatcher _watcher;
    QTimer _updateDelayTimer;
    QStringList _rootDirs;

    /* const */
    static const QString EXT_NAME;
    static const QString CFG_PATHS;
    static const QString CFG_FUZZY;
    static const bool    CFG_FUZZY_DEF;
    static const QString CFG_TERM;
    static const QString CFG_TERM_DEF;
    static const bool    UPDATE_DELAY;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);
};
}
