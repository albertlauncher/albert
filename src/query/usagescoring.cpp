// Copyright (c) 2022-2025 Manuel Schneider

#include "extension.h"
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

void UsageScoring::modifyMatchScore(const ItemKey &key, double &match_score) const
{ match_score = modifiedMatchScore(key, match_score); }

void UsageScoring::modifyMatchScore(const QString &extension_id, RankItem &rank_item) const
{ modifyMatchScore({extension_id, rank_item.item->id()}, rank_item.score); }

void UsageScoring::modifyMatchScore(const Extension &extension, RankItem &rank_item) const
{ modifyMatchScore(extension.id(), rank_item); }

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
        modifyMatchScore(key, rank_item.score);
    }
}

void UsageScoring::modifyMatchScores(const Extension &extension, vector<RankItem> &rank_items) const
{ modifyMatchScores(extension.id(), rank_items); }
