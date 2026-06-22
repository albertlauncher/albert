// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>

namespace albert
{
namespace detail { class Query; }
class QueryHandler;
class UsageScoring;

///
/// The query context.
///
/// This class is thread-safe.
///
/// \ingroup core_query
///
class ALBERT_EXPORT QueryContext
{
public:
    QueryContext(const detail::Query &);

    /// Returns `true` if the query is valid; `false` if it has been cancelled.
    bool isValid() const;

    /// Returns the identifier of the query.
    uint id() const;

    /// Returns the trigger string of the query.
    const QString &trigger() const;

    /// Returns the query string of the query.
    const QString &query() const;

    /// Returns the handler of this query.
    const QueryHandler &handler() const;

    /// Returns the usage scoring.
    const UsageScoring &usageScoring() const;

    /// Implicit QString context conversion.
    inline operator const QString &() const { return query(); }

    /// Implicit bool context conversion.
    inline operator bool() const { return isValid(); }

    /// Implicit UsageScoring context conversion.
    inline operator const UsageScoring &() const { return usageScoring(); }

private:
    const detail::Query &query_;
};

}  // namespace albert
