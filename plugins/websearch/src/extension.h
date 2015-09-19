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
#include <QAbstractTableModel>
#include <memory>
#include "plugininterfaces/extensioninterface.h"

class SearchEngine;
class ConfigWidget;

/** ***************************************************************************/
class Extension final : public QAbstractTableModel, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)

    typedef std::shared_ptr<SearchEngine> SharedSearchPtr;
    typedef QList<SharedSearchPtr> SharedSearchPtrList;
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
