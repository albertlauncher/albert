// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <albert/extension.h>
#include <albert/query.h>
#include <memory>
class QString;
class QueryEngine;

namespace albert
{
class QueryExecution;
class QueryResults;

///
/// Qt native, asynchronous and lazy query handler.
///
/// This class is the base of the query handling API. Besides general properties and methods
/// related to query handling and its user configuration it defines the abstract \ref
/// QueryExecution factory \ref execution.
///
/// When deriving this class you are supposed to handle the query asynchonously and lazily
/// the native Qt way by implementing \ref execution. Compared to the convenience subclasses this
/// is complex. Implement this class only if you know that you have to. Otherwise see \ref
/// AsyncGeneratorQueryHandler or its subclasses.
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
    /// Returns `true` if the handler supports error tolerant matching, otherwise returns `false`.
    ///
    /// If `true`, the user can enable fuzzy matching for this handler and \ref setFuzzyMatching(bool)
    /// should be implemented accordingly.
    ///
    /// The base class implementation returns `false`.
    ///
    virtual bool supportsFuzzyMatching() const;

    ///
    /// Creates a query execution for the given _query_.
    ///
    /// The results are added to _results_ as they become available.
    ///
    virtual std::unique_ptr<QueryExecution> execution(Query &query) = 0;

protected:
    /// Destructs the handler.
    ~QueryHandler() override;

    ///
    /// Sets the fuzzy matching mode to _enabled_.
    ///
    /// This function is called when the user enables or disables fuzzy matching for this handler.
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
