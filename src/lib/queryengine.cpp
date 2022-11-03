// Copyright (c) 2022 Manuel Schneider

#include "albert/itemindex.h"
#include "albert/logging.h"
#include "albert/queryhandler.h"
#include "queryengine.h"
using namespace albert;
using namespace std;
static const char *CFG_TRIGGER = "trigger";


QueryEngine::QueryEngine(ExtensionRegistry &registry) :
        ExtensionWatcher<QueryHandler>(registry),
        global_search_handler(registry)
{
}

std::unique_ptr<albert::Query> QueryEngine::query(const QString &query_string)
{
    for (auto *q  : alive_queries)
        q->cancel();

    unique_ptr<::Query> query;

    for (const auto &[trigger, handler] : trigger_map)
        if (query_string.startsWith(trigger))
            query = make_unique<::Query>(*handler, query_string.mid(trigger.size()), trigger);

    if (!query)
        query = make_unique<::Query>(global_search_handler, query_string);

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

    for (auto [id, handler]: extensionRegistry().extensionsOfType<QueryHandler>()) {
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
