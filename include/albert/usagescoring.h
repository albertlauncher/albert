// SPDX-FileCopyrightText: 2025-2026 Manuel Schneider

#pragma once
#include <QString>
#include <albert/export.h>
class ItemKey;

namespace albert
{
class RankItem;

///
/// Modifies match scores according to user usage history and preferences.
///
/// Usually you probably rather want to handle instances of this class opaquely and just pass it
/// to relevant functions like \ref lazySort() and \ref GeneratorQueryHandler::lazySort().
///
/// This class uses immutable shared state for speed and thread-safety.
///
/// \ingroup core_query
///
class ALBERT_EXPORT UsageScoring
{
public:

    UsageScoring();
    UsageScoring(bool prioritize_perfect_match, std::unordered_map<ItemKey, double> usage_scores);
    ~UsageScoring();

    /// Returns _rank_items_ with usage scoring applied.
    std::vector<albert::RankItem>
    applied(const QString &extension_id, std::vector<albert::RankItem> &&rank_items) const;

private:

    class Private;
    std::shared_ptr<const Private> d;

};

}