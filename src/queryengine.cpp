// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extensionregistry.h"
#include "albert/logging.h"
#include "albert/query/fallbackhandler.h"
#include "albert/query/globalqueryhandler.h"
#include "albert/query/triggerqueryhandler.h"
#include "queryengine.h"
#include "queryexecution.h"
#include "usagedatabase.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;
static const char *CFG_GLOBAL_HANDLER_ENABLED = "global_handler_enabled";
static const char *CFG_FALLBACK_ORDER = "fallback_order";
static const char *CFG_FALLBACK_EXTENSION = "extension";
static const char *CFG_FALLBACK_ITEM = "fallback";
static const char *CFG_TRIGGER = "trigger";
static const char *CFG_FUZZY = "fuzzy";

QueryEngine::QueryEngine(ExtensionRegistry &registry) : registry_(registry)
{
    UsageHistory::initialize();
    loadFallbackOrder();

    connect(&registry, &ExtensionRegistry::added, this, [this](Extension *e) {
        if (auto *th = dynamic_cast<albert::TriggerQueryHandler*>(e))
        {
            auto s = settings();
            s->beginGroup(th->id());
            auto t = s->value(CFG_TRIGGER, th->defaultTrigger()).toString();
            auto f = s->value(CFG_FUZZY, false).toBool();

            th->setTrigger(t);
            th->setFuzzyMatching(f);
            trigger_handlers_.emplace(piecewise_construct,
                                      forward_as_tuple(th->id()),
                                      forward_as_tuple(th, t, f));
            updateActiveTriggers();

            if (auto *gh = dynamic_cast<albert::GlobalQueryHandler*>(th))
            {
                auto en = settings()->value(QString("%1/%2")
                                                .arg(gh->id(), CFG_GLOBAL_HANDLER_ENABLED),
                                            true).toBool();
                global_handlers_.emplace(piecewise_construct,
                                         forward_as_tuple(gh->id()),
                                         forward_as_tuple(gh, en));
            }

            emit handlerAdded();
        }
        if (auto *fh = dynamic_cast<albert::FallbackHandler*>(e))
        {
            fallback_handlers_.emplace(fh->id(), fh);
            emit handlerAdded();
        }
    });

    connect(&registry, &ExtensionRegistry::removed, this, [this](Extension *e) {
        if (auto *th = dynamic_cast<albert::TriggerQueryHandler*>(e))
        {
            trigger_handlers_.erase(th->id());
            updateActiveTriggers();

            if (auto *gh = dynamic_cast<albert::GlobalQueryHandler*>(th))
                global_handlers_.erase(gh->id());

            emit handlerRemoved();
        }
        if (auto *fh = dynamic_cast<albert::FallbackHandler*>(e))
        {
            fallback_handlers_.erase(fh->id());
            emit handlerRemoved();
        }
    });
}

unique_ptr<QueryExecution> QueryEngine::query(const QString &query_string)
{
    vector<FallbackHandler*> fhandlers;
    for (const auto&[id, handler] : fallback_handlers_)
        fhandlers.emplace_back(handler);

    for (const auto &[trigger, handler] : active_triggers_)
        if (query_string.startsWith(trigger))
            return make_unique<QueryExecution>(this, ::move(fhandlers), handler, query_string.mid(trigger.size()), trigger);

    {
        vector<albert::GlobalQueryHandler*> handlers;
        for (const auto&[id, h] : global_handlers_)
            if (h.enabled)
                handlers.emplace_back(h.handler);
        return make_unique<GlobalQuery>(this, ::move(fhandlers), ::move(handlers), query_string);
    }
}

//
// Trigger handlers
//

map<QString, TriggerQueryHandler*> QueryEngine::triggerHandlers()
{
    map<QString, albert::TriggerQueryHandler*> handlers;
    for (const auto &[id, h] : trigger_handlers_)
        handlers.emplace(id, h.handler);
    return handlers;
}

const map<QString, TriggerQueryHandler *> &QueryEngine::activeTriggerHandlers()
{ return active_triggers_; }

void QueryEngine::updateActiveTriggers()
{
    active_triggers_.clear();
    for (const auto&[id, h] : trigger_handlers_)
        if (const auto&[it, success] = active_triggers_.emplace(h.trigger, h.handler); !success)
            WARN << QString("Trigger '%1' of '%2' already registered for '%3'.")
                        .arg(h.trigger, id, it->second->id());
}

QString QueryEngine::trigger(const QString &id) const
{ return trigger_handlers_.at(id).trigger; }

void QueryEngine::setTrigger(const QString &id, const QString& t)
{
    auto &h = trigger_handlers_.at(id);

    if (h.trigger == t || !h.handler->allowTriggerRemap())
        return;

    if (t.isEmpty() || t == h.handler->defaultTrigger())
    {
        h.trigger = h.handler->defaultTrigger();
        settings()->remove(QString("%1/%2").arg(id, CFG_TRIGGER));
    }
    else
    {
        h.trigger = t;
        settings()->setValue(QString("%1/%2").arg(id, CFG_TRIGGER), t);
    }

    h.handler->setTrigger(h.trigger);
    updateActiveTriggers();
}

bool QueryEngine::fuzzy(const QString &id) const
{ return trigger_handlers_.at(id).fuzzy; }

void QueryEngine::setFuzzy(const QString &id, bool f)
{
    auto &h = trigger_handlers_.at(id);

    if (h.handler->supportsFuzzyMatching())
    {
        h.fuzzy = f;
        settings()->setValue(QString("%1/%2").arg(id, CFG_FUZZY), f);
        h.handler->setFuzzyMatching(f);
    }
}


//
// Global handlers
//

map<QString, GlobalQueryHandler*> QueryEngine::globalHandlers()
{
    map<QString, albert::GlobalQueryHandler*> handlers;
    for (const auto &[id, h] : global_handlers_)
        handlers.emplace(id, h.handler);
    return handlers;
}

bool QueryEngine::isEnabled(const QString &id) const
{ return global_handlers_.at(id).enabled; }

void QueryEngine::setEnabled(const QString &id, bool e)
{
    auto &h = global_handlers_.at(id);

    if (h.enabled != e)
    {
        settings()->setValue(QString("%1/%2").arg(id, CFG_GLOBAL_HANDLER_ENABLED), e);
        h.enabled = e;
    }
}


//
// Fallback handlers
//

map<QString, FallbackHandler*> QueryEngine::fallbackHandlers()
{ return fallback_handlers_; }

map<pair<QString,QString>,int> QueryEngine::fallbackOrder() const
{ return fallback_order_; }

void QueryEngine::setFallbackOrder(map<pair<QString,QString>,int> order)
{
    fallback_order_ = order;
    saveFallbackOrder();
}

// bool QueryEngine::isEnabled(const FallbackHandler *h) const
// { return enabled_fallback_handlers_.contains(h->id()); }

// void QueryEngine::setEnabled(FallbackHandler *h, bool e)
// {
//     if (isEnabled(h) == e)
//         return;

//     settings()->setValue(QString("%1/%2").arg(h->id(), CFG_FHANDLER_ENABLED), e);

//     if (e)
//         enabled_fallback_handlers_.emplace(h->id(), h);
//     else
//         enabled_fallback_handlers_.erase(h->id());

//     emit handlersChanged();
// }

void QueryEngine::saveFallbackOrder() const
{
    // Invert to ordered list
    vector<pair<QString, QString>> o;
    for (const auto &[pair, prio] : fallback_order_)
        o.emplace_back(pair.first, pair.second);
    sort(begin(o), end(o), [&](const auto &a, const auto &b)
         { return fallback_order_.at(a) > fallback_order_.at(b); });

    // Save to settings
    auto s = settings();
    s->beginWriteArray(CFG_FALLBACK_ORDER);
    for (int i = 0; i < (int)o.size(); ++i)
    {
        s->setArrayIndex(i);
        s->setValue(CFG_FALLBACK_EXTENSION, o.at(i).first);
        s->setValue(CFG_FALLBACK_ITEM, o.at(i).second);
    }
    s->endArray();
}

void QueryEngine::loadFallbackOrder()
{
    // Load from settings
    vector<pair<QString, QString>> o;
    auto s = settings();
    int size = s->beginReadArray(CFG_FALLBACK_ORDER);
    for (int i = 0; i < size; ++i)
    {
        s->setArrayIndex(i);
        o.emplace_back(s->value(CFG_FALLBACK_EXTENSION).toString(),
                       s->value(CFG_FALLBACK_ITEM).toString());
    }
    s->endArray();

    // Create order map
    fallback_order_.clear();
    uint rank = 1;
    for (auto it = o.rbegin(); it != o.rend(); ++it, ++rank)
        fallback_order_.emplace(*it, rank);
}
