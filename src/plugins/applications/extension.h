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
#include "offlineindex.h"

namespace Applications {

class DesktopEntry;
class ConfigWidget;

class Extension final : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")
    Q_INTERFACES(IExtension)

    class Indexer;

public:

    Extension();
    ~Extension();

    /*
     * Implementation of extension interface
     */

    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(shared_ptr<Query> query) override;

    /*
     * Extension specific members
     */

    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();
    bool fuzzy();
    void setFuzzy(bool b = true);
    void updateIndex();

private:
    QPointer<ConfigWidget> widget_;
    std::vector<shared_ptr<DesktopEntry>> index_;
    OfflineIndex offlineIndex_;
    QMutex indexAccess_;
    QPointer<Indexer> indexer_;
    QFileSystemWatcher watcher_;
    QTimer updateDelayTimer_;
    QStringList rootDirs_;

    /* const */
    static const char* CFG_PATHS;
    static const char* CFG_FUZZY;
    static const bool  DEF_FUZZY;
    static const char* CFG_TERM;
    static const char* DEF_TERM;
    static const bool  UPDATE_DELAY;

signals:
    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);
};
}
