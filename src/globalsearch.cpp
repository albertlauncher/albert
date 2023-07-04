// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/logging.h"
#include "albert/util/timeprinter.h"
#include "globalqueryhandlerprivate.h"
#include "globalsearch.h"
#include "query.h"
#include <QtConcurrent>
#include <cmath>
using namespace std;
using namespace albert;

GlobalSearch::GlobalSearch(const std::set<GlobalQueryHandler*> &h)
    : handlers(h) {}

QString GlobalSearch::id() const { return "globalsearch"; }

QString GlobalSearch::name() const { return {}; }

QString GlobalSearch::description() const { return {}; }

void GlobalSearch::handleTriggerQuery(TriggerQuery *query) const
{
    if (query->string().trimmed().isEmpty())
        return;

    mutex m;  // 6.4 Still no move semantics in QtConcurrent
    vector<pair<Extension*,RankItem>> rank_items;

    auto * const internal_query = static_cast<::Query*>(query);

    function<void(GlobalQueryHandler*)> map =
        [&m, &rank_items, internal_query](GlobalQueryHandler *handler) {
            try {
                TimePrinter tp(QString("TIME: %1 µs ['%2':'%3']").arg("%1", handler->id(), internal_query->string()));
                    auto r = handler->d->handleGlobalQuery(internal_query);
                if (r.empty()) return;
                unique_lock lock(m);
                rank_items.reserve(rank_items.size()+r.size());
                for (auto &rank_item : r)
                    rank_items.emplace_back(handler, ::move(rank_item));
            } catch (const exception &e) {
                WARN << "Global search:" << handler->id() << "threw" << e.what();
            }
        };

    QtConcurrent::blockingMap(handlers, map);

    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.second.score > b.second.score; });

    internal_query->matches_.add(rank_items.begin(), rank_items.end());

//    auto it = rank_items.begin();
//    for (uint e = 0; pow(10,e)-1 < (uint)rank_items.size(); ++e){
//        auto begin = rank_items.begin()+(uint)pow(10u,e)-1;
//        auto end = rank_items.begin()+min((uint)pow(10u,e+1)-1, (uint)rank_items.size());
//        sort(begin, end, [](const auto &a, const auto &b){ return a.second.score > b.second.score; });
//         TODO c++20 ranges view
//    }
}

