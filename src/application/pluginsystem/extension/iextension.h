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
#include <QWidget>
#include <QObject>
#include <memory>
#include <vector>
#include "../iplugin.h"
#include "abstractobjects.hpp"
using std::shared_ptr;
using std::vector;
class Query;
class CoreApi;

class IExtension : public QObject, public IPlugin
{
    Q_OBJECT

public:
    IExtension() = delete;
    IExtension(QString _id, QString _name, QString _description)
        : id(_id), name(_name), description(_description) {}
    virtual ~IExtension() {}

    /**
     * @brief Indicates that this is a triggered extension.
     * Triggered extensions are queried only if one of its triggers are prefic
     * of the query term. \see triggers
     * @return True if this is a triggered extension, else false.
     */
    virtual bool isTriggerOnly() const {return false;}

    /**
     * @brief Get the triggers for an extension.
     * These are the triggers which let the extension be run only if one of
     * its triggers is prefix of the query. These trigger have no effect if
     * isTriggerOnly does return false. \see isTriggerOnly
     * @return The triggers for an extension.
     */
    virtual QStringList triggers() const {return QStringList();}

    /**
     * @brief Indicates that this is a exclusive extension.
     * An exclusive extensions is the single extension that is used to handle a
     * query. An exlusive extension has to be a triggered extension. This
     * property will be ignored if isTriggerOnly returns false. \see
     * isTriggerOnly
     * @return True if this is a exclusive extension, else false.
     */
    virtual bool runExclusive() const {return false;}

    /**
     * @brief Get static items.
     * Unless a query has not been triggered to run explusively, this method
     * will be called on every extension to get the items to build the global
     * offline index. This is intended to facilitate extension development and
     * a the use of unified global search algortihms. To inform the core about
     * changes in the static items emit the staticItemsChanged signal
     * \see
     * @return
     */
    virtual vector<shared_ptr<AlbertItem>> staticItems() const {return vector<shared_ptr<AlbertItem>>();}

    /**
     * @brief Session setup
     * Called when the main window is shown. Do short lived preparation stuff in
     * here. E.g. setup connections etc...
     */
    virtual void setupSession() {}

    /**
     * @brief Session teardown
     * Called when the main window hides/closes. Cleanup short lived stuff here.
     */
    virtual void teardownSession() {}

    /**
     * @brief Query handling
     * Called for every user input.
     * @param query Holds the query context
     */
    virtual void handleQuery(shared_ptr<Query> query)  {}

    /**
     * Deprecated.
     */
    virtual void handleFallbackQuery(shared_ptr<Query> query) {}

    const QString id;
    const QString name;
    const QString description;

signals:
    /**
     * @brief staticItemsChanged signal
     * This signal is emitted to inform the core about changes in the static
     * items.
     * @param e The extesion which static items changed.
     */
    void staticItemsChanged(IExtension *e);
};
#define ALBERT_EXTENSION_IID "org.albert.extension/r1"
Q_DECLARE_INTERFACE(IExtension, ALBERT_EXTENSION_IID)
