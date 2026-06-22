// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/querycontext.h>
class QueryEngine;
namespace albert
{
class QueryHandler;
class QueryResult;
class QueryResults;
class QueryExecution;
}  // namespace albert

namespace albert::detail
{

/// The query implementation.
class ALBERT_EXPORT Query
{
public:
    /// Constructs a query.
    Query(UsageScoring usage_scoring,
          std::vector<albert::QueryResult> &&fallbacks,
          QueryHandler &handler,
          QString trigger,
          QString string);

    /// Destructs the query.
    ~Query();

    /// \copydoc albert::Query::isValid
    bool isValid() const;

    /// Returns the identifier of the query.
    uint id() const;

    /// \copydoc albert::Query::handler
    QueryHandler &handler() const;

    /// \copydoc albert::Query::trigger
    const QString &trigger() const;

    /// \copydoc albert::Query::query
    const QString &query() const;

    /// \copydoc albert::Query::usageScoring
    const UsageScoring &usageScoring() const;

    /// Returns the execution of this query if running; else nullptr.
    QueryExecution &execution() const;

    /// Stops the query execution.
    void cancel();

    /// Returns the matches.
    QueryResults &matches();

    /// Returns the fallbacks.
    QueryResults &fallbacks();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace albert::detail
