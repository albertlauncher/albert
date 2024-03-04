// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/query/rankitem.h"
#include "albert/query/triggerqueryhandler.h"
#include <vector>
class GlobalQueryHandlerPrivate;

namespace albert
{

/// Global search query handler class.
/// A functional query handler returning scored items. Applicable for the
/// global search. Use this if you want your results show up in the global
/// search. Implements TriggeredQueryHandler.
/// @note Do _not_ use this for long running tasks! @see TriggeredQueryHandler
class ALBERT_EXPORT GlobalQueryHandler : public albert::TriggerQueryHandler
{
public:
    GlobalQueryHandler();
    ~GlobalQueryHandler() override;

    /// The query interface used by GlobalQueryHandler
    class GlobalQuery
    {
    public:
        virtual ~GlobalQuery() = default;

        /// The query string excluding the trigger.
        virtual QString string() const = 0;

        /// True if query has not been cancelled.
        /// @note Stop query processing if false.
        virtual const bool &isValid() const = 0;
    };

    /// The query processing function.
    /// The match score should make sense and often (if not always) be the
    /// fraction matched chars (legth of query string / length of item title).
    /// @return A list of match items. Empty query should return all items with
    /// a score of 0.
    /// @note Executed in a worker thread.
    virtual std::vector<RankItem> handleGlobalQuery(const GlobalQuery*) const = 0;

    /// Takes rank items and modifies the score according to the users usage.
    /// Use this if you want to reuse your global results in the trigger handler.
    void applyUsageScore(std::vector<RankItem>*) const;

    /// Implements handleTriggerQuery(…). Sort and batch add rankItems(…).
    /// @note Reimplement if the handler should have custom triggered behavior,
    /// but think twice if this is necessary. It may break user expectation.
    /// @see handleTriggerQuery and rankItems
    void handleTriggerQuery(TriggerQuery*) const override;

private:
    std::unique_ptr<GlobalQueryHandlerPrivate> d;
};

}
