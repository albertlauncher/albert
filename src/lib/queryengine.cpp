// Copyright (c) 2022 Manuel Schneider

#include <QObject>
#include "itemindex.h"
#include "logging.h"
#include "queryengine.h"
#include "queryhandler.h"
#include "query.h"
#include "scopedtimeprinter.hpp"
#include <QtConcurrent>
#include <mutex>
using namespace albert;
using namespace std;
static const char *CFG_TRIGGER = "trigger";


struct GlobalSearchHandler : GlobalQueryHandler, ExtensionWatcher<IndexQueryHandler>
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
        function<void(IndexQueryHandler*)> map = [&query, &m, &results](IndexQueryHandler *handler) {
            auto &&intermediate = handler->rankedItems(query);
            [[maybe_unused]] const lock_guard<mutex> lock(m);
            results.insert(end(results),
                           make_move_iterator(begin(intermediate)),
                           make_move_iterator(end(intermediate)));
        };
        QtConcurrent::blockingMap(ExtensionWatcher<IndexQueryHandler>::extensions(), map);
        return results;
    }
};


albert::Query* QueryEngine::query(const QString &query_string)
{
    ScopedTimePrinter stp(QString("TIME: %1 µs [QUERY TOTAL '%2']").arg("%1", query_string));

    if (!queries.empty())
        queries.back()->cancel();

    auto it = find_if(trigger_map.cbegin(), trigger_map.cend(),
                      [&query_string](const pair<QString,QueryHandler*> &pair)
                      { return query_string.startsWith(pair.first); });
    unique_ptr<::Query> query;
    if (it != trigger_map.cend()){  // trigger matched
        const auto&[trigger, handler] = *it;
        query = make_unique<::Query>(ExtensionWatcher<FallbackProvider>::extensions(),
                                     handler, query_string.mid(it->first.size()), trigger);
    }
    else {
        auto gsh = new GlobalSearchHandler;
        query = make_unique<::Query>(ExtensionWatcher<FallbackProvider>::extensions(),
                                     gsh, query_string.mid(it->first.size()));
        QObject::connect(query.get(), &albert::Query::finished, [gsh](){ delete gsh; });

    }
    return queries.emplace_back(::move(query)).get();
}

void QueryEngine::onAdd(QueryHandler *handler) { updateTriggers(); }

void QueryEngine::onRem(QueryHandler *handler) { updateTriggers(); } // Todo

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



