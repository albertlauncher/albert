// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "queryhandler.h"
class QueryEngine;

namespace albert
{

using Score = uint16_t;
constexpr Score MAX_SCORE = std::numeric_limits<Score>::max();


class ALBERT_EXPORT RankItem
{
public:
    RankItem(std::shared_ptr<Item> item, Score score);
    RankItem(RankItem&&) = default;
    RankItem(const RankItem&) = delete;
    RankItem& operator=(RankItem&&) = default;
    RankItem& operator=(const RankItem&) = delete;

    std::shared_ptr<Item> item;  /// The matched item
    Score score;  /// The match score. @note MAX_SCORE represents a full match.
};


/// Global query handler base class.
/// Run in a batch with all other global query handlers.
/// @note Inherits TriggeredQueryHandler, therefore this extension also handles triggers.
/// @note Do _not_ use this for long running tasks! @see TriggeredQueryHandler
class ALBERT_EXPORT GlobalQueryHandler : public QueryHandler
{
public:
    /// The query handling function. Subclasses should return matched items with appropriate match scores.
    /// The match score should make sense and often (if not always) the fraction of the string match makes sense.
    /// @note Remember to apply Usage scores. @see GlobalQueryHandler::applyUsageScores
    /// @note has to be thread safe!
    virtual std::vector<RankItem> rankItems(const QString &string, const bool& isValid) const = 0;

protected:
    /// Basic QueryHandler implementation: Applies mru scores, sorts and add the items to the query.
    /// @note You can reimplement this if you want your handler to behave different on triggered queries.
    void handleQuery(Query &query) const override;

    /// Takes a list of rank items and modifies its score such that it takes the users usage history into account.
    void applyUsageScores(std::vector<RankItem>&) const;

private:
    friend class ::QueryEngine;
    static void setScores(std::map<std::pair<QString,QString>,double> scores);
    static void setWeight(double weight);
};

}


