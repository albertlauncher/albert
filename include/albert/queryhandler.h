// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <albert/export.h>
#include <albert/extension.h>
#include <albert/query.h>
#include <memory>
class QString;
class QueryEngine;

namespace albert
{
class Query;
class QueryExecution;
class QueryResults;

///
/// Abstract asynchronous query handler.
///
/// This class is the base of the query handling API. Besides general properties and methods
/// related to query handling and its user configuration it defines the abstract \ref
/// QueryExecution factory \ref execution.
///
/// Since you are supposed to handle the query **asynchonously** and **lazy**, implementing \ref
/// execution is relatively complex. If you dont need that, e.g. because your handler is CPU
/// bound, use the subclasses that sacrifice some of the power of the asychronous API for ease
/// of development.
///
/// \ingroup core
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
