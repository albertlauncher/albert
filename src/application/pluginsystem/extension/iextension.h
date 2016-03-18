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
#include <memory>
#include "../iplugin.h"
using std::shared_ptr;
class Query;

class IExtension : public IPlugin
{
public:
    virtual ~IExtension() {}

    /**
     * @brief Session setup
     * Called when the main window is shown
     * Do short lived preparation stuff in here. E.g. setup connections etc...
     */
    virtual void setupSession() {}

    /**
     * @brief Session teardown
     * Called when the main window hides/closes
     * Cleanup short lived stuff, or start async indexing here
     */
    virtual void teardownSession() {}

    /**
     * @brief Indicates that the extension shall be run only if one of its
     * triggers is prefix of the query
     */
    virtual bool isTriggerOnly() const {return false;}
    virtual bool runExclusive() const {return false;}

    /**
     * @brief The triggers which let the extension be run solely, if one of its
     * triggers is prefix of the query
     * @return
     */
    virtual QStringList triggers() const {return QStringList();}

    /**
     * @brief Query handling
     * Called for every user input.
     * @param query Holds the query context
     */
    virtual void handleQuery(shared_ptr<Query> query) = 0;

    /**
     * @brief Fallback handling
     * Called if the preceeding query yielded no results.
     * @param query Holds the query context
     */
    virtual void handleFallbackQuery(shared_ptr<Query> query) { Q_UNUSED(query)}
};
#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extension"
Q_DECLARE_INTERFACE(IExtension, ALBERT_EXTENSION_IID)
