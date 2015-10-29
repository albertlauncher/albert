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
#include <QTimer>
#include <QPointer>
#include <QMutex>
#include <QFileSystemWatcher>
#include <vector>
#include <memory>
#include "iextension.h"
using std::vector;

namespace ChromeBookmarks {

class Bookmark;
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
    vector<shared_ptr<AlbertItem>> staticItems() const override;

    // API special to this extension
    const QString &path();
    void setPath(const QString &s);
    void restorePath();
    void updateIndex();

private:
    QPointer<ConfigWidget> widget_;
    vector<shared_ptr<AlbertItem>> index_;
    mutable QMutex indexAccess_;
    QPointer<Indexer> indexer_;
    QString bookmarksFile_;
    QFileSystemWatcher watcher_;

    /* constexpr */
    static constexpr const char* CFG_BOOKMARKS  = "bookmarkfile";

signals:
    void pathChanged(const QString&);
    void statusInfo(const QString&);
};
}
