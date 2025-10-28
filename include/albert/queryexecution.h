// Copyright (c) 2022-2025 Manuel Schneider

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
/// \ingroup core
///
class ALBERT_EXPORT QueryExecution : public QObject
{
    Q_OBJECT

public:

    /// Constructs a query execution for _query_
    QueryExecution(Query &query);

    /// The unique id of this query execution.
    const uint id;

    /// The unique id of this query execution.
    const Query &query;

    /// The results of this query.
    QueryResults results;

    /// Cancels the query processing.
    virtual void cancel() = 0;

    /// Fetches more results.
    virtual void fetchMore() = 0;

    /// Returns `true` if there are more results to fetch, otherwise returns `false`.
    virtual bool canFetchMore() const = 0;

    /// Returns `true` if the query is being processed, otherwise returns `false`.
    virtual bool isActive() const = 0;

signals:

    ///
    /// Emitted when query processing started or finished.
    ///
    /// @note The UI state machine expects results to be added only while active is `true`.
    ///
    void activeChanged(bool active);

};

}  // namespace albert
