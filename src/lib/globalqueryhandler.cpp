// Copyright (c) 2022 Manuel Schneider

#include "globalqueryhandler.h"
#include <algorithm>
using namespace std;


void albert::GlobalQueryHandler::handleQuery(albert::Query &query) const
{
    std::vector<Match> &&matches = rankedItems(query);
    sort(matches.begin(), matches.end(), [](const Match &a, const Match &b){ return a.score > b.score; });

    // TODO 0.18 not wasting time here since I cant test but this has tp be done before the release
    //if (useMruScores())
    // https://github.com/albertlauncher/albert/issues/695
    //    auto mru_scores = UsageHistory::mruScores();
    //    stable_sort(items.begin(), items.end(), [&mru_scores](const Match &a, const Match &b) -> bool {
    //        try {
    //            uint ascore = mru_scores.at(a->id());
    //            try {
    //                uint bscore = mru_scores.at(a->id());
    //                return ascore > bscore;
    //            } catch (const out_of_range &e) {
    //                return true;  // b == 0, therefore _not_ greater a
    //            }
    //        } catch (const out_of_range &e) {
    //            return false;  // a == 0, therefore _not_ greater b
    //        }
    //    });

    std::vector<SharedItem> items;
    items.reserve(matches.size());
    for (auto &match : matches)
    items.push_back(std::move(match.item));

    query.set(std::move(items));
}
