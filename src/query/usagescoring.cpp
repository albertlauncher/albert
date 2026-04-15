// Copyright (c) 2022-2026 Manuel Schneider

#include "itemkey.hpp"
#include "logging.h"
#include "rankitem.h"
#include "usagescoring.h"
using namespace albert;
using namespace std;

class UsageScoring::Private
{
public:

    /// If `true` perfect matches should be prioritized even if their usage score is lower.
    bool prioritize_perfect_match;

    /// The usage scores.
    std::unordered_map<ItemKey, double> usage_scores;

    inline double applied(const ItemKey &key, double match_score) const
    {
        const auto &it = usage_scores.find(key);

        if (match_score == 1.0 && prioritize_perfect_match)
        {
            if (it != usage_scores.end())
                match_score = 2.0 + it->second;
            else
                match_score = 2.0;
        }
        else if (it != usage_scores.end())
            match_score = 1.0 + it->second;
        // else score remains unmodified

        return match_score;
    }

    vector<RankItem> applied(const QString &extension_id, vector<RankItem> &&rank_items) const
    {
        ItemKey key{extension_id, {}}; // avoid execessive key creation
        for (auto &rank_item : rank_items)
        {
            try {
                key.item_id = rank_item.item->id();
            } catch (const std::exception &e) {
                WARN << QString("Item in extension '%1' threw exception in id(): %2")
                            .arg(extension_id, e.what());
                continue;
            } catch (...) {
                WARN << QString("Item in extension '%1' threw unknown exception in id()")
                            .arg(extension_id);
                continue;
            }
            rank_item.score = applied(key, rank_item.score);
        }
        return ::move(rank_items);
    }

};

UsageScoring::UsageScoring() : d(new Private(false, {}))
{}

UsageScoring::UsageScoring(bool prioritize_perfect_match,
                           std::unordered_map<ItemKey, double> usage_scores) :
    d(new Private{prioritize_perfect_match, ::move(usage_scores)})
{}

UsageScoring::~UsageScoring() = default;

vector<RankItem> UsageScoring::applied(const QString &extension_id, vector<RankItem> &&rank_items) const
{ return d->applied(extension_id, ::move(rank_items)); }
