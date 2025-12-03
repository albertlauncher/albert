// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <albert/item.h>
#include <memory>

namespace albert
{

///
/// An Item with a score.
///
/// Used to rank item results of mutliple handlers
///
/// \ingroup core_query
///
class ALBERT_EXPORT RankItem
{
public:

    ///
    /// Constructs a RankItem with the given `item` and `score`.
    ///
    explicit RankItem(const std::shared_ptr<Item> &item, double score) noexcept;

    ///
    /// Constructs a RankItem with the given `item` and `score` using move semantics.
    ///
    explicit RankItem(std::shared_ptr<Item> &&item, double score) noexcept;

    ///
    /// The less operator
    ///
    bool operator<(const RankItem &other) const;

    ///
    /// The greater operator
    ///
    bool operator>(const RankItem &other) const;

    ///
    /// The matched item
    ///
    std::shared_ptr<Item> item;

    ///
    /// The match score.
    ///
    /// The match score should make sense in the context of the matched item. Often "make sense"
    /// means it should be the fraction of matched characters over length of the string matched
    /// agaist. The empty string should yield a match with a score of 0.
    ///
    /// Must be in the range (0,1]. Not checked for performance.
    ///
    double score;
};

}
