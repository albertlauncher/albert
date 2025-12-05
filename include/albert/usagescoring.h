// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <unordered_map>
#include <vector>

namespace albert
{
class RankItem;


struct ALBERT_EXPORT ItemKey
{
    QString extension_id;
    QString item_id;
    bool operator==(const ItemKey&) const = default;
};


///
/// Modifies match scores according to user usage history and preferences.
///
/// \ingroup core
///
class ALBERT_EXPORT UsageScoring
{
public:

    /// Returns the modified _match_score_ for an item identified by _key_.
    double modifiedMatchScore(const ItemKey &key, double match_score) const;

    /// Modifies the match score of _rank_item_ for an item identified by _key_ in-place.
    void modifyMatchScores(const QString &extension_id, std::vector<albert::RankItem> &rank_items) const;

    /// If `true` perfect matches should be prioritized even if their usage score is lower.
    bool prioritize_perfect_match;

    /// The exponential decay applied to usage scores based on recency.
    /// This value adjusts the influence of recent item activations using a geometric weighting
    /// scheme: each activation contributes a weight of 1 / (memory_decay^recency).
    /// A value of 1.0 disables decay, assigning equal weight to all activations and the score of an
    /// item is the sum of its activations (Most Frequently Used).
    /// A value of 0.5 implies that for any activation a_i in history the sum of all older
    /// activations can not exceed the weight of a_i (Most Recently Used).
    /// Valid range: [0.5, 1.0]
    double memory_decay;

    /// The usage scores.
    std::shared_ptr<const std::unordered_map<ItemKey, double>> usage_scores;

};

}

// Hashing specialization for ItemKey
template <>
struct std::hash<albert::ItemKey>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline std::size_t operator()(const albert::ItemKey& key) const
    { return (qHash(key.extension_id) ^ (qHash(key.item_id)<< 1)); }
};
