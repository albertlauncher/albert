// SPDX-FileCopyrightText: 2024-2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/extension.h>
#include <albert/query.h>

namespace albert
{
class RankItem;

///
/// Abstract trigger query handler.
///
/// If the trigger matches this handler is the only query handler chosen to
/// process the user query. Inherit this class if you dont want your results to
/// be reordered or if you want to display your items of a long running query
/// as soon as they are available.
///
class ALBERT_EXPORT TriggerQueryHandler : virtual public Extension
{
public:
    /// The synopsis, displayed on empty query.
    /// Use this to give the user hints about accepted query strings.
    /// @returns Empty string.
    virtual QString synopsis(const QString &query) const;

    /// Enable user remapping of the trigger.
    /// @returns False.
    virtual bool allowTriggerRemap() const;

    /// The default (not user defined) trigger.
    /// @returns Extension::id().
    virtual QString defaultTrigger() const;

    /// Setter for the user defined trigger.
    /// Reimplement this if you need this information.
    /// @since 0.24
    virtual void setTrigger(const QString &);

    /// Fuzzy matching capability.
    /// @returns False.
    virtual bool supportsFuzzyMatching() const;

    /// Fuzzy matching behavior.
    /// Default does nothing.
    virtual void setFuzzyMatching(bool enabled);

    /// The trigger query processing function.
    /// @note Executed in a worker thread.
    virtual void handleTriggerQuery(Query &) = 0;

    /// Modifies the score of _rank_items_ to reflect the users usage history.
    void applyUsageScore(std::vector<RankItem> &rank_items);

protected:

    ~TriggerQueryHandler() override;

};

}
