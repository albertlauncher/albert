// Copyright (c) 2022 Manuel Schneider

#include "globalsearch.h"
#include "albert/util/timeprinter.hpp"
#include "query.h"
#include <QtConcurrent>
#include <cmath>
using namespace std;
using namespace albert;

QString GlobalSearch::id() const { return "globalsearch"; }

QString GlobalSearch::name() const { return {}; }

QString GlobalSearch::description() const { return {}; }

void GlobalSearch::handleQuery(Query &query) const
{
    mutex m;  // 6.4 Still no movesemantics in QtConcurrent
    vector<pair<QueryHandler*,RankItem>> rank_items;

    function<void(GlobalQueryHandler*)> map =
            [this, &m, &rank_items, &query](GlobalQueryHandler *handler) {
                TimePrinter tp(QString("TIME: %1 µs ['%2':'%3']").arg("%1", handler->id(), query.string()));
                auto r = handler->rankItems(query.string(), query.isValid());
                handler->applyUsageScores(r);
                unique_lock lock(m);
                rank_items.reserve(rank_items.size()+r.size());
                for (auto &rank_item : r)
                    rank_items.emplace_back(handler, ::move(rank_item));
            };

    QtConcurrent::blockingMap(handlers, map);

    auto *q = static_cast<::Query*>(&query);
    q->matches_.add(rank_items.begin(), rank_items.end());

//    auto it = rank_items.begin();
//    for (uint e = 0; pow(10,e)-1 < (uint)rank_items.size(); ++e){
//        auto begin = rank_items.begin()+(uint)pow(10u,e)-1;
//        auto end = rank_items.begin()+min((uint)pow(10u,e+1)-1, (uint)rank_items.size());
//        sort(begin, end, [](const auto &a, const auto &b){ return a.second.score > b.second.score; });
//         TODO c++20 ranges view
//    }
}

