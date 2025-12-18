// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <albert/extension.h>
#include <albert/querycontext.h>
#include <memory>
class QString;
class QueryEngine;

namespace albert
{
class QueryExecution;
class QueryResults;

///
/// Base query handler interface for triggered queries.
///
/// This class defines the fundamental contract between the core and a query
/// handler. It is used for triggered queries and is selected exclusively when
/// its trigger matches the user input.
///
/// Implementations are responsible for executing the query asynchronously and
/// providing results via a \ref QueryExecution created by \ref execution.
///
/// This interface is low-level and intentionally flexible. Implement it only
/// if your use case is not covered by the convenience subclasses.
///
/// \ingroup core_extension
///
class ALBERT_EXPORT QueryHandler : virtual public Extension
{
public:
    ///
    /// Returns the input hint for the given _query_.
    ///
    /// The returned string will be displayed in the input line if space permits.
    ///
    /// The base class implementation returns an empty string.
    ///
    virtual QString synopsis(const QString &query) const;

    ///
    /// Returns `true` if the user is allowed to set a custom trigger, otherwise returns `false`.
    ///
    /// The base class implementation returns `true`.
    ///
    virtual bool allowTriggerRemap() const;

    ///
    /// Returns the default trigger.
    ///
    /// The base class implementation returns \ref Extension::id() with a space appended.
    ///
    virtual QString defaultTrigger() const;

    ///
    /// Returns `true` if the handler supports fuzzy matching, otherwise returns `false`.
    ///
    /// If `true`, the user can enable fuzzy matching for this handler and \ref
    /// setFuzzyMatching(bool) should be implemented accordingly.
    ///
    /// The base class implementation returns `false`.
    ///
    virtual bool supportsFuzzyMatching() const;

    ///
    /// Creates a query execution for the given _context_.
    ///
    /// The results are added to _results_ as they become available.
    ///
    virtual std::unique_ptr<QueryExecution> execution(QueryContext &context) = 0;

protected:
    /// Destructs the handler.
    ~QueryHandler() override;

    ///
    /// Sets the fuzzy matching mode to _enabled_.
    ///
    /// This function is called when the user toggles fuzzy matching for this handler.
    ///
    /// The base class implementation does nothing.
    ///
    virtual void setFuzzyMatching(bool enabled);

    ///
    /// Notifies that the user-defined trigger has changed to _trigger_.
    ///
    /// This function is called when the user changes the trigger for this handler.
    ///
    /// The base class implementation does nothing.
    ///
    virtual void setTrigger(const QString &trigger);

    friend class ::QueryEngine;
};

}
