// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QString>
#include <QObject>
#include <QWidget>
#include <memory>
#include "query.h"
#include "abstractitem.h"
using std::shared_ptr;


/** ****************************************************************************
 * @brief The extension interface
 */
struct AbstractExtension : public QObject
{
    Q_OBJECT

public:

    AbstractExtension(const char * id) : id(id) {}
    virtual ~AbstractExtension() {}

    /**
     * @brief An application-wide unique identifier
     */
    const char* id;

    /**
     * @brief A human readable name of the extension
     * @return The human readable name
     */
    virtual QString name() const = 0;

    /**
     * @brief The settings widget factory
     * This has to return the widget that is accessible to the user from the
     * albert settings plugin tab. If the return value is a nullptr there will
     * be no settings widget available in the settings.
     * @return The settings widget
     */
    virtual QWidget* widget(QWidget *parent = nullptr) {return new QWidget(parent);}

    /**
     * @brief "Run excluscive" indicator
     * Indicates that this extension query wants to be the single extension to
     * be run. The extension must provide triggers for this behaviour.
     */
    virtual bool runExclusive() const { return false; }

    /**
     * @brief The triggers that make the extension beeing run
     * If runExclusice is set and the first word in the query matches one of
     * this triggers the extension is run exclusively.
     */
    virtual QStringList triggers() const {return QStringList();}

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
     * @brief The query handler
     * This method is called for every user input. Add the results to the query
     * passed as parameter. The results are sorted by usage. But after 100 ms
     * they are just appended to not disturb the users interaction. Queries can
     * get invalidated so make sure to regularly check isValid() to cancel
     * long running operations.
     * @param query Holds the query context
     */
    virtual void handleQuery(Query query) { Q_UNUSED(query) }

    /**
     * @brief Fallbacks of this extension
     * This items show up if a query yields no results
     */
    virtual vector<SharedItem> fallbacks(QString) {return vector<SharedItem>();}

};
#define ALBERT_EXTENSION_IID "org.albert.extension"
Q_DECLARE_INTERFACE(AbstractExtension, ALBERT_EXTENSION_IID)
