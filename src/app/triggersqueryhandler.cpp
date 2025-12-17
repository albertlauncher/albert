// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
#include "iconutil.h"
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

shared_ptr<Item> TriggersQueryHandler::makeItem(const QString &trigger, Extension *handler) const
{
    return StandardItem::make(
        handler->id(),
        QString(trigger).replace(" ", "•"),
        QString("%1 · %2").arg(handler->name(), handler->description()),
        []{ return makeGraphemeIcon(u"🚀"_s); },
        {{
            "set",
            tr("Set input text"),
            [trigger]{ App::instance().show(trigger); },
            false
        }},
        trigger
        );
}

vector<RankItem> TriggersQueryHandler::rankItems(Query &q)
{
    Matcher matcher(q, {.fuzzy = fuzzy_});
    vector<RankItem> r;

    for (shared_lock l(handler_triggers_mutex_);
         const auto &[t, h] : handler_triggers_)
        if (!q.isValid())
            break;
        else if (const auto m = matcher.match(t, h->name(), h->id()); m)
            r.emplace_back(makeItem(t, h), m);

    return r;
}

void TriggersQueryHandler::updateTriggers()
{
    lock_guard lock(handler_triggers_mutex_);
    handler_triggers_ = query_engine_.activeTriggerHandlers();
}
