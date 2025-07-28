// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QString>
#include <albert/export.h>
#include <unordered_map>
#include <vector>
namespace albert {
class Extension;
class Item;
class RankItem;
}

struct ItemKey
{
    QString extension_id;
    QString item_id;
    bool operator==(const ItemKey&) const = default;
};

// Hashing specialization for ItemKey
template <>
struct std::hash<ItemKey>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline std::size_t operator()(const ItemKey& key) const
    { return (qHash(key.extension_id) ^ (qHash(key.item_id)<< 1)); }
};

/// Holds and applies usage scores to items based on their usage history.
class ALBERT_EXPORT UsageScoring
{
public:

    double modifiedMatchScore(const ItemKey &key, double match_score) const;

    void modifyMatchScore(const ItemKey &key, double &match_score) const;

    void modifyMatchScore(const QString &extension_id, albert::RankItem &rank_item) const;

    void modifyMatchScore(const albert::Extension &extension, albert::RankItem &rank_item) const;

    void modifyMatchScores(const QString &extension_id,
                           std::vector<albert::RankItem> &rank_items) const;

    void modifyMatchScores(const albert::Extension &extension,
                           std::vector<albert::RankItem> &rank_items) const;

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
