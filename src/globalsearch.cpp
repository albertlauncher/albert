// Copyright (c) 2022 Manuel Schneider

#include "albert/util/timeprinter.hpp"
#include "extensions/globalqueryhandlerprivate.h"
#include "globalsearch.h"
#include "query.h"
#include <QtConcurrent>
#include <cmath>
using namespace std;
using namespace albert;

QString GlobalSearch::id() const { return "globalsearch"; }

QString GlobalSearch::name() const { return {}; }

QString GlobalSearch::description() const { return {}; }

void GlobalSearch::handleTriggerQuery(TriggerQuery &query) const
{
    if (query.string().trimmed().isEmpty())
        return;

    mutex m;  // 6.4 Still no move semantics in QtConcurrent
    vector<pair<Extension*,RankItem>> rank_items;

    function<void(GlobalQueryHandlerPrivate*)> map =
        [&m, &rank_items, &query](GlobalQueryHandlerPrivate *handler) {
            TimePrinter tp(QString("TIME: %1 µs ['%2':'%3']").arg("%1", handler->q->id(), query.string()));
            auto r = handler->handleGlobalQuery(dynamic_cast<GlobalQueryHandler::GlobalQuery&>(query));
            unique_lock lock(m);
            rank_items.reserve(rank_items.size()+r.size());
            for (auto &rank_item : r)
                rank_items.emplace_back(handler->q, ::move(rank_item));
        };

    QtConcurrent::blockingMap(handlers, map);

    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.second.score > b.second.score; });

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

