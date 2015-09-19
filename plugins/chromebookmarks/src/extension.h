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
#include <QTimer>
#include <QIcon>
#include <QFileSystemWatcher>
#include <memory>
#include "plugininterfaces/extensioninterface.h"
#include "search/fuzzysearch.h"
#include "search/prefixsearch.h"

class Bookmark;
class ConfigWidget;

/** ***************************************************************************/
class Extension final : public QObject, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)

    typedef std::shared_ptr<Bookmark> SharedBookmarkPtr;
    typedef QList<SharedBookmarkPtr> SharedBookmarkPtrList;
    typedef AbstractSearch<SharedBookmarkPtrList> BookmarkSearch;

public:
    explicit Extension() : _search(nullptr) {}
    ~Extension() {if (_search) delete _search;}

    void setPath(const QString &s);
    void restorePath();
    void setFuzzy(bool b = true);

    /*
     * Item management
     */
    void        action    (const Bookmark&, const Query&, Qt::KeyboardModifiers mods) const;
    QString     actionText(const Bookmark&, const Query&, Qt::KeyboardModifiers mods) const;
    QString     titleText (const Bookmark&b, const Query&) const;
    QString     infoText  (const Bookmark&b, const Query&) const;
    const QIcon &icon     (const Bookmark&b) const;

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
    void                         update();

    /* Configurable */
    QString         _bookmarksFile;
    bool            _fuzzy;
    BookmarkSearch* _search;

    /* Core elements */
    SharedBookmarkPtrList  _index;
    QPointer<ConfigWidget> _widget;
    QFileSystemWatcher     _watcher;
    QTimer                 _timer;
    QIcon                  _favicon;

    /* constexpr */
    static constexpr const char* DATA_FILE      = "chromebookmarks.dat";
    static constexpr const char* CFG_BOOKMARKS  = "ChromeBookmarks/bookmarkfile";
    static constexpr const char* CFG_FUZZY      = "ChromeBookmarks/fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF  = false;
    static constexpr const uint  UPDATE_TIMEOUT = 1000;

signals:
    void pathChanged(const QString&);
};
