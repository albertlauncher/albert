// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

/// \defgroup query_util Query utility API
/// \ingroup util
/// Utility classes and helper functions for query related tasks.

#pragma once
#include <QString>
#include <albert/export.h>

namespace albert
{
class QueryHandler;

///
/// Query interface.
///
/// \ingroup core
///
class ALBERT_EXPORT Query
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

    /// Implicit QString context conversion.
    operator QString() const { return string(); }

protected:

    virtual ~Query() = default;
};

}
