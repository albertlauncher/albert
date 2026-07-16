// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
#include "icon.h"
#include "logging.h"
#include "matcher.h"
#include "queryengine.h"
#include "standarditem.h"
#include "triggersqueryhandler.h"
#include <mutex>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

TriggersQueryHandler::TriggersQueryHandler(const QueryEngine &query_engine):
    query_engine_(query_engine)
{
    QObject::connect(&query_engine, &QueryEngine::activeTriggersChanged,
                     this, &TriggersQueryHandler::updateTriggers);
    updateTriggers();
}

QString TriggersQueryHandler::id() const { return u"triggers"_s; }

QString TriggersQueryHandler::name() const { return u"Triggers"_s; }

QString TriggersQueryHandler::description() const { return tr("Trigger completions"); }

void TriggersQueryHandler::setFuzzyMatching(bool fuzzy) { fuzzy_ = fuzzy; }

bool TriggersQueryHandler::supportsFuzzyMatching() const { return true; }

shared_ptr<Item> TriggersQueryHandler::makeItem(const TriggerHandler &h) const
{
    return StandardItem::make(
        h.id,
        QString(h.trigger).replace(" ", "•"),
        QString("%1 · %2").arg(h.name, h.description),
        []{ return Icon::grapheme(u"🚀"_s); },
        {{
            "set",
            tr("Set input text"),
            [&]{ app().show(h.trigger); },
            false
        }},
        h.trigger
        );
}

vector<RankItem> TriggersQueryHandler::rankItems(QueryContext &ctx)
{
    Matcher matcher(ctx, {.fuzzy = fuzzy_});
    vector<RankItem> r;

    for (shared_lock l(trigger_handlers_mutex_);
         const auto &h : trigger_handlers_)
        if (!ctx.isValid())
            break;
        else if (const auto m = matcher.match(h.trigger, h.name, h.id); m)
            r.emplace_back(makeItem(h), m);

    return r;
}

void TriggersQueryHandler::updateTriggers()
{
    try {
        vector<TriggerHandler> trigger_handlers;
        for (const auto &[t, h] : query_engine_.activeTriggerHandlers())
            trigger_handlers.emplace_back(h->id(), h->name(), h->description(), t);
        lock_guard lock(trigger_handlers_mutex_);
        trigger_handlers_ = ::move(trigger_handlers);
    }
    catch (...) {
        WARN << u"QueryHandler threw exception while updating TriggersQueryHandler."_s;
    }
}
