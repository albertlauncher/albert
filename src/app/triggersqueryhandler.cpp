// Copyright (c) 2023-2024 Manuel Schneider

#include "app.h"
#include "matcher.h"
#include "queryengine.h"
#include "standarditem.h"
#include "triggersqueryhandler.h"
using namespace albert::util;
using namespace albert;
using namespace std;

const QStringList TriggersQueryHandler::icon_urls{QStringLiteral(":app_icon")};

TriggersQueryHandler::TriggersQueryHandler(const QueryEngine &query_engine):
    query_engine_(query_engine) {}

QString TriggersQueryHandler::id() const
{ return QStringLiteral("triggers"); }

QString TriggersQueryHandler::name() const
{ return QStringLiteral("Triggers"); }

QString TriggersQueryHandler::description() const
{ return tr("Trigger completion items."); }

shared_ptr<Item> TriggersQueryHandler::makeItem(const QString &trigger, Extension *handler) const
{
    return StandardItem::make(
        trigger,
        QString(trigger).replace(" ", "•"),
        QString("%1 · %2").arg(handler->name(), handler->description()),
        trigger,
        {QStringLiteral("gen:?&text=🚀")},
        {{
            "set",
            tr("Set input text"),
            [trigger]{ App::instance()->show(trigger); },
            false
        }}
    );
}

void TriggersQueryHandler::handleTriggerQuery(Query &q)
{
    auto ris = handleGlobalQuery(q);
    applyUsageScore(&ris);
    ranges::sort(ris, greater());

    vector<shared_ptr<Item>> is;
    is.reserve(ris.size());
    for (auto &ri : ris)
        is.emplace_back(::move(ri.item));

    q.add(is);
}

vector<RankItem> TriggersQueryHandler::handleGlobalQuery(const Query &q)
{
    Matcher matcher(q);
    vector<RankItem> r;
    for (const auto &[t, h] : query_engine_.activeTriggerHandlers())
        if (const auto m = Matcher(q.string()).match(t, h->name(), h->id()); m)
            r.emplace_back(makeItem(t, h), m);
    return r;
}
