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
#include <memory>
using std::shared_ptr;
class Query;
class CoreApi;

struct IPlugin
{
    virtual ~IPlugin() {}

    /**
     * @brief The settings widget factory
     * This has to return the widget that is accessible to the user from the
     * albert settings plugin tab. If the return value is a nullptr there will
     * be no settings widget available in the settings.
     * @return The settings widget
     */
    virtual QWidget* widget() = 0;
};

struct IExtension : public IPlugin
{
    virtual ~IExtension() {}

    /**
     * @brief Initializes the extension
     * Gets called once after the extension has been loaded.
     * Do initial stuff in here.
     * @param manager A reference to the extension manager
     */
    virtual void initialize(/*CoreApi *coreApi*/) = 0;

    /**
     * @brief Finalizes the extension
     * Do final stuff in here, e.g. cleanup, serializing etc...
     * Gets called once before the extension will be unloaded
     */
    virtual void finalize() = 0;

    /**
     * @brief Session setup
     * Called when the main window is shown
     * Do short lived preparation stuff in here. E.g. setup connections etc...
     */
    virtual void setupSession() = 0;

    /**
     * @brief Session teardown
     * Called when the main window hides/closes
     * Cleanup short lived stuff, or start async indexing here
     */
    virtual void teardownSession() = 0;

    /**
     * @brief Query handling
     * Called for every user input.
     * @param query Holds the query context
     */
    virtual void handleQuery(shared_ptr<Query> query) = 0;
};
#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extension"
Q_DECLARE_INTERFACE(IExtension, ALBERT_EXTENSION_IID)
