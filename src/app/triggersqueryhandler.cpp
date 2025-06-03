// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
#include "matcher.h"
#include "queryengine.h"
#include "standarditem.h"
#include "triggersqueryhandler.h"
#include <mutex>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

TriggersQueryHandler::TriggersQueryHandler(const QueryEngine &query_engine):
    query_engine_(query_engine),
    trigger_handlers_(query_engine.activeTriggerHandlers())
{
    // Query engine is not thread safe. Keep a copy.

    QObject::connect(&query_engine, &QueryEngine::handlerAdded, this, [this]{
        lock_guard l(trigger_handlers_mutex_);
        trigger_handlers_ = query_engine_.activeTriggerHandlers();
    });

    QObject::connect(&query_engine, &QueryEngine::handlerRemoved, this, [this]{
        lock_guard l(trigger_handlers_mutex_);
        trigger_handlers_ = query_engine_.activeTriggerHandlers();
    });
}

QString TriggersQueryHandler::id() const { return u"triggers"_s; }

QString TriggersQueryHandler::name() const { return u"Triggers"_s; }

QString TriggersQueryHandler::description() const { return tr("Trigger completion items."); }

shared_ptr<Item> TriggersQueryHandler::makeItem(const QString &trigger, Extension *handler) const
{
    return StandardItem::make(
        trigger,
        QString(trigger).replace(" ", "•"),
        QString("%1 · %2").arg(handler->name(), handler->description()),
        trigger,
        {u"gen:?&text=🚀"_s},
        {{
            "set",
            tr("Set input text"),
            [trigger]{ App::instance()->show(trigger); },
            false
        }}
    );
}

vector<RankItem> TriggersQueryHandler::handleGlobalQuery(const Query &q)
{
    shared_lock l(trigger_handlers_mutex_);
    Matcher matcher(q);
    vector<RankItem> r;
    for (const auto &[t, h] : trigger_handlers_)
        if (const auto m = Matcher(q.string()).match(t, h->name(), h->id()); m)
            r.emplace_back(makeItem(t, h), m);
    return r;
}
