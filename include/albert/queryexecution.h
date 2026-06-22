// SPDX-FileCopyrightText: 2026 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <albert/queryresults.h>

namespace albert
{

///
/// Abstract asynchronous query execution interface.
///
/// Controls the execution of a query, reports busy state and allows to fetch results on demand.
///
/// \ingroup core_query
///
class ALBERT_EXPORT QueryExecution : public QObject
{
    Q_OBJECT

public:

    /// The results of this query.
    QueryResults results;

    ///
    /// Returns `true` if there are more results to fetch, otherwise returns `false`.
    ///
    /// Called only if the query is valid and inactive.
    ///
    /// \pre The query is valid \ref (`QueryContext::is_valid == true`)
    /// \pre The query is inactive (no \ref fetchMore call without subsequent \ref fetchFinished)
    ///
    virtual bool canFetchMore() const = 0;

    ///
    /// Asynchronously fetches more results.
    ///
    /// Called only if the query is valid and inactive and \ref canFetchMore returns `true`. After a
    /// call the query execution is considered active until \ref fetchFinished is emitted.
    ///
    /// @warning This method must not block. Do not forget to emit \ref fetchFinished eventually!
    ///
    /// \pre The query is valid \ref (`QueryContext::is_valid == true`)
    /// \pre The query is inactive (no \ref fetchMore call without subsequent \ref fetchFinished)
    /// \pre Theres more to fetch (\ref canFetchMore returns `true`)
    ///
    virtual void fetchMore() = 0;

    ///
    /// If possible, asynchronously cancels the current fetch operation.
    ///
    /// Support for cancellation of a fetch operation is an optional performance optimization.
    ///
    /// Called on cancellation of an active query. The query will be invalidated before the call.
    ///
    /// The base implementation does nothting.
    ///
    /// @warning This method must not block!
    ///
    /// \pre The query is invalid \ref (`QueryContext::is_valid == false`)
    /// \pre The query is active (\ref fetchMore call without subsequent \ref fetchFinished)
    ///
    virtual void cancel() {}

signals:

    /// Emitted when a fetch initiated by a prior call to \ref fetchMore has finished.
    void fetchFinished();

};

}  // namespace albert
