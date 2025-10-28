// Copyright (c) 2022-2025 Manuel Schneider

#include "logging.h"
#include "rankitem.h"
#include "usagescoring.h"
using namespace albert;
using namespace std;

double UsageScoring::modifiedMatchScore(const ItemKey &key, double match_score) const
{
    const auto &it = usage_scores->find(key);

    if (match_score == 1.0 && prioritize_perfect_match)
    {
        if (it != usage_scores->end())
            match_score = 2.0 + it->second;
        else
            match_score = 2.0;
    }
    else if (it != usage_scores->end())
        match_score = 1.0 + it->second;
    // else score remains unmodified

    return match_score;
}

void UsageScoring::modifyMatchScores(const QString &extension_id, vector<RankItem> &rank_items) const
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
            WARN << QString("Item in extension '%1' threw unknown exception in id()").arg(extension_id);
            continue;
        }
        rank_item.score = modifiedMatchScore(key, rank_item.score);
    }
}
