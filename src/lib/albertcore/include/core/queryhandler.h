// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
#include <QStringList>
#include "core_globals.h"

namespace Core {

class Query;

class EXPORT_CORE QueryHandler
{
public:

    QueryHandler(QString id) : id(id) {}
    virtual ~QueryHandler() {}

    const QString id;

    /**
     * @brief The triggers that makes the plugin beeing run exclusice
     */
    virtual QStringList triggers() const { return QStringList(); }

    /**
     * @brief Session setup
     * Called when the users started a session, i.e. before the the main window
     * is shown. Setup session stage has to be finished before the actual query
     * handling will begin so do not do any long running jobs in here. If you
     * really have to, do it asynchonous or threaded (only recommended if you
     * know what you do).
     */
    virtual void setupSession() {}

    /**
     * @brief Session teardown
     * Called when the user finshed a session, i.e. after the the main window
     * has been hidden. Although the app is hidden now this method should not
     * block since the user may want start another session immediately.
     * @see setupSession
     */
    virtual void teardownSession() {}

    /**
     * @brief The ExecutionType enum
     * The execution type of a queryhandler determines the way the handler is run. Batch handlers
     * are runned first. Query results are not displayed until all batch handlers finished. When all
     * batch handlers finished the results are sorted and displayed. Realtime handlers are started
     * afterwards. The results of the realtime handlers are not sorted anymore and displayed
     * instantly (well they are buffered for 50 ms). Note that the results are simply appended
     * once they are displayed, therefore it may be wise for realtime handlers to have triggers.
     */
    enum class ExecutionType { Batch, Realtime };

    /**
     * @brief executionType
     * @return The execution type of the queryhandler
     * @see ExecutionType
     */
    virtual ExecutionType executionType() const { return ExecutionType::Batch; }

    /**
     * @brief Query handling
     * This method is called for every user input. Add the results to the query
     * passed as parameter. The results are sorted by usage. After 100 ms
     * they are just appended to not disturb the users interaction. Queries can
     * get invalidated so make sure to regularly check isValid() to cancel
     * long running operations. This method is called in a thread without event
     * loop, be aware of the consequences (especially regarding signal/slot
     * mechanism).
     * Note that this method may run simultaneously in separate threads make
     * sure that everything you touch is threadsafe.
     * @param query Holds the query context
     */
    virtual void handleQuery(Query *query) = 0;

};

}
