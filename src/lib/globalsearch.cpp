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

QString GlobalSearch::default_trigger() const
{
    return QLatin1String("");
}

bool GlobalSearch::allow_trigger_remap() const
{
    return false;
}

std::vector<albert::RankItem> GlobalSearch::rankItems(const Query &query) const
{
    std::vector<albert::RankItem> rank_items;
    function<std::vector<albert::RankItem>(GlobalQueryHandler*)> map =
            [&query](GlobalQueryHandler *handler) -> std::vector<albert::RankItem> {
        TimePrinter tp(QString("TIME: %1 µs ['%2']").arg("%1", handler->id()));
        return handler->rankItems(query);
    };
    auto future = QtConcurrent::mapped(extensions(), map);
    future.waitForFinished();
    for (auto result : future)
        rank_items.insert(end(rank_items),
                          make_move_iterator(begin(result)),
                          make_move_iterator(end(result)));
    return rank_items;
}