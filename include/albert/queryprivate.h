// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/query.h>
class QueryEngine;
namespace albert
{
class QueryHandler;
class QueryResults;
class QueryExecution;
}  // namespace albert

namespace albert::detail
{

/// The query implementation.
class ALBERT_EXPORT Query : public albert::Query
{
public:
    /// Constructs a query.
    Query(std::vector<albert::QueryResult> &&fallbacks,
          QueryHandler &handler,
          QString trigger,
          QString string);

    /// Destructs the query.
    ~Query();

    /// \copydoc albert::Query::isValid
    bool isValid() const override;

    /// \copydoc albert::Query::handler
    QueryHandler &handler() const override;

    /// \copydoc albert::Query::trigger
    QString trigger() const override;

    /// \copydoc albert::Query::string
    QString string() const override;

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
