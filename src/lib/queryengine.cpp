// Copyright (c) 2022 Manuel Schneider

#include "globalqueryhandler.h"
#include "itemindex.h"
#include "queryengine.h"
#include "queryhandler.h"
#include "scopedtimeprinter.hpp"
#include <QtConcurrent>
#include <mutex>
using namespace albert;
using namespace std;
static const char *CFG_TRIGGER = "trigger";


struct GlobalSearchHandler : public GlobalQueryHandler, public ExtensionWatcher<GlobalQueryHandler>
{
    QString id() const override { return QStringLiteral("globalsearch"); }
    QString synopsis() const override { return QStringLiteral("<filter>"); }
    QString default_trigger() const override { return QLatin1String(""); }
    bool allow_trigger_remap() const override { return false; }
    vector<Match> rankedItems(const albert::Query &query) const override
    {
        // QtConcurrent5 sucks and does not support move (copies…!) hack around
        mutex m;
        vector<Match> results;
        function<void(GlobalQueryHandler*)> map = [&query, &m, &results](GlobalQueryHandler *handler) {
            auto &&intermediate = handler->rankedItems(query);
            [[maybe_unused]] const lock_guard<mutex> lock(m);
            results.insert(end(results),
                           make_move_iterator(begin(intermediate)),
                           make_move_iterator(end(intermediate)));
        };
        QtConcurrent::blockingMap(ExtensionWatcher<GlobalQueryHandler>::extensions(), map);
        return results;
    }
} global_search_handler;


std::unique_ptr<albert::Query> QueryEngine::query(const QString &query_string)
{
    ScopedTimePrinter stp(QString("TIME: %1 µs [QUERY TOTAL '%2']").arg("%1", query_string));

    unique_ptr<::Query> query;

    if (auto it = find_if(trigger_map.cbegin(), trigger_map.cend(),
                      [&query_string](const pair<QString,QueryHandler*> &pair)
                      { return query_string.startsWith(pair.first); }); it != trigger_map.cend())
        query = make_unique<::Query>(*it->second, query_string.mid(it->first.size()), it->first);
    else
        query = make_unique<::Query>(global_search_handler, query_string.mid(it->first.size()));

    // Keep track of queries. Clear items in case of extension unloading
    alive_queries.emplace(query.get());
    QObject::connect(query.get(), &QObject::destroyed, [this, q = query.get()](){ alive_queries.erase(q); });

    return query;
}

void QueryEngine::onAdd(QueryHandler *handler) { updateTriggers(); }

void QueryEngine::onRem(QueryHandler *handler)
{
    updateTriggers();

    // Avoid unloading a shared library while having its objects around
    for (::Query *q :alive_queries)
        q->clear();
}

void QueryEngine::updateTriggers()
{
    trigger_map.clear();

    for (auto *handler: ExtensionWatcher<QueryHandler>::extensions()) {
        auto trigger = handler->allow_trigger_remap()
                ? handler->settings()->value(CFG_TRIGGER, handler->default_trigger()).toString()
                : handler->default_trigger();

        if (trigger.isEmpty()) {
            WARN << QString("Triggers must not be empty: %1.").arg(handler->id());
            continue;
        }

        const auto &[it, success] = trigger_map.emplace(trigger, handler);
        if (!success)
            WARN << QString("Trigger conflict '%1': Already reserved for %2.").arg(trigger, it->second->id());
    }
}

