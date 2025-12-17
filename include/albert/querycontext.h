// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>

namespace albert
{
class QueryHandler;
class UsageScoring;

///
/// Query interface.
///
/// \ingroup core_query
///
class ALBERT_EXPORT QueryContext
{
public:

    /// Returns `true` if the query is valid; `false` if it has been cancelled.
    virtual bool isValid() const = 0;

    /// Returns the handler of this query.
    virtual const QueryHandler &handler() const = 0;

    /// Returns the trigger of this query.
    virtual QString trigger() const = 0;

    /// Returns the query string excluding the trigger.
    virtual QString string() const = 0;

    /// Returns the usage scoring.
    virtual const UsageScoring &usageScoring() const = 0;

    /// Implicit QString context conversion.
    operator QString() const { return string(); }

protected:

    virtual ~QueryContext() = default;
};

}
