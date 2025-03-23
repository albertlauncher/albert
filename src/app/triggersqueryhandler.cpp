// Copyright (c) 2023-2024 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "matcher.h"
#include "queryengine.h"
#include "standarditem.h"
#include "triggersqueryhandler.h"
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
    auto desc = QString("%1 - %2").arg(handler->name(), handler->description());
    return StandardItem::make(
        trigger,
        QString(trigger).replace(" ", "•"),
        desc,
        trigger,
        {QStringLiteral("gen:?&text=🚀")},
        {{
            "set",
            tr("Set input text"),
            [trigger]{ App::instance()->frontend()->setInput(trigger); },
            false
        }}
    );
}

void TriggersQueryHandler::handleTriggerQuery(Query &q)
{
    // Match tigger, id and name.

    vector<RankItem> RI;
    for (const auto &[trigger, handler] : query_engine_.activeTriggerHandlers())
        if (auto m = Matcher(q.string()).match(trigger, handler->name(), handler->id()); m)
            RI.emplace_back(makeItem(trigger, handler), m);

    applyUsageScore(&RI);

    ranges::sort(RI, greater());

    vector<shared_ptr<Item>> I;
    I.reserve(RI.size());
    for (auto &ri : RI)
        I.emplace_back(::move(ri.item));

    q.add(I);
}

vector<RankItem> TriggersQueryHandler::handleGlobalQuery(const Query &q)
{
    // Strictly match trigger

    vector<RankItem> rank_items;

    Matcher matcher(q.string(), { .ignore_case=false, .ignore_word_order=false });
    for (const auto &[trigger, handler] : query_engine_.activeTriggerHandlers())
        if (auto m = matcher.match(trigger); m)
            rank_items.emplace_back(makeItem(trigger, handler), m);

    return rank_items;
}
