// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/extension.h>
#include <albert/query.h>

namespace albert
{
class RankItem;

///
/// Abstract trigger query handler extension.
///
/// If the trigger matches, this handler is the only query handler chosen to
/// process the user query. Inherit this class if you dont want your results to
/// be reordered or if you want to display your items of a long running query
/// as soon as they are available.
///
/// \ingroup core
///
class ALBERT_EXPORT TriggerQueryHandler : virtual public Extension
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
    /// Notifies that the user-defined trigger has changed to _trigger_.
    ///
    /// The base class implementation does nothing.
    ///
    virtual void setTrigger(const QString &trigger);

    ///
    /// Returns `true` if the handler supports error tolerant matching, otherwise returns `false`.
    ///
    /// The base class implementation returns `false`.
    ///
    virtual bool supportsFuzzyMatching() const;

    ///
    /// Sets the fuzzy matching mode to _enabled_.
    ///
    /// The base class implementation does nothing.
    ///
    virtual void setFuzzyMatching(bool enabled);

    ///
    /// Handles the triggered _query_.
    ///
    /// @note Executed in a worker thread.
    ///
    virtual void handleTriggerQuery(Query &query) = 0;

protected:

    ///
    /// Destructs the trigger query handler.
    ///
    ~TriggerQueryHandler() override;

};

}
