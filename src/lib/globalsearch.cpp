// Copyright (c) 2022 Manuel Schneider

#include "albert/item.h"
#include "globalsearch.h"
#include "timeprinter.hpp"
#include <QtConcurrent>
#include <mutex>
using namespace albert;
using namespace std;


GlobalSearch::GlobalSearch(ExtensionRegistry &registry) : ExtensionWatcher<albert::GlobalQueryHandler>(registry)
{
}

QString GlobalSearch::id() const
{
    return QStringLiteral("globalsearch");
}

QString GlobalSearch::synopsis() const
{
    return QStringLiteral("<filter>");
}

QString GlobalSearch::default_trigger() const
{
    return QLatin1String("");
}

bool GlobalSearch::allow_trigger_remap() const
{
    return false;
}

std::vector<std::pair<std::shared_ptr<albert::Item>,uint16_t>>
GlobalSearch::rankedItems(const albert::Query &query) const {
    // QtConcurrent5 sucks and does not support move (copies…!) hack around
    mutex m;
    std::vector<std::pair<std::shared_ptr<albert::Item>,uint16_t>> results;
    function<void(GlobalQueryHandler*)> map = [&query, &m, &results](GlobalQueryHandler *handler) {
        TimePrinter tp(QString("TIME: %1 µs ['%2']").arg("%1", handler->id()));
        auto &&intermediate = handler->rankedItems(query);
        [[maybe_unused]] const lock_guard<mutex> lock(m);
        results.insert(end(results),
                       make_move_iterator(begin(intermediate)),
                       make_move_iterator(end(intermediate)));
    };
    QtConcurrent::blockingMap(ExtensionWatcher<GlobalQueryHandler>::extensions(), map);
    return results;
}