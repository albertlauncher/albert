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
#include <QSet>
#include <QHash>
#include <QList>
#include <QIcon>
#include <QTimer>
#include <QString>
#include <QFileSystemWatcher>
#include <QAbstractTableModel>
#include <memory>
using std::shared_ptr;
#include "extensioninterface.h"

class SearchEngine;
class ConfigWidget;

typedef shared_ptr<SearchEngine> SharedSearchPtr;
typedef QList<SharedSearchPtr> SharedSearchPtrList;


/** ***************************************************************************/
class Extension final : public QAbstractTableModel, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)

    enum class Section{Enabled, Name, Trigger, URL};
    static constexpr unsigned int cColumnCount = 4;

public:
    explicit Extension() {}
    ~Extension() {}

    void restoreDefaults();

    // TODO GLOBAL ACTIONS


    /*
     * Item management
     */
    void        action    (const SearchEngine&, const Query&, Qt::KeyboardModifiers mods) const;
    QString     actionText(const SearchEngine&, const Query&, Qt::KeyboardModifiers mods) const;
    QString     titleText (const SearchEngine&, const Query&) const;
    QString     infoText  (const SearchEngine&, const Query&) const;

    /*
     * Modelinterface
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    bool insertRows (int position, int rows, const QModelIndex & parent = QModelIndex()) override;
    bool removeRows (int position, int rows, const QModelIndex & parent = QModelIndex()) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

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
    /* Core elements */
    SharedSearchPtrList     _index;
    QPointer<ConfigWidget>  _widget;

    /* constexpr */
    static constexpr const char* DATA_FILE      = "websearch.dat";
};


/** ***************************************************************************/
class SearchEngine final : public ItemInterface
{
    friend class Extension;

public:
    SearchEngine() = delete;
    explicit SearchEngine(Extension *ext) : _extension(ext), _enabled(false), _usage(0) {}
    ~SearchEngine(){}

    void         action    (const Query &q, Qt::KeyboardModifiers mods) override { ++_usage; _extension->action(*this, q, mods); }
    QString      actionText(const Query &q, Qt::KeyboardModifiers mods) const override { return _extension->actionText(*this, q, mods); }
    QString      titleText (const Query &q) const override { return _extension->titleText(*this, q); }
    QString      infoText  (const Query &q) const override { return _extension->infoText(*this, q); }
    const QIcon  &icon     () const override { return _icon; }
    uint         usage     () const override { return _usage; }

private:
    Extension*  _extension; // Should never be invalid since the extension must not unload
    bool        _enabled;
    QString     _name;
    QString     _url;
    QString     _trigger;
    QString     _searchTerm;
    QString     _iconPath;
    QIcon       _icon;
    uint        _usage;
};
